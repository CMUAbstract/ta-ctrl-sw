#include <libgnss/gnss.h>


void app_gps_init();

int app_gps_gather();

#define GPS_FAIL_MAX 0xf
extern __nv uint8_t gps_start_count;

