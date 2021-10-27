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


__nv uint8_t first_init = 1;
__nv uint8_t solar_check = 0;
__nv uint8_t got_gps_fix = 0;

#define MAGIC_NUMBER 0xABCD

__nv uint16_t first_prog = MAGIC_NUMBER;

// Leave volatile
uint8_t expt_timer_triggered = 0;
uint8_t expt_need_time = 1;
uint8_t expt_need_jump = 1;


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
  expt_timer_triggered = 0;
  int count;
  init_timerA0();
  COMM_ENABLE;
  EXP_ENABLE; // Init expt but don't feed it anything just yet
  app_gps_init(); // Turn on gps
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
        //BIT_FLIP(1,1);
        if (telem_timer_triggered) {
          update_telemetry();
          telem_timer_triggered = 0;
        }
        next_task = GET_UART1;
        break;
      }
      case(GET_UART1):{
        //BIT_FLIP(1,1);
        //BIT_FLIP(1,1);
        if (expt_timer_triggered || expt_need_time) {
          if (expt_need_jump) {
            // Send jump command
            expt_ack_pending = 1;
            expt_write_jump();
            __delay_cycles(80000);
            int temp = process_uart1();
            if (temp == RCVD_PENDING_BOOTLOADER_ACK) {
              expt_ack_pending = 0;
              expt_need_jump = 0;
            }
          }
          got_gps_fix = app_gps_gather();
          if (got_gps_fix) {
            uint8_t* time = artibeus_get_time();
            uint8_t* date = artibeus_get_date();
            uint8_t time_date[ARTIBEUS_TIME_DATE_SIZE];
            memcpy(time_date,time,3);
            memcpy(time_date + 3,date,3);
            libartibeus_msg_id = 47;
            // We'll always run this through, it's init
            for (int i = 0; i < 2; i++) {
              expt_set_time(time_date);
              __delay_cycles(8000);
              expt_ack_pending = 1;
              int temp = process_uart1();
              if (temp == RCVD_PENDING_ACK) {
                expt_ack_pending = 0;
                expt_need_time = 0;
              break; }
            }
          }
        }
        else {
          BIT_FLIP(1,1);
          process_uart1();
        }
        next_task = GET_UART0;
        break;
      }
      case(GET_UART0):{
        //BIT_FLIP(1,1);
        //BIT_FLIP(1,1);
        //BIT_FLIP(1,1);
        int temp;
        temp = process_uart0();
        if (temp == RCVD_TELEM_ASCII) {
          // Transmit packet in response (in process_uart0)
          next_task = GET_UART0; // Make sure there aren't more requests waiting
        }
        else if (temp == RCVD_BUFF_REQ_ASCII) {
          // Transmit packet in response (in process_uart0)
          next_task = GET_UART0; // Make sure there aren't more requests waiting
        }
        else {
          next_task = RECORD_TELEM;
        }
        break;
      }
      default:
        next_task = RECORD_TELEM;
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
  BIT_FLIP(1,2);
  expt_timer_triggered = 1;
  telem_timer_triggered = 1;
  gps_timer++;
  TA0R = 0;
  __enable_interrupt();
}

