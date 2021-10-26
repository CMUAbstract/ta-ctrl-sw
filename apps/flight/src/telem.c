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
    BIT_FLIP(1,1);
    BIT_FLIP(1,1);
    BIT_FLIP(1,1);
    BIT_FLIP(1,1);
  }
  else if (gps_timer_triggered || !gps_timer_set) {
    gps_start_count++;
    app_gps_init();
    if (!gps_timer_set) {
      //TA0CCR0 = 40000; //~Around 10min
      TA0CCR0 = TELEM_PERIOD; //More like 1min
      TA0CTL = TASSEL__ACLK | MC__UP | ID_3 | TAIE_1;
      TA0CCTL0 |= CCIE;
      TA0R = 0;
    }
    int good_gps = app_gps_gather();
    if (good_gps) {
      // Pack up and stuff data into ring buffer
      BIT_FLIP(1,1);
      BIT_FLIP(1,1);
      artibeus_push_telem_pkt();
      gps_start_count = 0;
    }
  }
  else {
    // Update latest telem pkt
      BIT_FLIP(1,1);
      BIT_FLIP(1,1);
      BIT_FLIP(1,1);
    artibeus_set_telem_pkt(artibeus_latest_telem_pkt);
  }
  return 0;
}


void init_timerA0() {
  TA0CCR0 = TELEM_PERIOD;
  TA0CCTL0 |= CCIE;
  TA0CTL = TASSEL__ACLK | MC__UP | ID_3 | TAIE;
}

void init_expt_ascii_timer() {
  TA0CCTL2 |= CCIE;
}


