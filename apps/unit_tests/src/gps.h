#include <libgnss/gnss.h>

extern gps_data *cur_gps_data;

void scrape_gps_buffer(void);
int gps_update(uint8_t *pkt_ptr);
