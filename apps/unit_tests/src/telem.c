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
#include <libads/ads1115.h>
#include <libmspuartlink/uartlink.h>
#include <libgnss/gnss.h>
#include <liblsm9ds1/imu.h>
#include "gps.h"
#include "adc.h"

// leave in volatile memory
uint16_t gps_timer_triggered = 0;
//TODO double check the procedure for need_fix
int need_fix = -1;
// Self contained telemetry function that gathers data, updates the most recent
// query-able data elements, and updates the packets that will get transmitted.
int update_telemetry() {
  // Check GPS timer and set up if necessary
  uint16_t gps_timer_set = TA0CCTL0 | CCIE;
  if (gps_timer_triggered || !gps_timer_set) {
    if (!gps_timer_set) {
      TA0CCR0 = 40000; //~Around 10min
      TA0CTL = TASSEL__ACLK | MC__UP | ID_3 | TAIE_1;
      TA0CCTL0 |= CCIE;
      TA0R = 0;
    }
    // Collect GPS data if it's time
    //TODO pack this into a nice function
    do {
      uint16_t Vcap = get_vcap();
      if (!fix_recorded &&  Vcap < 2800) { break;}
      if (fix_recorded && Vcap < 1900) { break;}
      GNSS_ENABLE;
      uint8_t gps_loc[48];
      for(int i = 0; i < 90; i++) {
        // Wait one sec
        __delay_cycles(8000000);
        need_fix = scrape_gps_buffer();
        if (!need_fix) {
          fix_recorded = 1;
          no_fix_counter = 0;
          break;
        }
        // Increment no_fix count if we were supposed to have a fix but lost it
        if (i == 10 && fix_recorded) {
          no_fix_counter += 1;
        }
      }
      if (!need_fix) {
        gps_update(gps_loc);
        //last_location = good_location(gps_loc);
      }
      else {
        //last_location = NOWHERE;
        // Reset GPS if:
        // we still don't have a fix after 60s
        // or we tried 3 times to get a fix and still don't have it.
        if (!fix_recorded || (no_fix_counter > 3)) {
          RESET_GNSS;
        }
      }
      GNSS_DISABLE; //TODO do we really want this?
      uint8_t gps_dec_buf[ARTIBEUS_GPS_SIZE];
      uint8_t time_dec_buf[ARTIBEUS_TIME_SIZE];
      //TODO check this casting
      gps_dec_buf[0] = DEGS_LAT(cur_gps_data->lat);
      gps_dec_buf[2] = MIN_LAT(cur_gps_data->lat);
      gps_dec_buf[4] = SECS_LAT(cur_gps_data->lat);
      gps_dec_buf[6] = DEGS_LONG(cur_gps_data->longi);
      gps_dec_buf[8] = MIN_LONG(cur_gps_data->longi);
      gps_dec_buf[10] = SECS_LONG(cur_gps_data->longi);
      gps_dec_buf[11] = (NS(cur_gps_data->lat) << 1 ) & EW(cur_gps_data->longi);
      time_dec_buf[0] = UTC_HRS(cur_gps_data->time);
      time_dec_buf[1] = UTC_MMS(cur_gps_data->time);
      time_dec_buf[2] = UTC_SECS(cur_gps_data->time);
      // Update GPS location and time
      artibeus_set_gps(gps_dec_buf);
      artibeus_set_time(time_dec_buf);
      gps_timer_triggered = 0;
    } while(0);
  }
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
  return 0;
}


void __attribute ((interrupt(TIMER0_A0_VECTOR))) Timer0_A0_ISR(void)
{ //Handles overflows
  __disable_interrupt();
  gps_timer_triggered = 1;
  TA0R = 0;
  __enable_interrupt();// A little paranoia over comp_e getting thrown
  //Note: shutdown interrupt in librustic will handle grabbing final TA0R,
  //because COMP_E has high priority than TimerA, TimerA will never preempt
  //COMP_E and put TA0R and __paca_global_ovfl out of whack
}

