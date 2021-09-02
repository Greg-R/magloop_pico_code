
#include "DisplayManagement.h"



DisplayManagement::DisplayManagement(Adafruit_ILI9341 & tft, Calibrate & calibrate, DDS & dds, SWR & swr): tft(tft), calibrate(calibrate), dds(dds), swr(swr) {

}

/*****
  Purpose: To execute the FREQ menu option
  Argument list:
    void
  Return value:
    void
*****/
void DisplayManagement::frequencyMenuOption() {
  int SWRFlag1;
  int backCount = 0;
  long aveMinPosition;

  //updateMessage("Select Frequency"); TEMPORARILY COMMENTED

  whichBandOption = SelectBand();
  if (quickCalFlag == 1) {                  //Check if Single Band Button has been pushed.
    calibrate.DoSingleBandCalibrate(whichBandOption);
    quickCalFlag = 0;
  }
  currentFrequency = presetFrequencies[whichBandOption][3];    //Set initial frequency for each band from Preset list
  //currentFrequency = 7150000L;
  //ChangeFrequency(whichBandOption);                          //Alter the frequency TEMPORARILY COMMENTED
  dds.SendFrequency(currentFrequency);
  busy_wait_us_32(100L);                                 // Let DDS catch up
  SWRValue = swr.ReadSWRValue();
  readSWRValue        = SWRValue;
  //ShowSubmenuData(SWRValue);        TEMPORARILY COMMENTED
  tft.fillRect(0, 100, 311, 150, ILI9341_BLACK);
                          //Backup 20 counts to approach from CW direction
  currPosition = -50 +  bandLimitPositionCounts[whichBandOption][0]  + float((currentFrequency - bandEdges[whichBandOption][0])) / float(hertzPerStepperUnitVVC[whichBandOption]);
  stepper.setMaxSpeed(10000);
  stepper.setAcceleration(1100);
  //MoveStepperToPositionCorrected(currPosition); //Al 4-20-20
  //AutoTuneSWRQuick();        
  autotune.AutoTuneSWR();  //Auto tune here
 /* switch (whichBandOption) {   //Set amount to backup from final AutoTune position so nothing is missed
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
  currPosition = SWRMinPosition - backCount ;
  MoveStepperToPositionCorrected(currPosition); //Al 4-20-20
  //delay(100);
  AutoTuneSWR();*/
  dds.UpdateFrequency();
  //ShowSubmenuData(readSWRValueAuto); TEMPORARILY COMMENTED
  //GraphAxis(whichBandOption);        TEMPORARILY COMMENTED
  //PlotSWRValueNew(whichBandOption);  TEMPORARILY COMMENTED
  //updateMessage("Freq Encoder to Adjust"); TEMPORARILY COMMENTED
  //====================
  while (digitalRead(FREQUENCYENCODERSWITCH) != LOW) {
    if (quickCalFlag == 1) {
      DoSingleBandCalibrate(whichBandOption);
      quickCalFlag = 0;
    }
    if (menuEncoderMovement != 0) {          //Allow stepper to be moved maually
      ManualStepperControl();
    }
    if (frequencyEncoderMovement != 0) {     //Allow frequency to be changed maually

      ManualFrequencyControl(whichBandOption);  // In SWR.cpp
      frequencyEncoderMovement = 0;
    }
    if (digitalRead(AUTOTUNE) == LOW && digitalRead(MAXSWITCH) != LOW) {   //Redo the Autotune at new frequency/position
      currPosition = -80 +  bandLimitPositionCounts[whichBandOption][0]  + float((currentFrequency - bandEdges[whichBandOption][0])) / float(hertzPerStepperUnitVVC[whichBandOption]);
      stepper.setMaxSpeed(1000);
      stepper.setAcceleration(1100);
      MoveStepperToPositionCorrected(currPosition); //Al 4-20-20
      AutoTuneSWR();   //Auto tune here
      GraphAxis(whichBandOption);
      PlotSWRValueNew(whichBandOption);
      updateMessage("Freq: Adjust - ATune: Refine");
    }
  }
  // graphFlag == 0;
  MyDelay(100L);
  EraseBelowMenu();
  ShowMainDisplay(0, SWR);       // Draws top menu line
  ShowSubmenuData(SWR);
  loop();

}


/*****
  Purpose: Set new frequency

  Argument list:
    int bandIndex       // which of the three bands was selected?

  Return value:
    void
*****/
void DisplayManagement::ChangeFrequency(int bandIndex)  //Al Mod 9-8-19
{
  //EraseBelowMenu();
  int i, changeDigit, digitSpacing, halfScreen, incrementPad, insetMargin, insetPad;
  long defaultIncrement;
  insetPad         = 57;                         // Used to align digit indicator
  incrementPad     = 05;
  digitSpacing     = 10;
  insetMargin      = 20;
  defaultIncrement = 1000L;
  halfScreen   = PIXELHEIGHT / 2 - 25;
  updateMessage("Enter Frequency");
  tft.drawFastHLine(0, 20, 320, ILI9341_RED);
  if (bandIndex == 0) {                 // 40M
    insetPad = 32;           // smaller number, so less spacing to a given digit
  }
  EraseBelowMenu();
#ifdef DEBUG
  Serial.print("In ChangeFrequency() bandIndex = ");
  Serial.print(bandIndex);
  Serial.print("   insetMargin = ");
  Serial.println(insetMargin);
#endif
  tft.setTextSize(1);
  tft.setFont(&FreeSerif9pt7b);
  tft.setTextColor(ILI9341_WHITE);                      // Messages
  tft.setCursor(insetMargin, halfScreen + 60);
  tft.print("Increment:");
  tft.setCursor(insetMargin + 90, halfScreen + 60);
  tft.print("Menu");
  tft.setCursor(insetMargin, halfScreen + 80);
  tft.print("Digit:");
  tft.setCursor(insetMargin + 90, halfScreen + 80);
  tft.print("Freq");
  tft.setCursor(insetMargin, halfScreen + 100);
  tft.print("Tune:");
  tft.setCursor(insetMargin + 90, halfScreen + 100);
  tft.print("AutoTune Button");
  tft.setCursor(insetMargin, halfScreen + 120);
  tft.print("Exit:");
  tft.setCursor(insetMargin + 90, halfScreen + 120);
  tft.print("Freq Button");
  tft.setTextSize(1);
  tft.setFont(&FreeSerif24pt7b);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(insetMargin + (insetPad + incrementPad) + digitSpacing * 6 - 28, halfScreen + 5); // Assume 1KHz increment
  tft.print("_"); //underline selected character position
  tft.setTextColor(ILI9341_GREEN);
  tft.setCursor(insetMargin, halfScreen);
  tft.setTextSize(1);
  tft.setFont(&FreeSerif24pt7b);
  tft.print(currentFrequency);
  ShowSubmenuData(SWRValue);                 // Update screen SWR and freq
  tft.setFont(&FreeSerif24pt7b);

  while (digitalRead(AUTOTUNE) != LOW and digitalRead(MENUENCODERSWITCH) != LOW) {
    if (quickCalFlag == 1) {
      DoSingleBandCalibrate(whichBandOption);
      quickCalFlag = 0;
    }
    //--------------------------
    if (digitalRead(MENUBUTTON3) == LOW) {  //Menu Button3 Calibrate Menu option
      //DoNewCalibrate2();
      executeButton3();
    }
    //------------------------
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.setFont(&FreeSerif24pt7b);
    if (digitEncoderMovement == 1) {         // Change frequency digit increment
      tft.fillRect(0 , halfScreen + 6, PIXELWIDTH  * .90, 20, ILI9341_BLACK);
      defaultIncrement /= 10;
      if (defaultIncrement < 1) {                       // Don't go too far right
        defaultIncrement = 1L;
      }
      incrementPad += INCREMENTPAD;
      if (defaultIncrement > 1000000L) {
        defaultIncrement = 1000000L;
      }
      if (incrementPad > INCREMENTPAD * 4)
      { // Don't overshoot or...
        incrementPad -= INCREMENTPAD;
      }
      tft.setCursor(insetMargin + (insetPad + incrementPad) + digitSpacing * 6 - 28, halfScreen + 5); // Assume 1KHz increment
      tft.print("_");
      digitEncoderMovement = 0;
    }
    else {
      if (digitEncoderMovement == -1) {
        tft.fillRect(0 , halfScreen + 6, PIXELWIDTH  * .90, 20, ILI9341_BLACK);
        defaultIncrement *= 10;
        if (defaultIncrement > 1000000) {                       // Don't go too far right
          defaultIncrement = 1000000L;
        }
        incrementPad -= INCREMENTPAD;
        if (incrementPad < -INCREMENTPAD * 3)               // Don't undershoot either
          incrementPad += INCREMENTPAD;

        tft.setCursor(insetMargin + (insetPad + incrementPad) + digitSpacing * 6 - 28, halfScreen + 5); // Assume 1KHz increment
        tft.print("_");
        digitEncoderMovement = 0;
      }
    }
    tft.setTextColor(ILI9341_GREEN);
    digitEncoderMovement = 0;
#ifdef DEBUG
    Serial.print("In ChangeFreq: defaultIncrement = ");
    Serial.print(defaultIncrement);
    Serial.print("   incrementPad = ");
    Serial.println(incrementPad);
#endif
    menuEncoderMovement = 0;
    if (frequencyEncoderMovement) {     //Change digit value
      currentFrequency += (long) (frequencyEncoderMovement * defaultIncrement);
      SendFrequency(currentFrequency);    // Send the frequency
      SWR = ReadSWRValue();
      ShowSubmenuData(SWR);   
      currPosition = ConvertFrequencyToStepperCount(currentFrequency);
      tft.fillRect(insetMargin, halfScreen - 35, PIXELWIDTH * .80, 40, ILI9341_BLACK);
      tft.setCursor(insetMargin, halfScreen);
      tft.setTextSize(1);
      tft.setFont(&FreeSerif24pt7b);
      tft.print(currentFrequency);
      frequencyEncoderMovement = 0L;                   // Reset encoder flag
    }
    if (digitalRead(FREQUENCYENCODERSWITCH) == LOW) {   //Exit from routine
      menuIndex = FREQMENU;
      dds.SendFrequency(currentFrequency);    // Send the frequency
      //ShowMainDisplay(0, SWR);       // Draws top menu line        TEMPORARILY COMMENTED
      //ShowSubmenuData(SWR);          // Draws SWR and Freq info    TEMPORARILY COMMENTED
      menuIndex = MakeMenuSelection();
      loop();
      //break;
    }
  }
  //tft.setTextSize(2);                         // Back to normal
  tft.setTextColor(ILI9341_WHITE);
}

/*****
  Purpose: To get a main menu choice

  Argument list:
    Adafruit_ILI9341 tft      the display object
    float SWR                 the current SWR value

  Return value:
    int                       the menu selected
*****/
int DisplayManagement::MakeMenuSelection() //Al Mod 9-8-19
{
  tft.setFont();
  tft.setTextSize(2);
  int i, index = 0;
  while (digitalRead(MENUENCODERSWITCH) != LOW) {
    if (quickCalFlag == 1) {
      DoSingleBandCalibrate(whichBandOption);
      quickCalFlag = 0;
    }
    //---------------------------
    if (digitalRead(MENUBUTTON1) == LOW) {        //Menu Button1 Band select Option
      executeButton1();
    }
    //------------------------
    if (digitalRead(MENUBUTTON3) == LOW) {  //Menu Button3 Calibrate Menu option
      //DoNewCalibrate2();
      executeButton3();
    }
    //------------------------
    if (menuEncoderMovement) {                             // Must be i (CW) or -1 (CCW)
      if (menuEncoderMovement == 1) {
        index++;
        if (index == MAXMENUES) {        // wrap to first index
          index = 0;
        }
      }
      if (menuEncoderMovement == -1) {
        index--;
        if (index < 0) {        // wrap to first index
          index = MAXMENUES - 1;
        }
      }
      menuEncoderMovement = 0;
      tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
      for (i = 0; i < MAXMENUES; i++) {
        tft.setCursor(i * 100, 0);
        tft.print(menuOptions[i]);
      }
      tft.setTextColor(ILI9341_BLUE, ILI9341_WHITE);
      tft.setCursor(index * 100, 0);
      tft.print(menuOptions[index]);
    }
  }
  tft.setTextColor(ILI9341_BLUE, ILI9341_WHITE);
  tft.setCursor(index * 100, 0);
  tft.print(menuOptions[index]);

  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
#ifdef DEBUG
  Serial.print("Leaving MakeMenuSelection(), menuIndex = ");
  Serial.println(index);
#endif
  return index;
}

/*****
  Purpose: To get a band menu choice
  Argument list:
    Adafruit_ILI9341 tft      the display object
  Return value:
    int                       the menu selected
*****/
int SelectBand()
{
  updateMessage("Select Band");
  // tft.print("Select Band");
  tft.setTextSize(1);
  tft.setFont(&FreeSerif12pt7b);
  char *bands[] = {"40M", "30M", "20M"};
  int currBand[] = {40, 30, 20};
  int i, index, where = 0;
  tft.fillRect(0, 52, PIXELWIDTH, PIXELHEIGHT, ILI9341_BLACK);
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  for (int i = 0; i < 3; i++) {
    tft.setCursor(100, 110 + i * 30);
    tft.print(bands[i]);
  }
  tft.setCursor(100, 110);
  tft.setTextColor(ILI9341_BLUE, ILI9341_WHITE);
  tft.print(bands[0]);
  digitalWrite(MENUENCODERSWITCH, HIGH);
  MyDelay(100L);
  index = 0;
  while (true) {
    if (quickCalFlag == 1) {
      DoSingleBandCalibrate(whichBandOption);
      quickCalFlag = 0;
    }

    //---------------------------
    if (digitalRead(MENUBUTTON1) == LOW) {        //Menu Button1 Band select Option
      executeButton1();
    }
    //------------------------
    if (digitalRead(MENUBUTTON3) == LOW) {  //Menu Button3 Calibrate Menu option
      //DoNewCalibrate2();
      executeButton3();
    }
    //------------------------
    if (menuEncoderMovement) {
      if (menuEncoderMovement == 1) {
        index++;
        if (index == 3) {        // wrap to first index
          index = 0;
        }
      }
      if (menuEncoderMovement == -1) {
        index--;
        if (index < 0) {        // wrap to last index
          index = 2;
        }
      }
      menuEncoderMovement = 0;
      tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
      for (int i = 0; i < 3; i++) {
        tft.setCursor(100, 110 + i * 30);
        tft.print(bands[i]);
      }

      tft.setTextColor(ILI9341_BLUE, ILI9341_WHITE);
      tft.setCursor(100, 110 + index * 30);
      tft.print(bands[index]);

    }
    if (digitalRead(MENUENCODERSWITCH) == LOW)
      break;
  }

  MyDelay(500L);
  currentBand = currBand[index];
  return index;
}

/*****
  Purpose: To erase the display below the top two menu lines
  Argument list:
    void
  Return value:
    void
*****/
void DisplayManagement::EraseBelowMenu() //al mod 9-8-19
{
  tft.fillRect(0, 46, 340, 231, ILI9341_BLACK);
  tft.drawFastHLine(0, 45, 320, ILI9341_RED);
}

/*****
  Purpose: To erase the display page
  Argument list:
    void
  Return value:
    void
*****/
void ErasePage()
{
  tft.fillScreen(ILI9341_BLACK);
}


/*****
  Purpose: To display the main menu page

  Argument list:
    int whichMenuPage         the currently displayed menu page
    float SWR                 the current SWR value
  Return value:
    int                       the menu selected
*****/
int DisplayManagement::ShowMainDisplay(int whichMenuPage, float SWR)
{
  int lastMenuPage = 0;
  int i;
  tft.setFont();
  tft.setTextSize(2);
  tft.fillScreen(ILI9341_BLACK);
  for (i = 0; i < 3; i++) {
    if (i == 0) {
      tft.setTextColor(ILI9341_BLUE, ILI9341_WHITE);
    } else {
      tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
    }
    tft.setCursor(i * 100, 0);
    tft.print(menuOptions[i]);
  }
  return lastMenuPage;
}


/*****
  Purpose: To display the SWR amd frequency data
  Argument list:
    float SWR                 the current SWR value
  Return value:
    void
*****/
void DisplayManagement::ShowSubmenuData(float SWR) //al mod 9-8-19
{
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  tft.setFont(&FreeSerif9pt7b);
  tft.fillRect(0, 23, PIXELWIDTH, 20, ILI9341_BLACK);
  tft.drawFastHLine(0, 20, 320, ILI9341_RED);
  tft.setCursor(0, 40);
  tft.print("SWR ");
  tft.setFont(&FreeSerif12pt7b);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  if (SWR > 50.0 || SWR < .5)
  { // Real or bogus SWR?
    tft.print("??");                                //...bogus
  }
  else
  {
    if (SWR > 9.9999)
    {
      tft.print(SWR, 2);
    }
    else
    {
      tft.print(SWR, 3);                                //...real
    }
  }
  UpdateFrequency();
  tft.drawFastHLine(0, 45, 320, ILI9341_RED);

}


/*****
  Purpose: To rewrite the frequency display

  Argument list:

  Return value:
    int                       the menu selected
*****/
void DisplayManagement::UpdateFrequency()
{
  tft.setTextSize(1);
  tft.setFont(&FreeSerif9pt7b);
  tft.fillRect(140, 25, PIXELWIDTH, 20, ILI9341_BLACK);
  tft.setCursor(100, 40);
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  tft.print("FREQ ");
  tft.setFont(&FreeSerif12pt7b);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.print(currentFrequency);
  tft.setFont(&FreeSerif9pt7b);
  //tft.setCursor(250, 40);
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  tft.print("  p ");
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.print(currPosition);
  //Serial.print("currentFrequency = ");
  //Serial.println(currentFrequency);
}

/*****
  Purpose: Update the SWR value

  Argument list:
    float SWR                 the current SWR value

  Return value:
    void
*****/
void DisplayManagement::UpdateSWR(float SWR, char msg[])
{
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setCursor(60, 30);
  if (strlen(msg) > 0) {
    tft.print(msg);
  } else {
    if (SWR > .5 && SWR < 50.0) {
      tft.print(SWR);
    } else {
      tft.print("> 50");
    }
  }
}


/*****
  Purpose: Update Top Message Area

  Argument list:
    char message

  Return value:
    void
*****/
void DisplayManagement::updateMessage(char messageToPrint[]) {
  tft.fillRect(90, 0, 300, 20, ILI9341_BLACK);
  tft.drawFastHLine(0, 20, 320, ILI9341_RED);
  tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
  tft.setTextSize(1);
  tft.setFont(&FreeSerif9pt7b);
  tft.setCursor(91, 12);
  tft.print(messageToPrint);
}
