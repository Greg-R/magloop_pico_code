#include "SWR.h"
#include <Arduino.h>

//extern long bandLimitPositionCounts[3][2];  // extern, defined in magloop_pico.cpp
extern int menuEncoderMovement;
extern int frequencyEncoderMovement;
extern int frequencyEncoderMovement2;
extern int digitEncoderMovement;

SWR::SWR() {
adc_init();
adc_gpio_init(26);
adc_gpio_init(27);
forward_offset = 0;
reverse_offset = 0;
}

/*
Read and store the ADC offset values.  This must be done with the DDS off.

*/
void SWR::ReadADCoffsets(){
  adc_select_input(0);
  adc_read();
  adc_select_input(1);
  adc_read();
  busy_wait_ms(1000);
  adc_select_input(0);
  reverse_offset = adc_read();
  busy_wait_ms(1000);
  adc_select_input(1);
  forward_offset = adc_read();
}


/*****
  Purpose: To read one bridge measurement

  Parameter list:
  void

  Return value:
  int           the swr * 1000 so it comes back as an int

  CAUTION: Assumes that frequency has already been set
  ALTERNATIVE CALCULATION (Untested ?)
    p = sqrt( ((float)REV) / FWD );   // Calculate reflection coefficient
    VSWR = (1 + p) / (1 - p);         // Calculate VSWR
*****/
float SWR::ReadSWRValue()
{
  int i;
  float sum[2] = {0.0, 0.0};

  float FWD = 0.0;
  float REV = 0.0;
  float VSWR;
  for (i = 0; i < MAXPOINTSPERSAMPLE; i++) {             // Take multiple samples at each frequency
    busy_wait_ms(20);
    adc_select_input(1);
    sum[0] += (float) adc_read(); // - (float) forward_offset;  // Read forward voltage.
    busy_wait_ms(20);
    adc_select_input(0);
    sum[1] += (float) adc_read(); // - (float) reverse_offset;  //  Read reverse voltage.
  }
  forward_voltage = sum[0] / (float) MAXPOINTSPERSAMPLE - (float) forward_offset;
  reverse_voltage = sum[1] / (float) MAXPOINTSPERSAMPLE - (float) reverse_offset;
  //REV = REV+SWRREVOFFSET; 
  if (reverse_voltage >= forward_voltage) {
    VSWR = 999.0;                               // To avoid a divide by zero or negative VSWR then set to max 999
  } else {
    VSWR = ((forward_voltage + reverse_voltage) / (forward_voltage - reverse_voltage));         // Calculate VSWR
  }
  static float sNow = 1000.000, sLast = 1000.000;
  if (VSWR < 999.0) {
    sNow = VSWR;
    if (sNow < sLast) {
      sLast = sNow;
    }
  }

return VSWR;
}

/*****
  Purpose: To ReadNewSWRValue.  What is different???
  Parameter list:
  void
  return (VSWR)

*****/
float SWR::ReadNewSWRValue()
{
  int i;
  int sum[2] = {0, 0};
  int FWD = 0;
  int REV = 0;
  float VSWR;
  // Needs to be updated to Pi Pico:
//  for (i = 0; i < MAXPOINTSPERSAMPLE; i++) {             // Take multiple samples at each frequency
//    sum[0] += analogRead(ANALOGFORWARD);
 //   sum[1] += analogRead(ANALOGREFLECTED);
//  }
  FWD = sum[0] / MAXPOINTSPERSAMPLE;
  REV = sum[1] / MAXPOINTSPERSAMPLE;

  if (REV >= FWD) {
    VSWR = 999.0;                               // To avoid a divide by zero or negative VSWR then set to max 999
  } else {
    VSWR = ((float) (FWD + REV)) / ( (float) (FWD - REV));        // Calculate VSWR
  }

  return (VSWR);

}
