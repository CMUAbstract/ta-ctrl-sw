#include <msp430.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stddef.h>
#include <libmsp/mem.h>

#include "pkts.h"
#include "libartibeus/comm.h"

__nv uint8_t pkt_buffer[PKT_BUFFER_SIZE];
__nv uint8_t *pkt_ptr = pkt_buffer;

__nv uint8_t ack_buff[ACK_BUFFER_SIZE];

int is_bootloader_ack(uint8_t *buff) {
  int ack = 0;
  if (buff[8] == BOOTLOADER_ACK) {
    ack = 1; 
  }
  else {
    ack = 0;
  }
  // Clear buffer
  for (int i = 0; i < ACK_BUFFER_SIZE; i++) {
    buff[i] = 0;
  }
  return ack;
}

