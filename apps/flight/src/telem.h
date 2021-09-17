#include <libmsp/mem.h>
#include <libartibeus/query.h>

int update_telemetry();
uint8_t * pop_telem_pkt();
void pop_update_telem_ptrs();
void init_timer_A0();
void init_expt_ascii_timer();

#define TELEM_BUFF_SIZE 64 // Total size is 2816B (44*64)
#define TELEM_REPEAT_CNT 2
extern __nv uint8_t telem_buffer_head;
extern __nv uint8_t telem_buffer_tail;
extern __nv uint8_t telem_buffer_full;
extern __nv artibeus_telem_t telem_buffer[TELEM_BUFF_SIZE];
extern __nv uint8_t gps_timer_triggered;
extern __nv uint8_t ascii_timer_triggered;

#define TELEM_PERIOD 40000
#define EXPT_ASCII_PERIOD 400

