#ifndef _APP_PKTS_H_
#define _APP_PKTS_H_

#include <msp430.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stddef.h>

// All the data we're going to store

#define PKT_BUFFER_SIZE 512
#define ACK_BUFFER_SIZE 10

#define CHECK_PKT_SPACE(len) \
  do() { pkt_ptr + len < pkt_buffer + \
         PKT_BUFFER_SIZE } while(0);

extern uint8_t pkt_buffer[PKT_BUFFER_SIZE];
extern uint8_t *pkt_ptr;

extern uint8_t ack_buff[ACK_BUFFER_SIZE];

// Returns 1 if expt returns a bootloader ack, returns 0 if not
// clears buff
int is_bootloader_ack(uint8_t *buff);


#endif // _APP_PKTS_H_
