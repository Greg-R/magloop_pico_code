
#include "DDS.h"

// The routines in this file are use to interface with the AD9850 DDS in stead of using a library


DDS::DDS(uint DDS_RST, uint DDS_DATA, uint DDS_FQ_UD, uint DDS_WCLK): RESET(DDS_RST), DATA(DDS_DATA), FQ_UD(DDS_FQ_UD), WCLK(WCLK) {
gpio_set_function( RESET, GPIO_FUNC_SIO);
gpio_set_function( DATA, GPIO_FUNC_SIO);
gpio_set_function( FQ_UD, GPIO_FUNC_SIO);
gpio_set_function( WCLK, GPIO_FUNC_SIO);
gpio_set_dir( RESET, GPIO_OUT);
gpio_set_dir( DATA, GPIO_OUT);
gpio_set_dir( FQ_UD, GPIO_OUT);
gpio_set_dir( WCLK, GPIO_OUT);
}

void DDS::DDSWakeUp() {          //initialize DDS
  gpio_put(WCLK, 1);
  busy_wait_us_32(200);
  gpio_put(WCLK, 0);

  gpio_put(RESET, 1);
  busy_wait_us_32(200);
  gpio_put(RESET, 0);

  gpio_put(FQ_UD, 1);
  busy_wait_us_32(200);
  gpio_put(FQ_UD, 0);
  busy_wait_us_32(200);
}

void DDS::outOne() {
  busy_wait_us_32(200);
  gpio_put (WCLK, 0);
  busy_wait_us_32(200);
  gpio_put (DATA, 1);
  busy_wait_us_32(200);
  gpio_put (WCLK, 1);
  busy_wait_us_32(200);
  gpio_put (DATA, 0);
  busy_wait_us_32(200);
}

void DDS::outZero() {
  busy_wait_us_32(200);
  gpio_put (WCLK, 0);
  busy_wait_us_32(200);
  gpio_put (DATA, 0);
  busy_wait_us_32(200);
  gpio_put (WCLK, 1);
  busy_wait_us_32(200);
}

void DDS::byte_out(unsigned char byte) {
  int i;
  for (i = 0; i < 8; ++i) {
    if ((byte & 1) == 1)
      outOne();
    else
      outZero();
    byte = byte >> 1;
  }
}

void DDS::SendFrequency(long frequency) {   //Set DDS frequency

  long freq = frequency * 4294967295L / 125000000L; // note 125 MHz clock on 9850  Can be used to calibrate individual DDS
  gpio_put (FQ_UD, 0);
  for (int b = 0; b < 4; b++, freq >>= 8)
    byte_out (freq & 0xFF);
  byte_out (0x00);   // Final control byte, all 0 for 9850 chip
  gpio_put (FQ_UD, 1);  // Done!  Should see output
}
