#include <libgnss/gnss.h>


void app_gps_init();

int app_gps_gather();

#define GPS_FAIL_MAX 0x2
#define GPS_FAIL_WAIT 0xa

extern __nv uint8_t gps_fail_count;
extern __nv uint8_t gps_timer;

extern uint8_t gps_on;

