
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
#include <libmspuartlink/uartlink.h>

#include <libartibeus/artibeus.h>
#include <libartibeus/query.h>
#include <libgnss/gnss.h>


static uint8_t newDisable[16] = {0xA0,0xA1,0x00,0x09,0x08,0x00,0x00,
                     0x00,0x00,0x01,0x00,0x00,0x00,0x09,0x0D,0x0A};
__nv uint8_t gps_start_count = 0;

void app_gps_init() {
  GNSS_ENABLE;
  uartlink_open(2);
  // Write all sentences over to gnss
  uartlink_send_basic(2,newDisable,16);
  __delay_cycles(80000);
  // One more time for good measure
  uartlink_send_basic(2,newDisable,16);
  __delay_cycles(80000);
  return;
}

// Returns 1 if cur_gps is complete and we have a fix
int app_gps_gather() {
  if (cur_gps_data->complete && cur_gps_data->fix[0] == FIX_OK) {
    uint8_t gps_dec_buf[ARTIBEUS_GPS_SIZE];
    uint8_t time_dec_buf[ARTIBEUS_TIME_SIZE];
    uint8_t date_dec_buf[ARTIBEUS_DATE_SIZE];

    gps_dec_buf[0] = (DEGS_LAT(cur_gps_data->lat)) & 0xff;
    gps_dec_buf[1] = MIN_LAT(cur_gps_data->lat) & (0xff << 8);//TODO:remove!
    gps_dec_buf[2] = MIN_LAT(cur_gps_data->lat) & (0xff);
    gps_dec_buf[3] = (uint8_t)((SECS_LAT(cur_gps_data->lat) & (0xff << 8)) >> 8);
    gps_dec_buf[4] = SECS_LAT(cur_gps_data->lat) & (0xff);
    gps_dec_buf[5] = (uint8_t)((DEGS_LONG(cur_gps_data->longi) &
                                          (0xff << 8)) >> 8);
    gps_dec_buf[6] = DEGS_LONG(cur_gps_data->longi) & (0xff);
    gps_dec_buf[7] = (uint8_t)((MIN_LONG(cur_gps_data->longi) &
                                          (0xff << 8)) >> 8);
    gps_dec_buf[8] = MIN_LONG(cur_gps_data->longi) & (0xff);
    gps_dec_buf[9] = (uint8_t)((SECS_LONG(cur_gps_data->longi) &
                                          (0xff << 8)) >> 8);
    gps_dec_buf[10] = SECS_LONG(cur_gps_data->longi) & (0xff);
    gps_dec_buf[11] = (NS(cur_gps_data->lat) << 1 ) | EW(cur_gps_data->longi);

    time_dec_buf[0] = UTC_HRS(cur_gps_data->time);
    time_dec_buf[1] = UTC_MMS(cur_gps_data->time);
    time_dec_buf[2] = UTC_SECS(cur_gps_data->time);

    date_dec_buf[0] = DATE_MM(cur_gps_data->date);
    date_dec_buf[1] = DATE_DD(cur_gps_data->date);
    date_dec_buf[2] = DATE_YY(cur_gps_data->date);

    // Update GPS location and time
    artibeus_set_gps(gps_dec_buf);
    artibeus_set_time(time_dec_buf);
    artibeus_set_date(date_dec_buf);
    return 1;
  }
  return 0;
}
