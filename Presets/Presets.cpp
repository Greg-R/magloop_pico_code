
#include "Presets.h"

Presets::Presets(Adafruit_ILI9341 & tft, StepperManagement & steppermanage, AccelStepper & stepper, DDS & dds, AutoTune & autotune, SWR & swr, DisplayManagement & display): tft(tft), steppermanage(steppermanage), stepper(stepper), dds(dds), autotune(autotune), swr(swr), display(display) {}

/*****
  Purpose: To move the cap[acitor to the approximate location via the stepper motor

  Parameter list:
    Adafruit_ILI9341 tft      the display object
    int whichBandOption
    int submenuIndex          which of presets

  Return value:
    void

  CAUTION:

*****/
void Presets::ProcessPresets(int whichBandOption, int submenuIndex)
{
  int i;
  int backCount = 0;
  tft.setFont();
  tft.setTextSize(2);
  //updateMessage("Select Preset Frequency");  TEMPORARILY COMMENTED
  //EraseBelowMenu();                          TEMPORARILY COMMENTED
  tft.setTextSize(1);
  tft.setFont(&FreeSerif12pt7b);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);       // Show presets for selected band
  for (i = 0; i < PRESETSPERBAND; i++) {
    tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
    tft.setCursor(30, 70 + i * 30);
    tft.print(i + 1);
    tft.print(".");
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.setCursor(65, 70 + i * 30);
    tft.print(presetFrequencies[whichBandOption][i]);
  }
  tft.setTextColor(ILI9341_MAGENTA, ILI9341_WHITE);
  tft.setCursor(65, 70 + submenuIndex * 30);
  tft.print(presetFrequencies[whichBandOption][submenuIndex]);
  digitalWrite(MENUENCODERSWITCH, HIGH);
  menuEncoderState = 0;
  busy_wait_us_32(100L);
  while (digitalRead(AUTOTUNE) != LOW and digitalRead(MENUENCODERSWITCH) != LOW) {
    if (menuEncoderMovement == 1) {                                  // Turning clockwise
#ifdef DEBUG
      Serial.print("in enc = 1, submenuIndex = ");
      Serial.println(submenuIndex);
#endif
      RestorePreviousPresetChoice(submenuIndex, whichBandOption);
      busy_wait_us_32(200L);
      submenuIndex++;
      if (submenuIndex > PRESETSPERBAND - 1)
        submenuIndex = 0;
      HighlightNewPresetChoice(submenuIndex, whichBandOption);
      menuEncoderMovement = 0;
    }
    if (menuEncoderMovement == -1) {                                      // Tuning counter-clockwise
      RestorePreviousPresetChoice(submenuIndex, whichBandOption);
      submenuIndex--;
      if (submenuIndex < 0)
        submenuIndex = PRESETSPERBAND - 1;
      HighlightNewPresetChoice(submenuIndex, whichBandOption);
      menuEncoderMovement = 0;
    }
  }
  currentFrequency = presetFrequencies[whichBandOption][submenuIndex];
  // ShowSubmenuData(SWR);          // Draws SWR and Freq info  TEMPORARILY COMMENTED
  dds.SendFrequency(currentFrequency);
  // UpdateFrequency();   TEMPORARILY COMMENTED


  currPosition = -25 +  bandLimitPositionCounts[whichBandOption][0]  + float((currentFrequency - bandEdges[whichBandOption][0])) / float(hertzPerStepperUnitVVC[whichBandOption]);
  stepper.setMaxSpeed(10000);
  stepper.setAcceleration(1100);
  steppermanage.MoveStepperToPositionCorrected(currPosition); //Al 4-20-20
 // AutoTuneSWRQuick();   //Auto tune here
autotune.AutoTuneSWR();
/* switch (whichBandOption) {
    case 0:
      backCount = 60;
      break;
    case 1:
      backCount = 30;
      break;
    case 2:
      backCount = 35;
      break;
  }
  currPosition = SWRMinPosition - backCount ;*/
  //currPosition = SWRMinPosition - 50 ;
 // MoveStepperToPositionCorrected(currPosition); //Al 4-20-20
  //AutoTuneSWR();
  display.UpdateFrequency();
  //ShowSubmenuData(readSWRValueAuto);   TEMPORARILY COMMENTED
  //GraphAxis(whichBandOption);          TEMPORARILY COMMENTED
  //PlotSWRValueNew(whichBandOption);    TEMPORARILY COMMENTED

  while (digitalRead(FREQUENCYENCODERSWITCH) != LOW) {
    if (menuEncoderMovement != 0) {
      swr.ManualStepperControl();
    }
    if (frequencyEncoderMovement != 0) {
      swr.ManualFrequencyControl(whichBandOption);  // In SWR.cpp
      frequencyEncoderMovement = 0;
    }
    if (digitalRead(AUTOTUNE) == LOW && digitalRead(MAXSWITCH) != LOW) {   //Redo the Autotune at new frequency/position
      currPosition = -80 +  bandLimitPositionCounts[whichBandOption][0]  + float((currentFrequency - bandEdges[whichBandOption][0])) / float(hertzPerStepperUnitVVC[whichBandOption]);
      stepper.setMaxSpeed(1000);
      stepper.setAcceleration(1100);
      steppermanage.MoveStepperToPositionCorrected(currPosition); //Al 4-20-20
      autotune.AutoTuneSWR();   //Auto tune here
      //GraphAxis(whichBandOption);                   TEMPORARILY COMMENTED
      //PlotSWRValueNew(whichBandOption);             TEMPORARILY COMMENTED
      //updateMessage("Freq: Adjust - ATune: Refine");  TEMPORARILY COMMENTED
    }
   /* if (digitalRead(AUTOTUNE) == LOW && digitalRead(MAXSWITCH) != LOW) {
      currPosition = -80 +  bandLimitPositionCounts[whichBandOption][0]  + float((currentFrequency - bandEdges[whichBandOption][0])) / float(hertzPerStepperUnitVVC[whichBandOption]);
      stepper.setMaxSpeed(1000);
      stepper.setAcceleration(1100);
      MoveStepperToPositionCorrected(currPosition); //Al 4-20-20
      AutoTuneSWR();   //Auto tune here
      GraphAxis(whichBandOption);
      PlotSWRValueNew(whichBandOption);
      updateMessage("Freq: Adjust - ATune: Refine");
    }*/
  }
  //graphFlag == 0;
  busy_wait_us_32(100L);
  //EraseBelowMenu();        TEMPORARILY COMMENTED
  //ShowMainDisplay(0, SWR);       // Draws top menu line  TEMPORARILY COMMENTED
  //ShowSubmenuData(SWR);  TEMPORARILY COMMENTED
  loop();
}

/*****
  Purpose: To restore most recently highlighted preset choice

  Parameter list:
    Adafruit_ILI9341 tft      the display object
    int submenuIndex
    int whichBandOption

  Return value:
    void
*****/
void Presets::RestorePreviousPresetChoice(int submenuIndex, int whichBandOption)
{
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);               // restore old background
  tft.setCursor(65, 70 + submenuIndex * 30);
  tft.print(presetFrequencies[whichBandOption][submenuIndex]);
}


/*****
  Purpose: To highlight new preset choice

  Parameter list:
    Adafruit_ILI9341 tft      the display object
    int submenuIndex
    int whichBandOption

  Return value:
    void
*****/
void Presets::HighlightNewPresetChoice(int submenuIndex, int whichBandOption)
{
  tft.setTextColor(ILI9341_MAGENTA, ILI9341_WHITE);         // HIghlight new preset choice
  tft.setCursor(65, 70 + submenuIndex * 30);
  tft.print(presetFrequencies[whichBandOption][submenuIndex]);
}
