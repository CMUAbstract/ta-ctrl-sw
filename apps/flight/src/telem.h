#include <libmsp/mem.h>
#include <libartibeus/query.h>

int update_telemetry();
uint8_t * pop_telem_pkt();
void pop_update_telem_ptrs();
void init_timer_A0();
void init_expt_ascii_timer();

#define TELEM_REPEAT_CNT 2
extern __nv uint8_t telem_timer_triggered;

//#define TELEM_PERIOD 0x8700 // ~60 seconds
#define TELEM_PERIOD 3456 //TODO temporary!

