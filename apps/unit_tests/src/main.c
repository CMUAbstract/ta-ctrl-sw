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
#include <liblsm9ds1/imu.h>
#include "gps.h"

uint8_t msg[8] = {"ARTIBEUS"};
uint8_t mailbox[16];
uint8_t mailbox1[16];
uint8_t mailbox2[260];
uint8_t adc_vals[4];

#define MAGIC_NUMBER 0xABCD

__nv uint16_t first_prog = MAGIC_NUMBER;

int main(void) {
  artibeus_init();
  // Enable Everything

  GPIO(LIBARTIBEUS_PORT_EXP_EN, DIR) |= BIT(LIBARTIBEUS_PIN_EXP_EN);
  GPIO(LIBARTIBEUS_PORT_EXP_EN, OUT) |= BIT(LIBARTIBEUS_PIN_EXP_EN);

  GPIO(LIBARTIBEUS_PORT_COMM_EN, DIR) |= BIT(LIBARTIBEUS_PIN_COMM_EN);
  GPIO(LIBARTIBEUS_PORT_COMM_EN, OUT) |= BIT(LIBARTIBEUS_PIN_COMM_EN);

  GPIO(LIBARTIBEUS_PORT_GNSS_EN, DIR) |= BIT(LIBARTIBEUS_PIN_GNSS_EN);
  GPIO(LIBARTIBEUS_PORT_GNSS_EN, OUT) |= BIT(LIBARTIBEUS_PIN_GNSS_EN);

  uartlink_open(1); // Note, uartlinks 0 and 2  get opened in artibeus init

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
    GPIO(LIBARTIBEUS_PORT_GNSS_EN, OUT) |= BIT(LIBARTIBEUS_PIN_GNSS_EN);
    GPIO(LIBARTIBEUS_PORT_GNSS_EN, DIR) |= BIT(LIBARTIBEUS_PIN_GNSS_EN);
    for (int i = 0; i < 8; i++) {
      __delay_cycles(8000000);
    }
    GPIO(LIBARTIBEUS_PORT_GNSS_EN, OUT) &= ~BIT(LIBARTIBEUS_PIN_GNSS_EN);
    // Wait 2 seconds
    __delay_cycles(16000000);
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
    uartlink_open(1);
    int count = 0;
    while(!count) {
      count = uartlink_receive_basic(0,mailbox,8);
    }
    /*
    count = 0;
    while(!count) {
      count = uartlink_receive_basic(1,mailbox1,8);
    }
    */
    mailbox[7] = '\n';
    //mailbox1[7] = '\n';
    uartlink_send_basic(1,mailbox,8);
    __delay_cycles(4000000);
    /*
    uartlink_send_basic(0,mailbox1,8);
    __delay_cycles(4000000);
    */
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
    expt_ack_check();
    __delay_cycles(4000000);
    expt_write_program();
    __delay_cycles(4000000);
    expt_write_jump();
    while(1);
    /*uartlink_open_tx(1);
    expt_write_jump();
    uartlink_close(1);
    uartlink_open_rx(1);
    while(1) {
      for(int i = 0; i < 5; i++) {
        __delay_cycles(8000000);
      }
      // Try dis
      expt_write_program();
    }*/
#endif
#ifdef TEST_MAILBOX_COMMAND_PARSE

    //Turn on expt uart
    uartlink_open(1);

    while(1){
  
      count = 0;
      //Read in first 3 bytes of command so as to retreive length of rest of command
      while(!count) {
        count = uartlink_receive_basic(0,mailbox,3);
      }

      uint8_t len_of_command = mailbox[2];
      //Added 3 for the first three bytes (start_byte0, start_byte1 and length_byte)
      uint8_t entire_len = len_of_command + 3;
      count = 0;
      //Read in entire command into buffer of reasonable length
      while(!count) {
        count = uartlink_receive_basic(0,mailbox2,entire_len);
      }
      //Check for first start byte
      if (mailbox2[0] == 0x22){
        uint8_t ack_1[23] = {"First start byte found\n"};

        uartlink_send_basic(1,ack_1,23);
      }
      
      //Check for second start byte
      if (mailbox2[1] == 0x69){
        uint8_t ack_2[24] = {"Second start byte found\n"};

        uartlink_send_basic(1,ack_2,24);
      }
      
      //Check for length byte
      uint8_t ack_len[24] = {"Length of command is:  \n"};
      ack_len[22] = len_of_command;
      uartlink_send_basic(1,ack_len,24);

      //Check for hw_id_lsb byte
      uint8_t hw_id_lsb = mailbox2[3];
      uint8_t ack_hw_id_lsb[13] = {"HW_ID_LSB:  \n"};
      ack_hw_id_lsb[11] = hw_id_lsb;
      uartlink_send_basic(1,ack_hw_id_lsb,13);

      //Check for hw_id_msb byte
      uint8_t hw_id_msb = mailbox2[4];
      uint8_t ack_hw_id_msb[13] = {"HW_ID_MSB:  \n"};
      ack_hw_id_msb[11] = hw_id_msb;
      uartlink_send_basic(1,ack_hw_id_msb,13);

      //Check for msg_id_lsb byte
      uint8_t msg_id_lsb = mailbox2[5];
      uint8_t ack_msg_id_lsb[14] = {"MSG_ID_LSB:  \n"};
      ack_msg_id_lsb[12] = msg_id_lsb;
      uartlink_send_basic(1,ack_msg_id_lsb,14);

      //Check for msg_id_msb byte
      uint8_t msg_id_msb = mailbox2[6];
      uint8_t ack_msg_id_msb[14] = {"MSG_ID_MSB:  \n"};
      ack_msg_id_msb[12] = msg_id_msb;
      uartlink_send_basic(1,ack_msg_id_msb,14);

      //Sends over entire command
      uint8_t ack_comm[21] = {"Entire Command sent:\n"};
      uartlink_send_basic(1,ack_comm,21);
      uartlink_send_basic(1,mailbox2,entire_len);


      __delay_cycles(4000000);

    }


#endif
#ifdef TEST_EXPT_SET_TIME
  uartlink_open(1);
  uint32_t time = 675379830;
  uint8_t decimal[4] = {0x30, 0x75, 0x00, 0x00};
  uint8_t buff[16];
  while(1) {
    uint8_t new_time[8];
   // Start on
   uint32_t mask = 0xFF;
   uint32_t shift = 0;
    for (int i = 0; i < 4; i++) {
      new_time[i] = (time & mask) >> shift;
      shift += 8;
      mask = mask << 8;
    }
    for (int i = 0; i < 4; i++) {
      new_time[4 + i] = decimal[i];
    }
    expt_set_time(new_time);
   // Increment time
   time++;
   // Read in from expt board
    int count = 0;
    //Read in first 3 bytes of command so as to retreive length of rest of command
    while(!count) {
      count = uartlink_receive_basic(1,buff,9);
    }
    if (count  > 3) {
      if (buff[8] == ACK) {
        // End program if we receive an ack in response to a set time
        expt_ack_check();
        while(1);
      }
    }
    // Delay 1 second
    __delay_cycles(8000000);
  }
#endif //Testing expt set time
#ifdef TEST_IMU
  // Test imu functionality
  int temp = -1;
  while(temp < 0) {
    PRINTF("Temp is: %i\r\n",temp);
    temp = init_lsm9ds1();
  }

  while(1) {
    // Read from g,xl,m and print results
    int16_t x,y,z;
    read_g(&x,&y,&z);
    PRINTF("Gyro: X: %i, Y: %i, Z: %i\r\n",x,y,z);
    read_xl(&x,&y,&z);
    PRINTF("Accel: X: %i, Y: %i, Z: %i\r\n",x,y,z);
    read_m(&x,&y,&z);
    PRINTF("Mag: X: %i, Y: %i, Z: %i\r\n",x,y,z);
    // Delay 1 second
    __delay_cycles(8000000);
  }
#endif // testing imu
#ifdef TEST_WORMHOLE
  EXP_ENABLE;
  COMM_ENABLE; // Required for cntrl board v0 where we mixed up power rails
  uartlink_open(1);
  while(1);
#endif
}




