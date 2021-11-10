#include <msp430.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stddef.h>

#include <libio/console.h>
#include <libmspware/driverlib.h>
#include <libmsp/watchdog.h>
#include <libmsp/clock.h>
#include <libmsp/mem.h>
#include <libmsp/gpio.h>
#include <libmsp/periph.h>
#include <libmsp/sleep.h>


#include <libartibeus/artibeus.h>
#include <libartibeus/comm.h>
#include <libads/ads1115.h>
#include <libmspuartlink/uartlink.h>
#include <libgnss/gnss.h>

#include "main.h"
#include "pkts.h"
#include "adc.h"
#include "gps.h"

#define SLEEP_DELAY 36864 //~9 sec at 32768/8 Hz ticks --> need to loop 10x

__nv uint8_t first_init = 1;
__nv uint8_t solar_check = 0;

#define MAGIC_NUMBER 0xABCD

__nv uint16_t first_prog = MAGIC_NUMBER;

__nv artibeus_ctx ctx0 = { GET_UART1, -1 };
__nv artibeus_ctx ctx1 = { GET_UART1, -1 };
__nv artibeus_ctx *cur_ctx = &ctx0;



int main(void) {
  // Make sure we hold after programming
  // Pull this pin to ground externally BEFORE programming
  if (first_init) {
    artibeus_first_init();
    first_init = 0;
  }
  else {
    artibeus_init();
  }
  int count;
  // Clear GPS variables
  fix_recorded = 0;
  no_fix_counter = 0;
  // Clear transfer variables
  comm_expt_link.transfer_len = 0;
  comm_expt_link.status = TRANSFER_DONE;
  // Restore any corrupted data
  restore_from_backup(cur_ctx);
  // Scheduler loop!
  while(1) {
    locations last_location;
    uint16_t Vcap;
    artibeus_mode next_task;
    // Pet watchdog
    msp_watchdog_kick();
    switch (cur_ctx->cur_task) {
      case ADC: // Read from ADC
        // Check if there's room for all the data
        if ((pkt_ptr + (LIBADS_NUM_CHANNELS << 1)) >
          pkt_buffer + PKT_BUFFER_SIZE) {
          //TODO raise some kind of warning that our buffer is full
          next_task = IMU;
          break;
        }
        PRINTF("Pkt_ptr is %x .\r\n",pkt_ptr);
        write_to_log(cur_ctx, &pkt_ptr, sizeof(uint8_t *));
        adc_update(pkt_ptr);
        pkt_ptr = pkt_ptr + (LIBADS_NUM_CHANNELS << 1);
        next_task = IMU;
        break;
      case IMU:
        // Read from IMU next time
        next_task = GNSS;
        break;
      case GNSS:
        // If no fix recorded, vcap >4.7 (x factor for voltage divider),
        //otherwise vcap > 3.2
        Vcap = get_vcap();
        if (!fix_recorded &&  Vcap < 2800) {
          COMM_ENABLE;
          next_task = SLEEP;
          break;
        }
        if (fix_recorded && Vcap < 1900) {
          next_task = SLEEP;
          break;
        }
        GNSS_ENABLE;
        write_to_log(cur_ctx, &cur_gps_data, sizeof(gps_data *));
        int need_fix = -1;
        for(int i = 0; i < 90; i++) {
          // Wait one sec
          __delay_cycles(8000000);
          need_fix = scrape_gps_buffer();
          if (!need_fix) {
            fix_recorded = 1;
            no_fix_counter = 0;
            break;
          }
          // Increment no_fix count if we were supposed to have a fix but lost
          // it
          if (i == 10 && fix_recorded) {
            no_fix_counter += 1;
          }
        }
        if (!need_fix) {
          write_to_log(cur_ctx, &pkt_ptr, sizeof(uint8_t *));
          gps_update(pkt_ptr);
          last_location = good_location(cur_gps_data);
        }
        else {
          last_location = NOWHERE;
          // Reset GPS if:
          // we still don't have a fix after 60s
          // or we tried 3 times to get a fix and still don't have it.
          if (!fix_recorded || (no_fix_counter > 3)) {
            RESET_GNSS;
          }
        }
        if (last_location != NOWHERE) {
          next_task = COMM;
        }
        else {
           next_task = SLEEP;
        }
        break;
      case COMM:
        if (rf_dead == 0xAB || rf_kill_count > 5) {
          next_task = SLEEP;
          break;
        }
        // Check in with COMM board
        COMM_ENABLE;
        uartlink_close(0);
        uartlink_open_rx(0);
        // Turn on and linsten for some time
        for(int i = 0; i < 2; i++) {
          __delay_cycles(8000000);//TODO figure out time to turn on
        }
        // If kill count is too high, activate dead
        if (rf_kill_count > 5) {
          //TODO add message
          rf_dead = MAGIC_NUMBER;
          next_task = SLEEP;
          break;
        }
        uartlink_close(0);
        // Jump to XFER without transmit if we're ready
        // TODO make this work
        /*if (comm_expt_link.status == TRANSFER_ACTIVE) {
          next_task = XFER;
          break;
        }*/
        uartlink_open_tx(0);
        write_to_log(cur_ctx, &pkt_ptr, sizeof(uint8_t *));
        // Update score
        for (int i = 0; i < 32; i++) {
          *pkt_ptr++ = score_msg[i];
        }
        // Transfer telem + score
        comm_transmit_pkt(pkt_buffer, pkt_ptr - pkt_buffer);
        // Clear buffer
        pkt_ptr = pkt_buffer;
        // Keep listening
        uartlink_close(0);
        uartlink_open_rx(0);
        for(int i = 0; i < 3; i++) {
          __delay_cycles(8000000);//TODO figure out time to turn off
        }
        // disable comm
        COMM_DISABLE;
        // Leave comm enabled to check for transfer
        // TODO change this
        //next_task = XFER;
        next_task = EXPT;
        break;
#if 0
      case XFER:
        // Turn on EXPT board to dump bytes
        EXP_ENABLE;
        // If uart isr sets "transfer start" condition, tell ground we're ready
        // to receive
        if (comm_expt_link.status == TRANSFER_ACTIVE && rf_dead != MAGIC_NUMBER) {
          // Turn on comm board to listen (it should already be on)
          COMM_ENABLE;
          comm_transmit_ready();
        }
        while (comm_expt_link.status == TRANSFER_ACTIVE) {
          unsigned temp;
          // read from comm to mcu
          uartlink_open_rx(0);
          uartlink_open_tx(1);
          // If transfer ready, send it
          if (comm_expt_link.ready == 1) {
            // write from mcu to expt
            uartlink_send_basic(1,xfer_buffer,comm_expt_link.transfer_len);
          }
          /*
          TODO figure out how to get bytes back
          // read from expt to mcu
          uartlink_close(1);
          uartlink_open_rx(1);
          temp = uartlink_receive_basic(1,ack_buff,ACK_BUFFER_SIZE);
          if (temp != ACK_BUFFER_SIZE) {
            break;
          }
          if(!is_bootloader_ack(ack_buff)) {
            break;
          }*/
        }
        uartlink_close(1);
        next_task = EXPT;
        break;
#endif
      case EXPT:
        // if (!VOLTAGE_OK) DISABLE_COMM
        EXP_ENABLE;
        uartlink_open_tx(1);
        expt_write_jump();
        uartlink_close(1);
        uartlink_open_rx(1);
        // Check in with Expt board
        for(int i = 0; i < 2; i++) {
          __delay_cycles(8000000);
        }
        uartlink_close(1);
        //TODO pass uart vals from expt to comm
        next_task = SLEEP;
        break;
      case SLEEP:
        // Sleep for n seconds
        // Put GNSS, COMM, EXPT to sleep
        COMM_DISABLE;
        GNSS_DISABLE;
        EXP_DISABLE;
        // Sleep for ~90 sec
        for(int i = 0; i < 10; i++) {
          msp_sleep(SLEEP_DELAY);
        }
        next_task = ADC;
        break;
      default:
        next_task = SLEEP;
        break;
    }
    // Safely transition
    artibeus_ctx *next_ctx;
    next_ctx = (cur_ctx == &ctx0) ? &ctx1 : &ctx0;
    next_ctx->dbl_buffer_count = -1;
    next_ctx->cur_task = next_task;
    // Swap
    cur_ctx = next_ctx;
  }
}


