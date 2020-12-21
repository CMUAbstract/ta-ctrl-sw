
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
#include <libmspuartlink/uartlink.h>

#include <libartibeus/artibeus.h>
#include <libgnss/gnss.h>

__nv uint8_t gps_uart_payload[512];

//TODO figure out if we're handling this correctly for power failure
__nv gps_data gps_data1 = { {0}, {0}, {0}, 0, 0};
__nv gps_data gps_data2 = { {0}, {0}, {0}, 0, 0};
__nv gps_data *cur_gps_data = &gps_data1;

int time_compare(gps_data *newer, gps_data *older);

// Write after read on cur_gps_data
void scrape_gps_buffer() {
  //TODO make this more efficient
  unsigned buff_count;
  // Since all uart data is volatile, we don't need to worry about WARs in this
  buff_count = uartlink_receive_basic(2,gps_uart_payload,511);
  if (buff_count < FULL_SENTENCE_LEN) {
    LOG("Too short! got %i\r\n", buff_count);
    return;
  }
  // Zip through  uart buffer
  for(int i = 0; i < buff_count; i++) {
    char data;
    int pkt_done = 0;
    int pkt_error = 1;
    data = gps_uart_payload[i];
    LOG("got: %c, cout: %i \r\n",data,gnss_pkt_counter - 5);
    if (data == '$') {
      gnss_pkt_counter = 0;
      pkt_type = IN_PROGRESS;
      active_pkt = 0;
      LOG("Starting!\r\n");
      continue;
    }
    else if (pkt_type == IN_PROGRESS) {
      // Try to find packet header
      LOG("finding type\r\n");
      get_sentence_type(data);
      continue;
    }
    else if (active_pkt) {
      LOG("Active pkt!\r\n");
      pkt_done = get_sentence_pkt(data);
      if (!pkt_done) {
        continue;
      }
    }
    else {
      LOG("Searching...\r\n");
      continue;
    }
    gps_data *next_gps_data;
    if (pkt_done) {
      LOG("Swapping!\r\n");
      active_pkt = 0;
      gnss_pkt_counter = 0;
      next_gps_data = (cur_gps_data == &gps_data1) ?
        &gps_data2 : &gps_data1;
      pkt_error = process_sentence_pkt(cur_gnss_ptr, next_gps_data);
    }
    if (!pkt_error) {
      //Update latest gps coordinates if time is newer
      if (time_compare(next_gps_data, cur_gps_data) > 0) {
        cur_gps_data = next_gps_data;
      }
      else {
        LOG("no swap-- no time\r\n");
      }
    }
    else {
      LOG("Pkt error!\r\n");
    }
  }
  return;
}

int gps_update(uint8_t *cur_pkt_ptr) {
  //Put data in buffer
  gps_data *data_out;
  if (cur_gps_data->complete) {
    data_out = cur_gps_data;
  }
  else {
    data_out = (cur_gps_data == &gps_data1) ? &gps_data2 : &gps_data1;
  }
  for(int i = 0; i < 13; i++) {
    *cur_pkt_ptr++ = data_out->time[i];
  }
  for(int i = 0; i < 10; i++) {
    *cur_pkt_ptr++ = data_out->lat[i];
  }
  for(int i = 0; i < 11; i++) {
    *cur_pkt_ptr++ = data_out->longi[i];
  }
  *cur_pkt_ptr++ = data_out->fix[0];
  return 0;
}

// Returns 1 if  if  newer time > older time
// Returns 0 if newer time == older time
// Returns -1 if newer time < older time
int time_compare(gps_data *newer, gps_data *older) {
  // All times are %6.6f format, but we're only checking whole seconds at the
  // moment
  for(int i = 0; i < 6; i++) {
    if (newer->time[i] > older->time[i]) {
      return 1;
    }
    if (older->time[i] > newer->time[i]) {
      return -1;
    }
  }
  // Again, we're not checking after the "."
  return 0;
}
