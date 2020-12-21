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


#include <libartibeus/artibeus.h>
#include <libads/ads1115.h>

//#define DUMMY_ADS


// Since we write to cur_pkt_ptr, we need to log the value before we run this
// function
int adc_update(uint8_t *cur_pkt_ptr) {
    uint8_t *ptr = cur_pkt_ptr;
    uint16_t temp;
    for(int i = 0; i < LIBADS_NUM_CHANNELS; i++) {
#ifndef DUMMY_ADS
      temp = ads_se_read(i);
#else
      temp = 0xABCD + i;
#endif
      *ptr++ = temp >> 8;
      *ptr++ = temp & 0xFF;
    }
    return 0;
}

uint16_t get_vcap() {
  uint16_t temp;
#ifndef DUMMY_ADS
  temp = ads_se_read(3);
  temp = temp >> 3;
#else
  temp = 5;
#endif
  return temp;
}


