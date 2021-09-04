#include "AutoTune.h"


/*****
  Purpose: Main AutoTune routine
           Set desired frequency.  Vary Cap position to find lowest SWR
  Parameter list:
    void
  Return value:
    void

*****/


AutoTune::AutoTune(AccelStepper & stepper, SWR & swr, Adafruit_ILI9341 & tft, StepperManagement & steppermanage, DisplayManagement & display): stepper(stepper), swr(swr), tft(tft), steppermanage(steppermanage), display(display) {
}

void AutoTune::AutoTuneSWR() {    //Al Modified 9-14-19
  float oldMinSWR;
  oldMinSWR = 100;
  minSWRAuto = 100;
  int i;
  long currPositionTemp;
  SWRMinPosition = 20000;
  //updateMessage("Auto Tuning"); TEMPORARILY COMMENTED

  busy_wait_us_32(100);
  stepper.setMaxSpeed(FASTMOVESPEED);
  MoveStepperToPositionCorrected(currPosition);  //Move to initial position
  stepper.setMaxSpeed(NORMALMOVESPEED);
  for (int i = 0; i < MAXNUMREADINGS; i++) {   //reset temp arrays - used to plot SWR vs frequency
    tempSWR[i] = 0.0;
    tempCurrentPosition[i] = 0;
  }
  for (i = 0; i < MAXNUMREADINGS; i++) {       // loop to increment and find min SWR and save values to plot
    minSWR = swr.ReadSWRValue();                   //Save minimum SWR value
  //  ShowSubmenuData(minSWR); TEMPORARILY COMMENTED
    tempSWR[i] = minSWR;                       //Array of SWR values
    tempCurrentPosition[i] = currPosition;     //Array of Count position values
//#ifdef DEBUG                          // So we can see the EEPROM values
//    Serial.print("i = "); Serial.print(i); Serial.print("  currPosition = "); Serial.print(currPosition);
//    Serial.print("  stepper.currentPosition() = "); Serial.print(stepper.currentPosition());
//    Serial.print("   minSWR = "); Serial.println(minSWR);
//#endif
    if ( minSWR < minSWRAuto) {             // Test to find minimum SWR value
      minSWRAuto = minSWR;
      SWRMinPosition = currPosition;
      MoveStepperToPositionCorrected(SWRMinPosition);
    }
    if (minSWR > 3 and whichBandOption == 0) {   //Fast step for 40M band above SWR = 3
      stepper.setMaxSpeed(FASTMOVESPEED);
      currPosition = currPosition + 10;
      i = i + 10;
    }
    else
    {
      stepper.setMaxSpeed(NORMALMOVESPEED);
      currPosition++;
    }
    MoveStepperToPositionCorrected(currPosition);
    if (currPosition > SWRMinPosition + 10 and minSWR > minSWRAuto + 1.5 and minSWRAuto < 3.5) {   //Test to find if position is after minimum
      break;                                                                                      //if after minimum break out of for loop
    }
    SWRFinalPosition = currPosition;              // Save final value for calibrate to continue to find band end positions
    if (i > 498) {                                //Repeat loop if minimum is not found
      i = 1;
      currPosition = currPosition - 50;
    }
  } // for (true)
  minSWR = swr.ReadSWRValue();
  currPosition = SWRMinPosition;
  MoveStepperToPositionCorrected(SWRMinPosition - 50);   // back up position to take out backlash
  busy_wait_us_32(200);
  MoveStepperToPositionCorrected(SWRMinPosition);        //Move to final position in CW direction
  iMax = i; //max value in array for plot
 // ShowSubmenuData(minSWRAuto);  //Update SWR value  TEMPORARILY COMMENTED
  tft.fillRect(0, PIXELHEIGHT - 47, 311, 29, ILI9341_BLACK);   //Clear lower screen
  tft.fillRect(100, 0, 300, 20, ILI9341_BLACK);
  tft.drawFastHLine(0, 20, 320, ILI9341_RED);
}



/*****
  Purpose: Faster Autotune routine
           Does not save SWR values for plot
  Parameter list:
    void
  Return value:
    void

  CAUTION:
*****/
void AutoTune::AutoTuneSWRQuick() {    //Al Modified 9-14-19
  float oldMinSWR = minSWRAuto = 100;
  int i;
  long currPositionTemp;
  SWRMinPosition = 20000;
  if (gpio_get(ACCURACYBUTTON) == LOW) {
    currPositionTemp = currPosition;
    steppermanage.ResetStepperToZero();
    currPosition = currPositionTemp;
  }
  //updateMessage("Auto Quick Tuning");  TEMPORARILY COMMENTED
  busy_wait_us_32(100);

  stepper.setMaxSpeed(5000);
  stepper.setAcceleration(1100);
  //MoveStepperToPositionCorrected(currPosition);
  MoveStepperToPositionCorrected(2500);   //  Move stepper to mid-range.
  stepper.setMaxSpeed(500);

  for (i = 0; i < MAXNUMREADINGS; i++) {       // loop to increment and find min SWR and save values to plot
    minSWR = swr.ReadSWRValue();

  display.ErasePage();
  tft.setTextSize(2);
  tft.setCursor(10, 20);
  tft.print(minSWR);


  //  ShowSubmenuData(minSWR);  TEMPORARILY COMMENTED
//#ifdef DEBUG                          // So we can see the EEPROM values
//    Serial.print("i = "); Serial.print(i); Serial.print("  currPosition = "); Serial.print(currPosition);
//    Serial.print("  stepper.currentPosition() = "); Serial.print(stepper.currentPosition());
//    Serial.print("   minSWR = "); Serial.println(minSWR);
//#endif
    if ( minSWR < minSWRAuto) {             // Test to find minimum SWR value
      minSWRAuto = minSWR;
      SWRMinPosition = currPosition;
      //MoveStepperToPositionCorrected(SWRMinPosition);
    }
    if (minSWR > 3 ) {
      //if (minSWR > 3 and whichBandOption == 0) {
      stepper.setMaxSpeed(FASTMOVESPEED);
      currPosition = currPosition + 20;
      i = i + 20;
    }
    else
    {
      stepper.setMaxSpeed(NORMALMOVESPEED);
      currPosition++;
    }
    if (i > 498) {
      i = 1;
      currPosition = currPosition - 50;
    }
    MoveStepperToPositionCorrected(currPosition);
    if (currPosition > SWRMinPosition + 5 and minSWR > minSWRAuto + 1.0 and minSWRAuto < 3.5) { //Test to find if position is after minimum
      break;                                                             //if after minimum break out of for loop
    }

    SWRFinalPosition = currPosition;// Save final value to continue to find band end positions
  } // for (true)
  minSWR = swr.ReadSWRValue();
  currPosition = SWRMinPosition;
  MoveStepperToPositionCorrected(SWRMinPosition - 20);    // back up position to take out backlash
  busy_wait_us_32(100);
  MoveStepperToPositionCorrected(SWRMinPosition);         //Move to final position in CW direction 
  iMax = i; //max value in array for plot
//  ShowSubmenuData(minSWRAuto);  //Update SWR value  TEMPORARILY COMMENTED
}


void AutoTune::MoveStepperToPositionCorrected(long currentPosition) {
  int stepperDirection;
  long stepperDistance;
  while (1)
  {
    stepperDistanceOld = stepperDistance;
    stepper.moveTo(currentPosition);
    stepper.run();
    stepperDistance = stepper.distanceToGo();
    if (stepper.distanceToGo() == 0)
    {
      if (stepperDistanceOld >= 0)
      {
        stepperDirection = 1;
      }
      else
      {
        if (stepperDistanceOld < 0)
        {
          stepperDirection = -1;
        }
      }
      break;
    }
  }
  if (stepperDirection != stepperDirectionOld)
  {
    stepper.setCurrentPosition(currentPosition - 1);
  }
  stepperDirectionOld = stepperDirection;
}
