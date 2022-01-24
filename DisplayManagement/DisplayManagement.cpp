
#include "DisplayManagement.h"

DisplayManagement::DisplayManagement(Adafruit_ILI9341 & tft, DDS & dds, SWR & swr, 
                                     AutoTune & autotune, StepperManagement & stepper
                                     ): tft(tft), dds(dds), swr(swr),
                                     autotune(autotune), stepper(stepper) {}

void DisplayManagement::Splash(std::string version, std::string releaseDate)
{
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_MAGENTA, ILI9341_BLACK);
  tft.setCursor(10, 20);
  tft.print("Microcontroller Projects");
  tft.setCursor(40, 45);
  tft.print("for Amateur Radio");
  tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
  tft.setCursor(PIXELWIDTH / 2 - 30, PIXELHEIGHT / 4 + 20);
  tft.print("by");
  tft.setCursor(65, PIXELHEIGHT - 100);
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  tft.println("Al Peter  AC8GY");
  tft.setCursor(45, PIXELHEIGHT - 70);
  tft.print("Jack Purdum  W8TEE");
  tft.setCursor(65, PIXELHEIGHT - 40);
  tft.print("Version ");
  tft.print(version.c_str());
  tft.setCursor(65, PIXELHEIGHT - 20);
  tft.print("Release Date ");
  tft.print(releaseDate.c_str());
  tft.setTextSize(2);
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

  updateMessage("Select Frequency"); //TEMPORARILY COMMENTED

  whichBandOption = SelectBand();
  if (quickCalFlag == 1) {                  //Check if Single Band Button has been pushed.
    DoSingleBandCalibrate(whichBandOption);
    quickCalFlag = 0;
  }
  currentFrequency = presetFrequencies[whichBandOption][3];    //Set initial frequency for each band from Preset list
  //currentFrequency = 7150000L;
  ChangeFrequency(whichBandOption);       //Alter the frequency TEMPORARILY COMMENTED
  dds.SendFrequency(currentFrequency);
  busy_wait_us_32(100L);                  // Let DDS catch up
  SWRValue = swr.ReadSWRValue();
  readSWRValue        = SWRValue;
  ShowSubmenuData(SWRValue, currentFrequency);      //  TEMPORARILY COMMENTED
  tft.fillRect(0, 100, 311, 150, ILI9341_BLACK);
                          //Backup 20 counts to approach from CW direction
  currPosition = -50 +  bandLimitPositionCounts[whichBandOption][0]  + float((currentFrequency - bandEdges[whichBandOption][0])) / float(hertzPerStepperUnitVVC[whichBandOption]);
  stepper.setMaxSpeed(10000);
  stepper.setAcceleration(1100);
  //MoveStepperToPositionCorrected(currPosition); //Al 4-20-20
  //AutoTuneSWRQuick();
  updateMessage("Auto Tuning");        
  minSWRAuto = autotune.AutoTuneSWR();  //Auto tune here
  ShowSubmenuData(minSWRAuto, currentFrequency);  //Update SWR value  TEMPORARILY COMMENTED
  tft.fillRect(0, PIXELHEIGHT - 47, 311, 29, ILI9341_BLACK);   //Clear lower screen
  tft.fillRect(100, 0, 300, 20, ILI9341_BLACK);
  tft.drawFastHLine(0, 20, 320, ILI9341_RED);
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
  busy_wait_ms(100);
  AutoTuneSWR();*/
  DisplayManagement::UpdateFrequency(currentFrequency);
  //ShowSubmenuData(readSWRValueAuto); TEMPORARILY COMMENTED
  //GraphAxis(whichBandOption);        TEMPORARILY COMMENTED
  //PlotSWRValueNew(whichBandOption);  TEMPORARILY COMMENTED
  //updateMessage("Freq Encoder to Adjust"); TEMPORARILY COMMENTED
  //====================
  while (gpio_get(FREQUENCYENCODERSWITCH) != LOW) {
    if (quickCalFlag == 1) {
      DoSingleBandCalibrate(whichBandOption);
      quickCalFlag = 0;
    }
    if (menuEncoderMovement != 0) {          //Allow stepper to be moved maually
      swr.ManualStepperControl();
    }
    if (frequencyEncoderMovement != 0) {     //Allow frequency to be changed maually

      swr.ManualFrequencyControl(whichBandOption);  // In SWR.cpp
      frequencyEncoderMovement = 0;
    }
    if (gpio_get(AUTOTUNE) == LOW && gpio_get(MAXSWITCH) != LOW) {   //Redo the Autotune at new frequency/position
      currPosition = -80 +  bandLimitPositionCounts[whichBandOption][0]  + float((currentFrequency - bandEdges[whichBandOption][0])) / float(hertzPerStepperUnitVVC[whichBandOption]);
      stepper.setMaxSpeed(1000);
      stepper.setAcceleration(1100);
      stepper.MoveStepperToPositionCorrected(currPosition); //Al 4-20-20
      updateMessage("Auto Tuning");
      minSWRAuto = autotune.AutoTuneSWR();   //Auto tune here
      ShowSubmenuData(minSWRAuto, currentFrequency);  //Update SWR value  TEMPORARILY COMMENTED
  tft.fillRect(0, PIXELHEIGHT - 47, 311, 29, ILI9341_BLACK);   //Clear lower screen
  tft.fillRect(100, 0, 300, 20, ILI9341_BLACK);
  tft.drawFastHLine(0, 20, 320, ILI9341_RED);
      //GraphAxis(whichBandOption);          TEMPORARILY COMMENTED
      //PlotSWRValueNew(whichBandOption);    TEMPORARILY COMMENTED
      //updateMessage("Freq: Adjust - ATune: Refine");  TEMPORARILY COMMENTED
    }
  }
  // graphFlag == 0;
  busy_wait_us_32(100L);
  EraseBelowMenu();               //                        TEMPORARILY COMMENTED
  ShowMainDisplay(0, SWRcurrent);       // Draws top menu line   TEMPORARILY COMMENTED
  ShowSubmenuData(SWRcurrent, currentFrequency);           //                        TEMPORARILY COMMENTED
  //loop();

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
  // updateMessage("Enter Frequency");  TEMPORARILY COMMENTED
  tft.drawFastHLine(0, 20, 320, ILI9341_RED);
  if (bandIndex == 0) {                 // 40M
    insetPad = 32;           // smaller number, so less spacing to a given digit
  }
  EraseBelowMenu();
//#ifdef DEBUG
//  Serial.print("In ChangeFrequency() bandIndex = ");
//  Serial.print(bandIndex);
 // Serial.print("   insetMargin = ");
//  Serial.println(insetMargin);
//#endif
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
  ShowSubmenuData(SWRValue, currentFrequency);   // Update screen SWR and freq
  tft.setFont(&FreeSerif24pt7b);

  while (gpio_get(AUTOTUNE) != LOW and gpio_get(MENUENCODERSWITCH) != LOW) {
    if (quickCalFlag == 1) {
      DoSingleBandCalibrate(whichBandOption);
      quickCalFlag = 0;
    }
    //--------------------------
    if (gpio_get(MENUBUTTON3) == LOW) {  //Menu Button3 Calibrate Menu option
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
//#ifdef DEBUG
//    Serial.print("In ChangeFreq: defaultIncrement = ");
//    Serial.print(defaultIncrement);
//    Serial.print("   incrementPad = ");
//    Serial.println(incrementPad);
//#endif
    menuEncoderMovement = 0;
    if (frequencyEncoderMovement) {     //Change digit value
      currentFrequency += (long) (frequencyEncoderMovement * defaultIncrement);
      dds.SendFrequency(currentFrequency);    // Send the frequency
      SWRcurrent = swr.ReadSWRValue();  // Used???
      ShowSubmenuData(SWRcurrent, currentFrequency);
      currPosition = stepper.ConvertFrequencyToStepperCount(currentFrequency);
      tft.fillRect(insetMargin, halfScreen - 35, PIXELWIDTH * .80, 40, ILI9341_BLACK);
      tft.setCursor(insetMargin, halfScreen);
      tft.setTextSize(1);
      tft.setFont(&FreeSerif24pt7b);
      tft.print(currentFrequency);
      frequencyEncoderMovement = 0L;                   // Reset encoder flag
    }
    if (gpio_get(FREQUENCYENCODERSWITCH) == LOW) {   //Exit from routine
      menuIndex = FREQMENU;
      dds.SendFrequency(currentFrequency);    // Send the frequency
      ShowMainDisplay(0, SWRcurrent);       // Draws top menu line        TEMPORARILY COMMENTED
      ShowSubmenuData(SWRcurrent, currentFrequency);          // Draws SWR and Freq info    TEMPORARILY COMMENTED
      menuIndex = MakeMenuSelection();
     // loop();
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
  while (gpio_get(MENUENCODERSWITCH) != LOW) {
    if (quickCalFlag == 1) {
      DoSingleBandCalibrate(whichBandOption);
      quickCalFlag = 0;
    }
    //---------------------------
    if (gpio_get(MENUBUTTON1) == LOW) {        //Menu Button1 Band select Option
      executeButton1();
    }
    //------------------------
    if (gpio_get(MENUBUTTON3) == LOW) {  //Menu Button3 Calibrate Menu option, also called BANDCAL
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
        tft.print(menuOptions[i].c_str());
      }
      tft.setTextColor(ILI9341_BLUE, ILI9341_WHITE);
      tft.setCursor(index * 100, 0);
      tft.print(menuOptions[index].c_str());
    }
  }
  tft.setTextColor(ILI9341_BLUE, ILI9341_WHITE);
  tft.setCursor(index * 100, 0);
  tft.print(menuOptions[index].c_str());

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
int DisplayManagement::SelectBand()
{
  updateMessage("Select Band");  // TEMPORARILY COMMENTED
  tft.print("Select Band");
  tft.setTextSize(1);
  tft.setFont(&FreeSerif12pt7b);
  const std::string bands[3] = {"40M", "30M", "20M"};
  int currBand[] = {40, 30, 20};
  int i, index, where = 0;
  tft.fillRect(0, 52, PIXELWIDTH, PIXELHEIGHT, ILI9341_BLACK);
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  for (int i = 0; i < 3; i++) {
    tft.setCursor(100, 110 + i * 30);
  //  char * band = &(bands[i][0]);
    tft.print(bands[i].c_str());
   // tft.print(band);
  }
  tft.setCursor(100, 110);
  tft.setTextColor(ILI9341_BLUE, ILI9341_WHITE);
  //char * bands0 = &(bands[0][0]);
  tft.print(bands[0].c_str());
  //tft.print(bands0);
  //digitalWrite(MENUENCODERSWITCH, HIGH);
  busy_wait_us_32(100L);
  index = 0;
  while (true) {
    if (quickCalFlag == 1) {
      DoSingleBandCalibrate(whichBandOption);
      quickCalFlag = 0;
    }

    //---------------------------
    if (gpio_get(MENUBUTTON1) == LOW) {        //Menu Button1 Band select Option
      executeButton1();
    }
    //------------------------
    if (gpio_get(MENUBUTTON3) == LOW) {  //Menu Button3 Calibrate Menu option
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
      //  char * band = &(bands[i][0]);
        tft.print(bands[i].c_str());
      //tft.print(band);
      }

      tft.setTextColor(ILI9341_BLUE, ILI9341_WHITE);
      tft.setCursor(100, 110 + index * 30);
     // char * bands = &(bands[index]);
      tft.print(bands[index].c_str());
      //tft.print(bands);
    }
    if (gpio_get(MENUENCODERSWITCH) == LOW)
      break;
  }

  busy_wait_us_32(500L);
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
void DisplayManagement::ErasePage()
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
    tft.print(menuOptions[i].c_str());
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
void DisplayManagement::ShowSubmenuData(float SWR, int currentFrequency) //al mod 9-8-19
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
      tft.print(SWR, 2);                                //...real
    }
  }
  UpdateFrequency(currentFrequency);
  tft.drawFastHLine(0, 45, 320, ILI9341_RED);

}


/*****
  Purpose: To rewrite the frequency display

  Argument list:

  Return value:
    int                       the menu selected
*****/
void DisplayManagement::UpdateFrequency(int frequency)
{
  tft.setTextSize(1);
  tft.setFont(&FreeSerif9pt7b);
  tft.fillRect(140, 25, PIXELWIDTH, 20, ILI9341_BLACK);
  tft.setCursor(100, 40);
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  tft.print("FREQ ");
  tft.setFont(&FreeSerif12pt7b);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.print(frequency);
  tft.setFont(&FreeSerif9pt7b);
  //tft.setCursor(250, 40);
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  tft.print("  p ");
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.print(stepper.currentPosition());
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
//void DisplayManagement::UpdateSWR(float SWR, char msg[])
void DisplayManagement::UpdateSWR(float SWR, std::string msg)
{
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setCursor(60, 30);
  if (msg.size() > 0) {
    tft.print(msg.c_str());
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
void DisplayManagement::updateMessage(std::string messageToPrint) {
  tft.fillRect(90, 0, 300, 20, ILI9341_BLACK);
  tft.drawFastHLine(0, 20, 320, ILI9341_RED);
  tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
  tft.setTextSize(1);
  tft.setFont(&FreeSerif9pt7b);
  tft.setCursor(91, 12);
  tft.print(messageToPrint.c_str());
}

/*****
  Purpose: To set the band end point counts
           Uses initial Band Edge counts for rapid re-calibrate
  Argument list:
    void

  Return value:
    void
*****/
void DisplayManagement::DoNewCalibrate2()  //Al modified 9-14-19
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
      UpdateFrequency(currentFrequency);                   // Change main display data

      while (true) {
        if (gpio_get(MAXSWITCH) != HIGH) {           // At the end stop switch?
          stepper.ResetStepperToZero();                         // Reset back to zero
          return;
        }
        currentSWR = swr.ReadSWRValue();
        updateMessage("Auto Tuning");
        minSWRAuto = autotune.AutoTuneSWR();
  ShowSubmenuData(minSWRAuto, currentFrequency);  //Update SWR value  TEMPORARILY COMMENTED
  tft.fillRect(0, PIXELHEIGHT - 47, 311, 29, ILI9341_BLACK);   //Clear lower screen
  tft.fillRect(100, 0, 300, 20, ILI9341_BLACK);
  tft.drawFastHLine(0, 20, 320, ILI9341_RED);
        UpdateFrequency(currentFrequency);  // Is this redundant???
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
          busy_wait_ms(200);

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

  while (gpio_get(FREQUENCYENCODERSWITCH) != LOW) { //Wait until Freq Encoder switch is pressed

  }
  //ShowMainDisplay(0, SWR);       // Draws top menu line  TEMPORARILY COMMENTED
  //ShowSubmenuData(SWR);                                  TEMPORARILY COMMENTED
  //loop();
  busy_wait_us_32(100L);
}


/*****
  Purpose: Do initial Calibrate.  Does not assume band edge counts to begin

  Argument list:
    void

  Return value:
    void
*****/
void DisplayManagement::DoFirstCalibrate()  //Al modified 9-14-19
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
        if (gpio_get(MAXSWITCH) != HIGH) {           // At the end stop switch?
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
        updateMessage("Auto Tuning");
        minSWRAuto = autotune.AutoTuneSWR();
  ShowSubmenuData(minSWRAuto, currentFrequency);  //Update SWR value  TEMPORARILY COMMENTED
  tft.fillRect(0, PIXELHEIGHT - 47, 311, 29, ILI9341_BLACK);   //Clear lower screen
  tft.fillRect(100, 0, 300, 20, ILI9341_BLACK);
  tft.drawFastHLine(0, 20, 320, ILI9341_RED);
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
          busy_wait_ms(200);

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
  while (gpio_get(FREQUENCYENCODERSWITCH) != LOW) {

  }
  //ShowMainDisplay(0, SWR);       // Draws top menu line  TEMPORARILY COMMENTED
  //ShowSubmenuData(SWR);                                  TEMPORARILY COMMENTED
  quickCalFlag = 0;
//  loop();
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
void DisplayManagement::CalSWR() {
  currentFrequency = 7150000L;
  dds.SendFrequency(currentFrequency);
  busy_wait_us_32(100L);
  int i;

  float VSWR;
  while (gpio_get(MENUENCODERSWITCH) != LOW)
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
void DisplayManagement::DoSingleBandCalibrate(int whichBandOption) { //Al Added 4-18-20
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
      if (gpio_get(MAXSWITCH) != HIGH) {           // At the end stop switch?
        stepper.ResetStepperToZero();                         // Yep, so leave.
        return;
      }
      currentSWR = swr.ReadSWRValue();
updateMessage("Auto Tuning");
      minSWRAuto = autotune.AutoTuneSWRQuick();
      ShowSubmenuData(minSWRAuto, currentFrequency);  //Update SWR value 
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
        busy_wait_ms(200);

        break;                                // This sends control to next edge
      }
    }
    currPosition = bandLimitPositionCounts[whichBandOption][1] - 50;

  }       // end for (j
  currPosition = SWRFinalPosition + 50;
  //WritePositionCounts();                      // Write values to EEPROM  TEMPORARILY COMMENTED
  busy_wait_us_32(100L);
  //updateMessage("Press Exit");     TEMPORARILY COMMENTED
  while (gpio_get(FREQUENCYENCODERSWITCH) != LOW) {

  }
  //ShowMainDisplay(0, SWR);       // Draws top menu line   TEMPORARILY COMMENTED
  //ShowSubmenuData(SWR);                                   TEMPORARILY COMMENTED
  quickCalFlag = 0;
//  loop();
  busy_wait_us_32(100L);

}

/*****
  Purpose: To execute the Button1 Preset

  Argument list:
    void
  Return value:
    void
*****/
void DisplayManagement::executeButton1() {
  whichBandOption = SelectBand();            // Select the band to be used
  submenuIndex = 0;
  ProcessPresets(whichBandOption, submenuIndex);         // Select a preselected frequency
  menuIndex = FREQMENU;                         // When done, start over...
  //ShowMainDisplay(menuIndex, SWR);   TEMPORARILY COMMENTED
  //ShowSubmenuData(SWR);              TEMPORARILY COMMENTED
  dds.SendFrequency(currentFrequency);
  //SWR = ReadNewSWRValue();           TEMPORARILY COMMENTED
  //EraseBelowMenu();                  TEMPORARILY COMMENTED

}

/*****
  Purpose: To execute the Button3 Preset Select Menu Option

  Argument list:
    void
  Return value:
    void
*****/
void DisplayManagement::executeButton3() {
  DoNewCalibrate2();
}

/******
  Purpose: The ISR to detect the QuickCal Button push

  Parameter list:
    void

  Return value:
    void
*****/
void DisplayManagement::quickCalISR() {
  quickCalFlag = 1;
}

/*****
  Purpose: To move the capacitor to the approximate location via the stepper motor

  Parameter list:
    Adafruit_ILI9341 tft      the display object
    int whichBandOption
    int submenuIndex          which of presets

  Return value:
    void

  CAUTION:

*****/
void DisplayManagement::ProcessPresets(int whichBandOption, int submenuIndex)
{
  int i;
  int backCount = 0;
  tft.setFont();
  tft.setTextSize(2);
  //updateMessage("Select Preset Frequency"); // TEMPORARILY COMMENTED
  EraseBelowMenu();                         // TEMPORARILY COMMENTED
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
  //gpio_get(MENUENCODERSWITCH, HIGH);
  menuEncoderState = 0;
  busy_wait_us_32(100L);
  while (gpio_get(AUTOTUNE) != LOW and gpio_get(MENUENCODERSWITCH) != LOW) {
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
  stepper.MoveStepperToPositionCorrected(currPosition); //Al 4-20-20
 // AutoTuneSWRQuick();   //Auto tune here
 updateMessage("Auto Tuning");
minSWRAuto = autotune.AutoTuneSWR();
  ShowSubmenuData(minSWRAuto, currentFrequency);  //Update SWR value  TEMPORARILY COMMENTED
  tft.fillRect(0, PIXELHEIGHT - 47, 311, 29, ILI9341_BLACK);   //Clear lower screen
  tft.fillRect(100, 0, 300, 20, ILI9341_BLACK);
  tft.drawFastHLine(0, 20, 320, ILI9341_RED);
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
  //  ShowSubmenuData(minSWRAuto);  //Update SWR value  TEMPORARILY COMMENTED
  //tft.fillRect(0, PIXELHEIGHT - 47, 311, 29, ILI9341_BLACK);   //Clear lower screen
  //tft.fillRect(100, 0, 300, 20, ILI9341_BLACK);
  //tft.drawFastHLine(0, 20, 320, ILI9341_RED);
  UpdateFrequency(currentFrequency);
  //ShowSubmenuData(readSWRValueAuto);   TEMPORARILY COMMENTED
  //GraphAxis(whichBandOption);          TEMPORARILY COMMENTED
  //PlotSWRValueNew(whichBandOption);    TEMPORARILY COMMENTED

  while (gpio_get(FREQUENCYENCODERSWITCH) != LOW) {
    if (menuEncoderMovement != 0) {
      swr.ManualStepperControl();
    }
    if (frequencyEncoderMovement != 0) {
      swr.ManualFrequencyControl(whichBandOption);  // In SWR.cpp
      frequencyEncoderMovement = 0;
    }
    if (gpio_get(AUTOTUNE) == LOW && gpio_get(MAXSWITCH) != LOW) {   //Redo the Autotune at new frequency/position
      currPosition = -80 +  bandLimitPositionCounts[whichBandOption][0]  + float((currentFrequency - bandEdges[whichBandOption][0])) / float(hertzPerStepperUnitVVC[whichBandOption]);
      stepper.setMaxSpeed(1000);
      stepper.setAcceleration(1100);
      stepper.MoveStepperToPositionCorrected(currPosition); //Al 4-20-20
      updateMessage("Auto Tuning");
      minSWRAuto = autotune.AutoTuneSWR();   //Auto tune here
        ShowSubmenuData(minSWRAuto, currentFrequency);  //Update SWR value  TEMPORARILY COMMENTED
  tft.fillRect(0, PIXELHEIGHT - 47, 311, 29, ILI9341_BLACK);   //Clear lower screen
  tft.fillRect(100, 0, 300, 20, ILI9341_BLACK);
  tft.drawFastHLine(0, 20, 320, ILI9341_RED);
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
//  loop();
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
void DisplayManagement::RestorePreviousPresetChoice(int submenuIndex, int whichBandOption)
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
void DisplayManagement::HighlightNewPresetChoice(int submenuIndex, int whichBandOption)
{
  tft.setTextColor(ILI9341_MAGENTA, ILI9341_WHITE);         // HIghlight new preset choice
  tft.setCursor(65, 70 + submenuIndex * 30);
  tft.print(presetFrequencies[whichBandOption][submenuIndex]);
}
