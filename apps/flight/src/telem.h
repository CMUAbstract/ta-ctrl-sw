#include <libmsp/mem.h>
#include <libartibeus/query.h>

int update_telemetry();

#define TELEM_BUFF_SIZE 64 // Total size is 2816B (44*64)
extern __nv uint8_t telem_buffer_head;
extern __nv uint8_t telem_buffer_tail;
extern __nv uint8_t telem_buffer_full;
extern __nv artibeus_telem_t telem_buffer[TELEM_BUFF_SIZE];



