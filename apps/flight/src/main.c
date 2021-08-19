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
#include <libmsp/sleep.h>


#include <libartibeus/artibeus.h>
#include <libartibeus/comm.h>
#include <libads/ads1115.h>
#include <libmspuartlink/uartlink.h>
#include <libgnss/gnss.h>

#include "main.h"
#include "pkts.h"
#include "adc.h"
#include "gps.h"
#include "telem.h"

#define SLEEP_DELAY 36864 //~9 sec at 32768/8 Hz ticks --> need to loop 10x

__nv uint8_t first_init = 1;
__nv uint8_t solar_check = 0;

#define MAGIC_NUMBER 0xABCD

__nv uint16_t first_prog = MAGIC_NUMBER;

__nv artibeus_ctx ctx0 = { ADC, -1 };
__nv artibeus_ctx ctx1 = { ADC, -1 };
__nv artibeus_ctx *cur_ctx = &ctx0;

// We're only going to reference these here
static __nv uint8_t backup_data[BACKUP_DATA_LEN]; // Buffer where we'll put data
// Data structure for each double buffered entry
static __nv dbl_buffer_entry double_buffer[DBL_BUFF_LEN];



int main(void) {
  // Make sure we hold after programming
  // Pull this pin to ground externally BEFORE programming
  if (first_init) {
    artibeus_first_init();
    first_init = 0;
  }
  else {
    artibeus_init();
  }
  int count;
  // Clear GPS variables
  fix_recorded = 0;
  no_fix_counter = 0;
  // Clear transfer variables
  comm_expt_link.transfer_len = 0;
  comm_expt_link.status = TRANSFER_DONE;
  // Restore any corrupted data
  restore_from_backup(cur_ctx);
  // Scheduler loop!
  while(1) {
    locations last_location;
    uint16_t Vcap;
    artibeus_mode next_task;
    // Pet watchdog
    msp_watchdog_kick();
    switch (cur_ctx->cur_task) {
      case(RECORD_TELEM):{
        update_telemetry();
        next_task = GET_UART1;
        break;
      }
      case(GET_UART1):{

        next_task = GET_UART2;
        break;
      }
      case(GET_UART2):{

        next_task = RECORD_TELEM;
        break;
      }
    }
    // Safely transition
    artibeus_ctx *next_ctx;
    next_ctx = (cur_ctx == &ctx0) ? &ctx1 : &ctx0;
    next_ctx->dbl_buffer_count = -1;
    next_ctx->cur_task = next_task;
    // Swap
    cur_ctx = next_ctx;
  }
}


void restore_from_backup(artibeus_ctx *ctx) {
  PRINTF("Got %i items to restore\r\n",ctx->dbl_buffer_count);
  while(ctx->dbl_buffer_count > -1) {
    // Memcpy data
    PRINTF("Restoring!\r\n");
    uint8_t *source = double_buffer[ctx->dbl_buffer_count].source;
    uint8_t *dest = double_buffer[ctx->dbl_buffer_count].dest;
    size_t len = double_buffer[ctx->dbl_buffer_count].len;
    memcpy(source, dest, len);
    ctx->dbl_buffer_count--;
  }
  return;
}

int write_to_log(artibeus_ctx *ctx, uint8_t *data, size_t len) {
  int16_t index = ctx->dbl_buffer_count;
  index++;
  if (index > DBL_BUFF_LEN) {
    return -1;
  }
  PRINTF("Writing! %i, len = %i\r\n", *data, len);
  // Get address of last entry in buffer
  uint8_t *last_addr;
  if (index > 0) {
    last_addr = double_buffer[index-1].dest + double_buffer[index-1].len;
  }
  else {
    // Set address to start of buffer if first entry
    last_addr = backup_data;
  }
  // Add one to get to very next available address
  last_addr++;
  memcpy(last_addr, data, len);
  // This is fine because we only log __nv data
  double_buffer[index].source = data;
  double_buffer[index].dest = last_addr;
  double_buffer[index].len = len;
  ctx->dbl_buffer_count++;
  return 0;
}
