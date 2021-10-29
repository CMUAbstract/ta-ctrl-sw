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
#include "telem.h"

//Volatile and initialized to 1 so we get telem packets even if we're experiencing frequent failures
uint8_t telem_timer_triggered = 1;

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
  write_to_log(cur_ctx,&gps_fail_count,sizeof(uint8_t));
  write_to_log(cur_ctx,&gps_timer,sizeof(uint8_t));
  // TODO check nv vars changed
  if (gps_on) {
    BIT_FLIP(1,2);
    BIT_FLIP(1,2);
    // Try to read
    int good_gps = app_gps_gather();
    if (good_gps) {
      // if fix
      gps_fail_count = 0;
      gps_timer = 0;
    }
    else {
      // if no fix, update fail count
      gps_fail_count++;
      // if fail count too high, disable gnss
      if (gps_fail_count > GPS_FAIL_MAX) {
        uartlink_close(2);
        GNSS_DISABLE;
        gps_on = 0;
        gps_timer = 0;
      }
    }
  }
  else {
    BIT_FLIP(1,2);
    BIT_FLIP(1,2);
    BIT_FLIP(1,2);
    // Check if we've waited long enough
    if (gps_timer > GPS_FAIL_WAIT) {
      // Re-init
      app_gps_init();
      gps_timer = 0;
      gps_fail_count = 0;
    }
  }
  artibeus_push_telem_pkt();
  return 0;
}


void init_timerA0() {
  TA0CCR0 = TELEM_PERIOD;
  TA0CCTL0 |= CCIE;
  TA0EX0 = 0x7; // Divide by 8
  TA0CTL = TASSEL__ACLK | MC__UP | ID_3 | TAIE;
}

void init_expt_ascii_timer() {
  TA0CCTL2 |= CCIE;
}


