#include "SWR.h"
#include <Arduino.h>
extern long bandLimitPositionCounts[3][2];  // extern, defined in magloop_pico.cpp

SWR::SWR(StepperManagement & steppermanage, Adafruit_ILI9341 & tft): steppermanage(steppermanage), tft(tft) {
adc_init();
adc_gpio_init(26);
adc_gpio_init(27);
forward_offset = 0;
reverse_offset = 0;
//  steppermanage = steppermanage;
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
  Purpose: Manual Setting the Frequency

  Parameter list:

  Return value:
    void
*****/
void SWR::ManualFrequencyControl(int whichBandOption) {
  //Serial.print("ManualFrequencyControl  "); 
 //   updateMessage("Press Freq: Move to Freq"); TEMPORARILY COMMENTED
  int i, k, yIncrement, xIncrement;
  int stepIncr;
  long currentFrequencyOld;
  long tempTime;
  xIncrement = (XAXISEND - XAXISSTART ) / 3;
  yIncrement = (YAXISEND - YAXISSTART) / 3;
  int xDotIncrement = 10;
  int yTick = YAXISSTART + 5;
  frequencyEncoderMovement = 0;
   //Serial.print("ManualFrequencyControl2  ");
  //  GraphAxis(whichBandOption);  TEMPORARILY COMMENTED
  //Serial.print("ManualFrequencyControl3  ");
  //tempTime = millis();
  //Serial.print("frequencyEncoderMovement2=  "); Serial.println(frequencyEncoderMovement2);
     // Serial.print("frequencyEncoderMovement=  "); Serial.println(frequencyEncoderMovement);
  if (frequencyEncoderMovement2 != 0) {
    currentFrequencyOld = currentFrequency;


    while (gpio_get(FREQUENCYENCODERSWITCH) != LOW) {
      
      if (frequencyEncoderMovement2 != 0) {
        //Serial.print("if (frequencyEncoderMovement2  ");
        currentFrequency = currentFrequency + frequencyEncoderMovement2 * 1000;
        //Serial.print("if (currentFrequency  "); Serial.println(currentFrequency);
  //      UpdateFrequency(); TEMPORARILY COMMENTED
        frequencyEncoderMovement2 = 0;
        
        //delay(100);
      }
   
    }
    // updateMessage("Freq: Adjust - ATune: Refine");  TEMPORARILY COMMENTED
    tempTime = 0;
   // Serial.print("currentFrequency=  "); Serial.println(currentFrequency);
   // SendFrequency(currentFrequency);  TEMPORARILY COMMENTED
    currPosition = currPosition + ((currentFrequency - currentFrequencyOld) / (hertzPerStepperUnitVVC[whichBandOption]));
    steppermanage.MoveStepperToPositionCorrected(currPosition); //Al 4-20-20
    currPosition -= 20;
    steppermanage.MoveStepperToPositionCorrected(currPosition); //Al 4-20-20
    currPosition += 20;
    steppermanage.MoveStepperToPositionCorrected(currPosition); //Al 4-20-20
    readSWRValue = ReadSWRValue();
   // Serial.print("readSWRValue=  "); Serial.println(readSWRValue);
//    delay(100);
    int k = 0;
    frequencyEncoderMovement = 0;
    frequencyEncoderMovement2 = 0;
  }
  //updateMessage("Freq: Adjust - ATune: Refine");  TEMPORARILY COMMENTED
  //ShowSubmenuData(readSWRValue);    TEMPORARILY COMMENTED
  readSWRValue = ReadSWRValue();
  //PlotNewStartingFrequency(whichBandOption);   TEMPORARILY COMMENTED
  //ShowSubmenuData(ReadSWRValue());    TEMPORARILY COMMENTED
}

/*****
  Purpose: Manual Setting the Stepper

  Parameter list:

  Return value:
    void
*****/
void SWR::ManualStepperControl() {

  currPosition = currPosition + menuEncoderMovement;
  steppermanage.MoveStepperToPositionCorrected(currPosition); //Al 4-20-20
  currPosition -= 20;
  steppermanage.MoveStepperToPositionCorrected(currPosition); //Al 4-20-20
  currPosition += 20;
  steppermanage.MoveStepperToPositionCorrected(currPosition); //Al 4-20-20
  readSWRValue = ReadSWRValue();
  // ShowSubmenuData(ReadSWRValue()); TEMPORARILY COMMENTED
  //  UpdateFrequency();  TEMPORARILY COMMENTED
  menuEncoderMovement = 0;
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
  Purpose: To ReadNewSWRValue
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
