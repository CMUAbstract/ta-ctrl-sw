#ifndef _APP_ADC_H_
#define _APP_ADC_H_

#include <msp430.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stddef.h>

int adc_update(uint8_t *cur_pkt_ptr);
uint16_t get_vcap();

#endif
