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

#include "gps.h"

uint8_t msg[8] = {"ARTIBEUS"};
uint8_t mailbox[16];
uint8_t mailbox1[16];
uint8_t adc_vals[4];

#define MAGIC_NUMBER 0xABCD

__nv uint16_t first_prog = MAGIC_NUMBER;

int main(void) {
  artibeus_init();
  // Enable
  /*(GPIO(LIBARTIBEUS_PORT_EXP_EN, DIR) |= BIT(LIBARTIBEUS_PIN_EXP_EN);
  GPIO(LIBARTIBEUS_PORT_EXP_EN, OUT) |= BIT(LIBARTIBEUS_PIN_EXP_EN);

  GPIO(LIBARTIBEUS_PORT_COMM_EN, DIR) |= BIT(LIBARTIBEUS_PIN_COMM_EN);
  GPIO(LIBARTIBEUS_PORT_COMM_EN, OUT) |= BIT(LIBARTIBEUS_PIN_COMM_EN);

  GPIO(LIBARTIBEUS_PORT_GNSS_EN, DIR) |= BIT(LIBARTIBEUS_PIN_GNSS_EN);
  GPIO(LIBARTIBEUS_PORT_GNSS_EN, OUT) |= BIT(LIBARTIBEUS_PIN_GNSS_EN);
  */
  int count = 0;
  msg[0] = 'E';
  msg[1] = 'F';
  msg[2] = 'G';
  msg[3] = 'H';
  msg[4] = ' ';
  msg[5] = ' ';
  msg[6] = ' ';
  msg[7] = '\n';

#ifdef TEST_POWER_ON //Load program test
  // Make sure we correctly turn _off_ the GNSS enable
  while(1) {
    // Wait 8 seconds
    for (int i = 0; i < 8; i++) {
      __delay_cycles(8000000);
    }
    GPIO(LIBARTIBEUS_PORT_GNSS_EN, OUT) &= ~BIT(LIBARTIBEUS_PIN_GNSS_EN);
    // Wait 3 ms
    __delay_cycles(24000);
  }
#endif
#ifdef TEST_ECHO
  // Single uart RX and TX
  while(1) {
    uartlink_send_basic(0,msg,8);
    __delay_cycles(4000000);
    int count = 0;
    while(!count) {
      count = uartlink_receive_basic(0,mailbox,8);
    }
    for (int i = 0; i < count; i++) {
      msg[i] = mailbox[i];
    }
    msg[count] = count;
    msg[7] = '\n';
    __delay_cycles(400000);
  }
#endif//single uart rx/tx
#ifdef TEST_UART_ADC
  // Single uart rx/tx and adc
  while(1) {
    uartlink_send_basic(0,msg,8);
    uint16_t temp;
    temp = ads_se_read(0);
    // Shift left by 3 bits so that we can get translation to mV
    temp = temp >> 3;
    //Start
    adc_vals[0] = ' ';
    // MSB
    adc_vals[1] = temp >> 8;
    // LSB
    adc_vals[2] = temp & 0xFF;
    // Endline
    adc_vals[3] = '\n';
    uartlink_send_basic(0,adc_vals,4);
    __delay_cycles(4000000);
    int count = 0;
    while(!count) {
      count = uartlink_receive_basic(0,mailbox,8);
    }
    for (int i = 0; i < count; i++) {
      msg[i] = mailbox[i];
    }
    msg[count] = count;
    msg[7] = '\n';
    __delay_cycles(400000);
  }
#endif //single uart rx/tx and adc

#ifdef TEST_MULTI_UART_TX
  // Multiple uarts tx'ing
  while(1) {
    uartlink_send_basic(0, msg, 8);
    __delay_cycles(4000000);
    for (int i = 0; i < 4; i++) {
      msg[i]++;
    }
    uartlink_send_basic(1, msg, 8);
    __delay_cycles(4000000);
    for (int i = 0; i < 4; i++) {
      msg[i]++;
    }
    uartlink_send_basic(2, msg, 8);
    __delay_cycles(4000000);
    msg[0] = 'A';
    msg[1] = 'B';
    msg[2] = 'C';
    msg[3] = 'D';
  }
#endif // multiple uarts tx
#ifdef TEST_MAILBOX
  while(1) {
    int count = 0;
    while(!count) {
      count = uartlink_receive_basic(0,mailbox,8);
    }
    count = 0;
    while(!count) {
      count = uartlink_receive_basic(1,mailbox1,8);
    }
    mailbox[7] = '\n';
    mailbox1[7] = '\n';
    uartlink_send_basic(0,mailbox,8);
    __delay_cycles(4000000);
    uartlink_send_basic(0,mailbox1,8);
    __delay_cycles(4000000);
  }
#endif // multiple UARTS TX/RX
#ifdef TEST_ADC
  // Testing adc
  while(1) {
    // Wait 8 seconds
    for (int i = 0; i < 20; i++) {
      //PRINTF("Reading ads! %i\r\n",i);
      uint16_t temp;
      temp = ads_se_read(0);
      // Shift left by 3 bits so that we can get translation to mV
      temp = temp >> 3;
      PRINTF("Temp val is: %x\r\n",temp);
      __delay_cycles(800000);
    }
    // Disable
    GPIO(LIBARTIBEUS_PORT_EXP_EN, OUT) &= ~BIT(LIBARTIBEUS_PIN_EXP_EN);
    // Wait 3 ms
    __delay_cycles(24000);
  }
#endif // testing adc

#ifdef TEST_ACKS
  // Comm board ACK test
  EXP_ENABLE;
  COMM_ENABLE;
  char msg[5] = "Good\n";
  char msg1[5] = "Bad!\n";
  while(1) {
    unsigned count;
    //uartlink_send_basic(1,msg,5);
    //uartlink_send_basic(0,msg,5);

    count = expt_ack_check();
    count = comm_ack_check();
    if (count > 0) {
      P1OUT |= BIT0;
      P1DIR |= BIT0;
      P1OUT &= ~BIT0;
      uartlink_send_basic(0,msg,5);
    }
    else {
      uartlink_send_basic(0,msg1,5);
    }
    __delay_cycles(8000000);
  }
#endif
#ifdef TEST_COMM_REPEAT
  while(1) {
    COMM_ENABLE;
      P1OUT |= BIT1;
      P1DIR |= BIT1;
      P1OUT |= BIT2;
      P1DIR |= BIT2;
    for(int i = 0; i < 2; i++) {
      __delay_cycles(8000000);//TODO figure out time to turn on
    }
    comm_rf_check();
    __delay_cycles(8000000);
    COMM_DISABLE;
    for (int i = 0; i < 6; i++) {
      __delay_cycles(8000000);
    }
  }
#endif

#ifdef TEST_COMM_TX_LONG
  char long_pkt[0];
  for (int i =0 ;i < 1024; i++) {
    long_pkt[i] = 'A';
  }
  while(1) {
    P1OUT |= BIT0;
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;
    comm_transmit_pkt(long_pkt, 1024);
    for (int i = 0; i < 2; i++) {
      __delay_cycles(8000000);
    }
  }
#endif

#ifdef TEST_BURN_WIRE
  //Burn check before we update the burn pin...
    while(1) {
      P1OUT |= BIT0;
      P1DIR |= BIT0;
      __delay_cycles(8000000);
      P1OUT &= ~BIT0;
    }
#endif
#ifdef TEST_GNSS
    // GNSS check
    char buffer[48];
    GNSS_ENABLE;
    uartlink_open(2);
    P2OUT &= ~BIT3;
    P2DIR |= BIT3;
    __delay_cycles(800000);
    P2DIR &= ~BIT3;
    for(int i = 0; i < 60; i++) {
      __delay_cycles(8000000);
      PRINTF("Waiting!\r\n");
    }
    while(1) {
      LOG("Scraping new buffer\r\n");
      scrape_gps_buffer();
      gps_update(buffer);
      PRINTF("\r\n-->");
      for(int i = 0; i < 48; i++) {
        PRINTF("%c",buffer[i]);
      }
      __delay_cycles(8000000);
    }
#endif
#ifdef TEST_XFER
    // Turn on EXPT board to dump bytes
    EXP_ENABLE;
    COMM_ENABLE; // Required for cntrl board v0 where we mixed up power rails
    while(1) {
      expt_ack_check();
      __delay_cycles(4000000);
    }
    uartlink_open_tx(1);
    expt_write_jump();
    uartlink_close(1);
    uartlink_open_rx(1);
    while(1) {
      for(int i = 0; i < 5; i++) {
        __delay_cycles(8000000);
      }
      // Try dis
      expt_write_program();
    }
#endif
}


