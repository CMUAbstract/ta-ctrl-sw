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
#include <libartibeus/handle_uarts.h>
#include <libartibeus/backup.h>
#include <libads/ads1115.h>
#include <libmspuartlink/uartlink.h>
#include <libgnss/gnss.h>

#include "main.h"
#include "pkts.h"
#include "adc.h"
#include "gps.h"
#include "telem.h"

#define SLEEP_DELAY 36864 //~9 sec at 32768/8 Hz ticks --> need to loop 10x

__nv uint8_t first_init = 1;
__nv uint8_t solar_check = 0;
__nv uint8_t got_gps_fix = 0;

#define MAGIC_NUMBER 0xABCD

__nv uint16_t first_prog = MAGIC_NUMBER;


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
  //TODO move this into app code somewhere, it invalidates variables here
  app_gps_init();
  uint8_t* time_date[ARTIBEUS_TIME_DATE_SIZE];
  __delay_cycles(8000000); 
  __delay_cycles(8000000); //Try to update the time
  got_gps_fix = app_gps_gather();
  if (got_gps_fix) {
    uint8_t* time = artibeus_get_time();
    uint8_t* date = artibeus_get_date();
    memcpy(time_date,time,3);
    memcpy(time_date + 3,date,3);
    GNSS_DISABLE;
    EXP_ENABLE;
    libartibeus_msg_id = 0;
    // We'll always run this through, it's init
    for (int i = 0; i < 2; i++) {
      expt_set_time_utc(time_date);
      __delay_cycles(8000);
      libartibeus_msg_id = 0;
      expt_ack_pending = 1;
      int temp = process_uart1();
      if (temp == RCVD_PENDING_ACK) { expt_ack_pending = 0; break; }
    }
  }
  init_timerA0();
  COMM_ENABLE;
  // Clear transfer variables
  // Restore any corrupted data
  restore_from_backup(cur_ctx);
  // Scheduler loop!
  while(1) {
    artibeus_mode next_task;
    // Pet watchdog
    msp_watchdog_kick();
    switch (cur_ctx->cur_task) {
      case(RECORD_TELEM):{
        update_telemetry();
        next_task = GET_UART1;
        break;
      }
      case(GET_UART1):{
        process_uart1();
        next_task = GET_UART0;
        break;
      }
      case(GET_UART0):{
        process_uart0();
        next_task = CHECK_GNSS_TIMER;
        break;
      }
      case(CHECK_GNSS_TIMER):{
        if (!gps_timer_triggered) {
          next_task = CHECK_ASCII_TIMER;
          break;
        }
        write_to_log(cur_ctx,&gps_timer_triggered,sizeof(uint8_t));
        gps_timer_triggered = 0;
        // Check if there's anything in the buffer
        if (telem_buffer_tail == telem_buffer_head && telem_buffer_full == 0) {
          next_task = CHECK_ASCII_TIMER;
          break;
        }
        uint8_t *telem_ptr = pop_telem_pkt();
        for (int i = 0; i < TELEM_REPEAT_CNT; i++) {
          libartibeus_msg_id = telem_buffer_tail;
          comm_transmit_pkt(telem_ptr,sizeof(artibeus_telem_t) + 1);
          __delay_cycles(80000);
          comm_ack_pending = 1; // sanitize value
          int temp = process_uart0();
          if (temp == RCVD_PENDING_ACK) { comm_ack_pending = 0; break; }
        }
        pop_update_telem_ptrs();
        next_task = CHECK_ASCII_TIMER;
        break;
      }
      case CHECK_ASCII_TIMER: {
        if (!ascii_timer_triggered) {
          next_task = CHECK_ASCII_TIMER;
          break;
        }
        write_to_log(cur_ctx,&ascii_timer_triggered,sizeof(uint8_t));
        ascii_timer_triggered = 0;
        if (artibeus_ascii_is_empty()) {
          break;
        }
        uint8_t *ascii_ptr = artibeus_pop_ascii_pkt();
        for (int i = 0; i < TELEM_REPEAT_CNT; i++) {
          libartibeus_msg_id = expt_ascii_tail;
          comm_transmit_pkt(ascii_ptr,ARTIBEUS_MAX_ASCII_SIZE);
          __delay_cycles(80000);
          comm_ack_pending = 1; // sanitize value
          int temp = process_uart1();
          if (temp == RCVD_PENDING_ACK) { comm_ack_pending = 0; break; }
        }
        artibeus_pop_update_ascii_ptrs();
        next_task = RECORD_TELEM;
        break;
      }
      default:
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


void __attribute ((interrupt(TIMER0_A0_VECTOR))) Timer0_A0_ISR(void)
{ //Handles overflows
  __disable_interrupt();
  switch(__even_in_range(TAIV,TAIV__TAIFG)) {
    case TAIV_2: //TA0CCR1
      TA0CCR0 += TELEM_PERIOD;
      gps_timer_triggered = 1;
      break;
    case TAIV_4: //TA0CCR2
      TA0CCR1 += EXPT_ASCII_PERIOD;
      ascii_timer_triggered = 1;
      break;
    case TAIV_6: //TA0CCR3
      TA0CCR2 += SLEEP_PERIOD;
    default:
      break;
  }
  TA0R = 0;
  __enable_interrupt();// A little paranoia over comp_e getting thrown
}

