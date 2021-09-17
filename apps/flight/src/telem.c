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
#include <libartibeus/query.h>
#include <libartibeus/backup.h>
#include <libads/ads1115.h>
#include <libmspuartlink/uartlink.h>
#include <libgnss/gnss.h>
#include <liblsm9ds1/imu.h>
#include "gps.h"
#include "adc.h"
#include "telem.h"

__nv artibeus_telem_t telem_buffer[TELEM_BUFF_SIZE]; 
__nv uint8_t telem_buffer_head = 0;
__nv uint8_t telem_buffer_tail = 0;
__nv uint8_t telem_buffer_full = 0;

__nv uint8_t gps_timer_triggered = 0;
__nv uint8_t ascii_timer_triggered = 0;
//TODO double check the procedure for need_fix
int need_fix = -1;
// Self contained telemetry function that gathers data, updates the most recent
// query-able data elements, and updates the packets that will get transmitted.
int update_telemetry() {
  // Grab data from IMU
  int temp = -1;
  while(temp < 0) { temp = init_lsm9ds1(); }
  // Update most recent IMU data and update averaged data structures
  int16_t x,y,z;
  read_xl(&x,&y,&z);
  artibeus_set_xl(x,y,z);
  read_g(&x,&y,&z);
  artibeus_set_g(x,y,z);
  read_m(&x,&y,&z);
  artibeus_set_m(x,y,z);
  // Grab power data
  uint16_t temp_buf[4];
  for(int i = 0; i < 4; i++) {
    temp_buf[i] = ads_se_read(i);
    // Shift left by 3 bits so that we can get translation to mV
    //TODO move this transform into libads or libartibeus
    temp_buf[i] = temp_buf[i] >> 3;
  }
  // Update most recent power data & averaged data structures
  artibeus_set_pwr(temp_buf);
  uint16_t gps_timer_set = TA0CCTL1 | CCIE;
  write_to_log(cur_ctx,&gps_start_count,sizeof(uint8_t));
  // Check GPS timer and set up if necessary
  if (!gps_timer_set && (gps_start_count > GPS_FAIL_MAX)) {
    GNSS_DISABLE;
    TA0CCTL1 |= CCIE;
    TA0R = 0;
    gps_start_count = 0;
  }
  else if (gps_timer_triggered || !gps_timer_set) {
    gps_start_count++;
    app_gps_init();
    if (!gps_timer_set) {
      //TA0CCR0 = 40000; //~Around 10min
      TA0CCR0 = 4000; //More like 1min
      TA0CTL = TASSEL__ACLK | MC__UP | ID_3 | TAIE_1;
      TA0CCTL0 |= CCIE;
      TA0R = 0;
    }
    write_to_log(cur_ctx,&telem_buffer_tail,sizeof(uint8_t));
    write_to_log(cur_ctx,&telem_buffer_head,sizeof(uint8_t));
    write_to_log(cur_ctx,&telem_buffer_full,sizeof(uint8_t));
    int good_gps = app_gps_gather();
    if (good_gps) {
      gps_start_count = 0;
      // Pack up and stuff data into ring buffer
      uint8_t temp_cnt = telem_buffer_tail;
      artibeus_set_telem_pkt(telem_buffer + temp_cnt);
      // For squishing into an ascii packet instead of a telem packet
      *((uint8_t *)(telem_buffer + temp_cnt)) = ASCII_TELEM;
      temp_cnt++;
      if (temp_cnt >= TELEM_BUFF_SIZE) {
        temp_cnt = 0;
      }
      // Set full flag
      telem_buffer_full = (temp_cnt == telem_buffer_head) ? 1 : 0;
      // Update tail
      telem_buffer_tail = temp_cnt;
    }
  }
  // Update latest telem pkt
  artibeus_set_telem_pkt(artibeus_latest_telem_pkt);
  return 0;
}

uint8_t * pop_telem_pkt() {
  // Return pointer to head of ring
  return &(telem_buffer[telem_buffer_tail]);
}

// A function you call _after_ the telemetry packet has definitely been
// transmitted, as long as there's only one variable, you don't have to double
// buffer it
void pop_update_telem_ptrs() {
  if (telem_buffer_head <= TELEM_BUFF_SIZE - 1) { 
    telem_buffer_head++;
  }
  else {
    telem_buffer_head = 0;
  }
}

void init_timerA0() {
  TA0CTL = TASSEL__ACLK | MC__CONTINUOUS | ID_3; 
}

void init_expt_ascii_timer() {
  TA0CCTL2 |= CCIE; 
}

