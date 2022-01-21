
#include "Calibrate.h"


Calibrate::Calibrate(DisplayManagement & display, StepperManagement & stepper, Adafruit_ILI9341 & tft,
                     DDS & dds, SWR & swr, AutoTune & autotune): display(display), stepper(stepper), tft(tft),
                     dds(dds), swr(swr), autotune(autotune) {}

//Calibrate(DisplayManagement & display, AccelStepper & stepper, StepperManagement & steppermanage, Adafruit_ILI9341 & tft, Calibrate & calibrate);

/*****
  Purpose: To set the band end point counts
           Uses initial Band Edge counts for rapid re-calibrate
  Argument list:
    void

  Return value:
    void
*****/
void Calibrate::DoNewCalibrate2()  //Al modified 9-14-19
{
  int bandBeingCalculated;
  int i, j, whichLine;
  long localPosition, minCount;
  float currentSWR;
  //display.updateMessage("Full Calibrate"); TEMPORARILY COMMENTED
  bandBeingCalculated = 0;
  stepper.ResetStepperToZero();                  //Start off at zero
  stepper.setCurrentPosition(0);        //Set Stepper count to zero
  currPosition = 100L;
  stepper.runToNewPosition(currPosition);
  tft.fillRect(0, 46, 340, 231, ILI9341_BLACK);
  tft.drawFastHLine(0, 45, 320, ILI9341_RED);
  tft.setFont(&FreeSerif9pt7b);
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  tft.setCursor(0, 65);
  tft.print("Frequency               SWR       Count");      // Table header
  tft.setCursor(0, 90);                       // Read to show mins...

  whichLine = 0;                              // X coord for mins
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK); // Table data

  for (i = 0; i < MAXBANDS; i++) {            // For the 3 bands...
    for (j = 0; j < 2; j++) {
#ifdef DEBUG                          // So we can see the EEPROM values
      Serial.print("i=  "); Serial.println(i);
      Serial.print("j=  "); Serial.println(j);                   // So we can see the EEPROM values
#endif
      currentFrequency = bandEdges[i][j];     // Select a band edge
      currPosition = bandLimitPositionCounts[i][j] - 200;       //Set Band limit count -200 counts to approach band limit from CW direction
      dds.SendFrequency(currentFrequency);        // Tell the DDS the edge frequency...
      busy_wait_us_32(100L);
      display.UpdateFrequency();                   // Change main display data

      while (true) {
        if (digitalRead(MAXSWITCH) != HIGH) {           // At the end stop switch?
          stepper.ResetStepperToZero();                         // Reset back to zero
          return;
        }
        currentSWR = swr.ReadSWRValue();
        autotune.AutoTuneSWR();

        display.UpdateFrequency();
        if (minSWRAuto < TARGETMAXSWR) {                   //Ignore values greater than Target Max
          bandLimitPositionCounts[i][j] = SWRMinPosition;
          tft.setCursor(0, 90 + whichLine * TEXTLINESPACING);
          if (currentFrequency < 10000000) {
            tft.print(" ");
          }
          tft.print(currentFrequency);
          tft.setCursor(150, 90 + whichLine * TEXTLINESPACING);
          tft.print(minSWRAuto);
          tft.setCursor(230, 90 + whichLine * TEXTLINESPACING);
          tft.print(SWRMinPosition);
          whichLine++;                                                      // Ready for next line of output
          delay(200);

          break;                                // This sends control to next edge
        }
      }
      currPosition = SWRFinalPosition + 50;
    }       // end for (j
    currPosition = SWRFinalPosition + 50;
  }         // end for (i

  // WritePositionCounts();                      // Write values to EEPROM  TEMPORARILY COMMENTED
  busy_wait_us_32(100L);
  //updateMessage("Press Exit");  TEMPORARILY COMMENTED

  while (digitalRead(FREQUENCYENCODERSWITCH) != LOW) { //Wait until Freq Encoder switch is pressed

  }
  //ShowMainDisplay(0, SWR);       // Draws top menu line  TEMPORARILY COMMENTED
  //ShowSubmenuData(SWR);                                  TEMPORARILY COMMENTED
  loop();
  busy_wait_us_32(100L);
}


/*****
  Purpose: Do initial Calibrate.  Does not assume band edge counts to begin

  Argument list:
    void

  Return value:
    void
*****/
void Calibrate::DoFirstCalibrate()  //Al modified 9-14-19
{
  int bandBeingCalculated;
  int i, j, whichLine;
  long localPosition, minCount;
  float currentSWR;
  // updateMessage("Initial Calibrate");  TEMPORARILY COMMENTED
  //tft.print("1st Cal ");
  bandBeingCalculated = 0;
  stepper.ResetStepperToZero();
  stepper.setCurrentPosition(0);        //Set Stepper count to zero
  currPosition = 100L;
  stepper.MoveStepperToPositionCorrected(currPosition);
  tft.fillRect(0, 46, 340, 231, ILI9341_BLACK);
  tft.drawFastHLine(0, 20, 320, ILI9341_RED);
  tft.drawFastHLine(0, 45, 320, ILI9341_RED);
  tft.setFont(&FreeSerif9pt7b);
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  tft.setCursor(0, 65);
  tft.print("Frequency               SWR       Count");    // Table header
  tft.setCursor(0, 90);                       // Read to show mins...

  whichLine = 0;                              // X coord for mins
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK); // Table data

  for (i = 0; i < MAXBANDS; i++) {            // For the 3 bands...
    for (j = 0; j < 2; j++) {
#ifdef DEBUG                          // So we can see the EEPROM values
      Serial.print("i=  "); Serial.println(i);
      Serial.print("j=  "); Serial.println(j);                   // So we can see the EEPROM values
#endif
      currentFrequency = bandEdges[i][j];     // Select a band edge to calibrate
      dds.SendFrequency(currentFrequency);        // Tell the DDS the edge frequency...
      busy_wait_us_32(100L);
      //UpdateFrequency();                   // Change main display data  TEMPORARILY COMMENTED
      //updateMessage("Moving to Freq");                                  TEMPORARILY COMMENTED
      while (true) {
        if (digitalRead(MAXSWITCH) != HIGH) {           // At the end stop switch?
          stepper.ResetStepperToZero();                         // Yep, so leave.
          return;
        }
        while (swr.ReadSWRValue() > 5) {    //Move stepper in CW direction in larger steps for SWR>5
          currPosition += 20;
          //UpdateFrequency();                     TEMPORARILY COMMENTED
          //ShowSubmenuData(ReadSWRValue());       TEMPORARILY COMMENTED
          stepper.MoveStepperToPositionCorrected(currPosition);
        }
        currentSWR = swr.ReadSWRValue();
        autotune.AutoTuneSWR();

        // UpdateFrequency(); TEMPORARILY COMMENTED
        if (minSWRAuto < TARGETMAXSWR) {                   //Ignore values greater than Target Max
          bandLimitPositionCounts[i][j] = SWRMinPosition;
          tft.setCursor(0, 90 + whichLine * TEXTLINESPACING);
          if (currentFrequency < 10000000) {
            tft.print(" ");
          }
          tft.print(currentFrequency);
          tft.setCursor(150, 90 + whichLine * TEXTLINESPACING);
          tft.print(minSWRAuto);
          tft.setCursor(230, 90 + whichLine * TEXTLINESPACING);
          tft.print(SWRMinPosition);
          whichLine++;                                                      // Ready for next line of output
          delay(200);

          break;                                // This sends control to next edge
        }
      }
      currPosition = SWRFinalPosition - 50;
    }       // end for (j
    currPosition = SWRFinalPosition - 50;
  }         // end for (i

  //WritePositionCounts();                      // Write values to EEPROM TEMPORARILY COMMENTED
  busy_wait_us_32(100L);
  tft.fillRect(0, PIXELHEIGHT - 40, 311, 25, ILI9341_BLACK);
  tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
  tft.setTextSize(1);
  tft.setFont(&FreeSerif9pt7b);
  tft.setCursor(40, PIXELHEIGHT - 20);
  tft.print("Press Exit");
  while (digitalRead(FREQUENCYENCODERSWITCH) != LOW) {

  }
  //ShowMainDisplay(0, SWR);       // Draws top menu line  TEMPORARILY COMMENTED
  //ShowSubmenuData(SWR);                                  TEMPORARILY COMMENTED
  quickCalFlag = 0;
  loop();
  busy_wait_us_32(100L);

}


/*****
  Purpose: To aid Calibration of SWR readings
             Not normally used in regular operation
  Argument list:
  void

  Return value:
  void
*****/
void Calibrate::CalSWR() {
  currentFrequency = 7150000L;
  dds.SendFrequency(currentFrequency);
  busy_wait_us_32(100L);
  int i;

  float VSWR;
  while (digitalRead(MENUENCODERSWITCH) != LOW)
  {
    /*  float sum[2] = {0.0, 0.0};

  float FWD = 0.0;
  float REV = 0.0;
    for (i = 0; i < MAXPOINTSPERSAMPLE; i++) {             // Take multiple samples at each frequency
      //MyDelay(5L);
      sum[0] +=  analogRead(ANALOGFORWARD);
      //MyDelay(5L);
      sum[1] +=  analogRead(ANALOGREFLECTED);
    }
    FWD = sum[0] /  MAXPOINTSPERSAMPLE;
    REV = sum[1] /  MAXPOINTSPERSAMPLE;
   REV += SWRREVOFFSET;
    if (REV >= FWD) {
      VSWR = 999.0;                               // To avoid a divide by zero or negative VSWR then set to max 999
    } else {
      VSWR = ((FWD + REV) / (FWD - REV));         // Calculate VSWR
    }
   
    //#ifdef DEBUG
    Serial.print("FWD=  "); Serial.println(FWD);
    Serial.print("REV=  "); Serial.println(REV);
    Serial.print("VSWR=  "); Serial.println(VSWR);
     Serial.println(" ");*/
    //#endif
     //SWR = swr.ReadSWRValue();
     // ShowSubmenuData(swr.ReadSWRValue());    TEMPORARILY COMMENTED
busy_wait_us_32(500);
  }
}

/*****
  Purpose: To set the band end point counts for a single band for intermediate calibration


  Argument list:
    void

  Return value:
    void
*****/
void Calibrate::DoSingleBandCalibrate(int whichBandOption) { //Al Added 4-18-20
  int bandBeingCalculated;
  int i, j, whichLine;
  long localPosition, minCount;
  float currentSWR;
  bandBeingCalculated = 0;
  stepper.ResetStepperToZero();
  stepper.setCurrentPosition(0);        //Set Stepper count to zero

  tft.fillRect(0, 46, 340, 231, ILI9341_BLACK);
  tft.drawFastHLine(0, 45, 320, ILI9341_RED);
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  tft.setTextSize(1);
  tft.setFont(&FreeSerif9pt7b);
  tft.setCursor(0, 65);
  tft.print("Frequency               SWR       Count");      // Table header
  tft.setCursor(0, 90);                       // Read to show mins...

  whichLine = 0;                              // X coord for mins
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK); // Table data

  for (j = 0; j < 2; j++) {                 // For each band edge...
    currentFrequency = bandEdges[whichBandOption][j];     // Select a band edge
    currPosition = bandLimitPositionCounts[whichBandOption][j] - 50;
    stepper.setMaxSpeed(FASTMOVESPEED);
    stepper.MoveStepperToPositionCorrected(currPosition); //Al 4-20-20
    dds.SendFrequency(currentFrequency);        // Tell the DDS the edge frequency...
    busy_wait_us_32(100L);
    //UpdateFrequency();                   // Change main display data  TEMPORARILY COMMENTED

    while (true) {
      if (digitalRead(MAXSWITCH) != HIGH) {           // At the end stop switch?
        stepper.ResetStepperToZero();                         // Yep, so leave.
        return;
      }
      currentSWR = swr.ReadSWRValue();

      autotune.AutoTuneSWRQuick();
      // UpdateFrequency();     TEMPORARILY COMMENTED
      if (minSWRAuto < TARGETMAXSWR) {                   //Ignore values greater than Target Max
        bandLimitPositionCounts[whichBandOption][j] = SWRMinPosition;

        tft.setCursor(0, 90 + whichLine * TEXTLINESPACING);
        if (currentFrequency < 10000000) {
          tft.print(" ");
        }
        tft.print(currentFrequency);
        tft.setCursor(150, 90 + whichLine * TEXTLINESPACING);
        tft.print(minSWRAuto);
        tft.setCursor(230, 90 + whichLine * TEXTLINESPACING);
        tft.print(SWRMinPosition);
        whichLine++;                                                      // Ready for next line of output
        delay(200);

        break;                                // This sends control to next edge
      }
    }
    currPosition = bandLimitPositionCounts[whichBandOption][1] - 50;

  }       // end for (j
  currPosition = SWRFinalPosition + 50;
  //WritePositionCounts();                      // Write values to EEPROM  TEMPORARILY COMMENTED
  busy_wait_us_32(100L);
  //updateMessage("Press Exit");     TEMPORARILY COMMENTED
  while (digitalRead(FREQUENCYENCODERSWITCH) != LOW) {

  }
  //ShowMainDisplay(0, SWR);       // Draws top menu line   TEMPORARILY COMMENTED
  //ShowSubmenuData(SWR);                                   TEMPORARILY COMMENTED
  quickCalFlag = 0;
  loop();
  busy_wait_us_32(100L);

}
