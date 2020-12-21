#include <libgnss/gnss.h>

extern gps_data *cur_gps_data;
extern int fix_recorded;
extern int no_fix_counter;

int scrape_gps_buffer(void);
int gps_update(uint8_t *pkt_ptr);
