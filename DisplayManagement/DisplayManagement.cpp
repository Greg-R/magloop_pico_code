
#include "DisplayManagement.h"

int menuEncoderMovement;
int frequencyEncoderMovement;
int frequencyEncoderMovement2;
int digitEncoderMovement;
int quickCalFlag;

DisplayManagement::DisplayManagement(Adafruit_ILI9341 & tft, DDS & dds, SWR & swr, 
                                     StepperManagement & stepper, EEPROM & eeprom, Data & data
                                     ): tft(tft), dds(dds), swr(swr),
                                     stepper(stepper), eeprom(eeprom), GraphPlot(tft, dds, data), data(data) {}

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
  long frequency;
  State state;  // State machine next state.
  state = State::state1;  // Begin with Select Band state.
  // A state machine follows.  Run AutoTune when AutoTune button pushed.  Exit switch leaves while loop.
    while(true) {
       switch(state) {
          case State::state0:
            return;  //  Exit frequency selection and return to top menu Freq.
          case State::state1:
            whichBandOption = SelectBand(band);  // state1
            frequency = data.presetFrequencies[whichBandOption][3];  //Set initial frequency for each band from Preset list
            state = State::state2;  // Proceed to manual frequency adjustment state.
           break;
          case State::state2:           
            frequency = ChangeFrequency(whichBandOption, frequency);   //Alter the frequency using encoders.  Enter button returns frequency.
            // Exit frequency change if a zero is returned:
            if(frequency == 0) {
               state = State::state0;
               break;
            }
            dds.SendFrequency(frequency);  // Done in ChangeFrequency???
            busy_wait_ms(100);             // Let DDS catch up
            SWRValue = swr.ReadSWRValue();
            readSWRValue        = SWRValue;   // Redundant???
            ShowSubmenuData(SWRValue, frequency);
            tft.fillRect(0, 100, 311, 150, ILI9341_BLACK);  // ???
            //Backup 20 counts to approach from CW direction
            position = -50 +  data.bandLimitPositionCounts[whichBandOption][0]  + float((frequency - data.bandEdges[whichBandOption][0])) / float(data.hertzPerStepperUnitVVC[whichBandOption]);
            //  Move the stepper to the approximate location based on the current frequency:
            stepper.MoveStepperToPositionCorrected(position); //Al 4-20-20
            minSWRAuto = AutoTuneSWR();  //Auto tune here
            // After AutoTune, do full update of display with SWR vs. frequency plot:
            ShowSubmenuData(minSWRAuto, dds.currentFrequency);
            GraphAxis(whichBandOption);
            PlotSWRValueNew(whichBandOption, iMax, tempCurrentPosition, tempSWR, SWRMinPosition); 
            // Manual frequency and position tune:
            frequency = manualTune();
          break;  //  state is not changed; should go back to state2.
       }  // end switch
  }  // end of while loop and state machine
  return;
}

/*****
  Purpose: Manage the manual frequency and stepper functions.
  Argument list:
    void
  Return value:
    int frequency
*****/
int DisplayManagement::manualTune() {
while(true) {
  if(gpio_get(exitbutton) == false) return dds.currentFrequency;  // Exit manual tuning.
    if (menuEncoderMovement != 0) {          //Allow stepper to be moved maually
      ManualStepperControl();                //  Is this working???
    }
    if (frequencyEncoderMovement != 0) {     //Allow frequency to be changed maually.
      ManualFrequencyControl(whichBandOption);
      frequencyEncoderMovement = 0;
    }
                                             // Is this MAXSWITCH protection needed here???
    if (gpio_get(autotunebutton) == LOW && gpio_get(MAXSWITCH) != LOW) {   //Redo the Autotune at new frequency/position
      position = -80 +  data.bandLimitPositionCounts[whichBandOption][0]  + float((dds.currentFrequency - data.bandEdges[whichBandOption][0])) / float(data.hertzPerStepperUnitVVC[whichBandOption]);
      stepper.MoveStepperToPositionCorrected(position); //Al 4-20-20
      minSWRAuto = AutoTuneSWR();   //Auto tune here
      SWRMinPosition = stepper.currentPosition();  // Get the autotuned stepper position.
      GraphAxis(whichBandOption);
      PlotSWRValueNew(whichBandOption, iMax, tempCurrentPosition, tempSWR, SWRMinPosition);
      updateMessageTop("Freq: Adjust - AutoTune: Refine");
    }
}  // end while
}


/*****
  Purpose: Set new frequency
  This needs to be re-written as a state machine.???
  Argument list:
    int bandIndex    Which of the three bands was selected?
    long frequency   The current frequency.

  Return value:
    The new frequency is returned.
  
  Dependencies:  DDS, SWR, Adafruit_ILI9341
*****/
long DisplayManagement::ChangeFrequency(int bandIndex, long frequency)  //Al Mod 9-8-19
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
  updateMessageTop("Enter Frequency");
  tft.drawFastHLine(0, 20, 320, ILI9341_RED);
  if (bandIndex == 0) {                 // 40M
    insetPad = 32;           // smaller number, so less spacing to a given digit
  }
  //  The following configures the display for frequency selection mode.
  EraseBelowMenu();
  tft.setTextSize(1);
  tft.setFont(&FreeSerif9pt7b);
  tft.setTextColor(ILI9341_WHITE);                      // Messages
  tft.setCursor(insetMargin, halfScreen + 60);
  tft.print("Increment:");
  tft.setCursor(insetMargin + 90, halfScreen + 60);
  tft.print("Menu Encoder");
  tft.setCursor(insetMargin, halfScreen + 80);
  tft.print("Digit:");
  tft.setCursor(insetMargin + 90, halfScreen + 80);
  tft.print("Frequency Encoder");
  tft.setCursor(insetMargin, halfScreen + 100);
  tft.print("Tune:");
  tft.setCursor(insetMargin + 90, halfScreen + 100);
  tft.print("AutoTune Button");
  tft.setCursor(insetMargin, halfScreen + 120);
  tft.print("Exit:");
  tft.setCursor(insetMargin + 90, halfScreen + 120);
  tft.print("Exit Button");
  tft.setTextSize(1);
  tft.setFont(&FreeSerif24pt7b);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(insetMargin + (insetPad + incrementPad) + digitSpacing * 6 - 28, halfScreen + 5); // Assume 1KHz increment
  tft.print("_"); //underline selected character position
  tft.setTextColor(ILI9341_GREEN);
  tft.setCursor(insetMargin, halfScreen);
  tft.setTextSize(1);
  tft.setFont(&FreeSerif24pt7b);
  tft.print(frequency);
  //ShowSubmenuData(SWRValue, dds.currentFrequency);   // Update screen SWR and freq.  Where was SWRValue updated???
  tft.setFont(&FreeSerif24pt7b);

  // State Machine for frequency input with encoders.
  while (gpio_get(autotunebutton) != LOW and gpio_get(enterbutton) != LOW) {
    //  Exit this menu:
    if(gpio_get(exitbutton) == false) {
      frequency = 0;
      return frequency;
    }
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.setFont(&FreeSerif24pt7b);
    if (digitEncoderMovement == 1) {         // Change frequency digit increment
      tft.fillRect(0 , halfScreen + 6, PIXELWIDTH  * .90, 20, ILI9341_BLACK);
      defaultIncrement /= 10;
      if (defaultIncrement < 1) {            // Don't go too far right
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
    menuEncoderMovement = 0;
    if (frequencyEncoderMovement) {     //Change digit value
      frequency += (long) (frequencyEncoderMovement * defaultIncrement);
      dds.SendFrequency(frequency);    // Send the frequency
      SWRcurrent = swr.ReadSWRValue();
      ShowSubmenuData(SWRcurrent, dds.currentFrequency);
      position = stepper.ConvertFrequencyToStepperCount(frequency);
      tft.fillRect(insetMargin, halfScreen - 35, PIXELWIDTH * .80, 40, ILI9341_BLACK);
      tft.setCursor(insetMargin, halfScreen);
      tft.setTextSize(1);
      tft.setFont(&FreeSerif24pt7b);
      tft.print(dds.currentFrequency);
      frequencyEncoderMovement = 0L;   // Reset encoder flag
    }
  //  if (gpio_get(exitbutton) == false) {   //Exit from routine.  Redundant to while loop???
      //menuIndex = FREQMENU;
  //    dds.SendFrequency(frequency);  // Send the frequency
  //    ShowMainDisplay(menuIndex);       // Draws top menu line
  //    ShowSubmenuData(SWRcurrent, dds.currentFrequency);          // Draws SWR and Freq info
      // menuIndex = MakeMenuSelection(menuIndex);
     // loop();
      //break;
  //  }
  }  // end while loop
  tft.setTextSize(2);        // Back to normal
  tft.setTextColor(ILI9341_WHITE);
  return frequency;
}

/*****
  Purpose: To get a main menu choice:  Freq, Presets, or 1st Cal.

  Argument list:

  Return value:
    int          The menu selected

  Dependencies:  Adafruit_ILI9341
*****/
int DisplayManagement::MakeMenuSelection(int index) //Al Mod 9-8-19
{
  tft.setFont();
  tft.setTextSize(2);
  int i;

  // State Machine:
  while (gpio_get(enterbutton) != LOW) {
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
  }  // end State Machine

  tft.setTextColor(ILI9341_BLUE, ILI9341_WHITE);
  tft.setCursor(index * 100, 0);
  tft.print(menuOptions[index].c_str());
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);

  return index;
}

/*****
  Purpose: To get a band menu choice
  Argument list:
    const std::string bands[3].  Example: {"40M", "30M", "20M"}
  Return value:
    int                       the menu selected

  Dependencies:  Adafruit_ILI9341 tft
*****/
int DisplayManagement::SelectBand(const std::string bands[3])
{
  updateMessageTop("Select Band");
  tft.setTextSize(1);
  tft.setFont(&FreeSerif12pt7b);
  int currBand[] = {40, 30, 20};  // Used???
  int i, index, where = 0;
  tft.fillRect(0, 52, PIXELWIDTH, PIXELHEIGHT, ILI9341_BLACK);
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  for (int i = 0; i < 3; i++) {
    tft.setCursor(100, 110 + i * 30);
    tft.print(bands[i].c_str());
  }
  tft.setCursor(100, 110);
  tft.setTextColor(ILI9341_BLUE, ILI9341_WHITE);
  tft.print(bands[0].c_str());
  busy_wait_us_32(100L);
  index = 0;

  // State Machine.  Calling this function enters this loop and stays until Exit is pressed.
  while (true) {
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
        tft.print(bands[i].c_str());
      }
      tft.setTextColor(ILI9341_BLUE, ILI9341_WHITE);
      tft.setCursor(100, 110 + index * 30);
      tft.print(bands[index].c_str());
    }
    if (gpio_get(enterbutton) == LOW)  // Exit the state machine.
      break;
  }

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
    
  Return value:
    void                      
*****/
void DisplayManagement::ShowMainDisplay(int whichMenuPage)
{
  int lastMenuPage = 0;
  int i;
  tft.setFont();
  tft.setTextSize(2);
  tft.fillScreen(ILI9341_BLACK);
  for (i = 0; i < 3; i++) {
    if (i == whichMenuPage) {
      tft.setTextColor(ILI9341_BLUE, ILI9341_WHITE);
    } else {
      tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
    }
    tft.setCursor(i * 100, 0);
    tft.print(menuOptions[i].c_str());
  }
//  return lastMenuPage;  Doesn't need to return anything.???
}


/*****
  Purpose: To display the SWR and frequency data
  Argument list:
    float SWR, the current SWR value to be displayed
    int currentFrequency, the frequency to be displayed
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
    tft.print("??");         //...bogus
  }
  else
  {
    if (SWR > 9.9999)
    {
      tft.print(SWR, 2);
    }
    else
    {
      tft.print(SWR, 2);    // real
    }
  }
  UpdateFrequency(currentFrequency);
  tft.drawFastHLine(0, 45, 320, ILI9341_RED);
}


/*****
  Purpose: To rewrite the frequency display
  Does not reset DDS to new frequency!
  Argument list:

  Return value:
    void
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
}

/*****
  Purpose: Update the SWR value

  Argument list:
    float SWR                 the current SWR value

  Return value:
    void
*****/
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
void DisplayManagement::updateMessageTop(std::string messageToPrint) {
  tft.fillRect(90, 0, 300, 20, ILI9341_BLACK);
  tft.drawFastHLine(0, 20, 320, ILI9341_RED);
  tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
  tft.setTextSize(1);
  tft.setFont(&FreeSerif9pt7b);
  tft.setCursor(91, 12);
  tft.print(messageToPrint.c_str());
}


/*****
  Purpose: Update Bottom Message Area

  Argument list:
    char message

  Return value:
    void
*****/
void DisplayManagement::updateMessageBottom(std::string messageToPrint) {
  tft.fillRect(90, 200, 320, 240, ILI9341_BLACK);  // Erase previous message.
  //tft.drawFastHLine(0, 20, 320, ILI9341_RED);
  tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
  tft.setTextSize(1);
  tft.setFont(&FreeSerif9pt7b);
  tft.setCursor(91, 220);
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
  long localPosition, minCount, frequency;
  float currentSWR;
  updateMessageTop("Full Calibrate");
  bandBeingCalculated = 0;
  stepper.ResetStepperToZero();                  //Start off at zero
  //stepper.setCurrentPosition(0);        //Set Stepper count to zero
  //position = 100L;
  //stepper.runToNewPosition(position);
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
      frequency = data.bandEdges[i][j];     // Select a band edge
      position = data.bandLimitPositionCounts[i][j] - 200;       //Set Band limit count -200 counts to approach band limit from CW direction
      dds.SendFrequency(frequency);        // Tell the DDS the edge frequency...
      //busy_wait_us_32(100L);
      UpdateFrequency(frequency);                   // Change main display data

      while (true) {
        if (gpio_get(MAXSWITCH) != HIGH) {           // At the end stop switch?
          stepper.ResetStepperToZero();                         // Reset back to zero
          return;
        }
        //currentSWR = swr.ReadSWRValue();
        //updateMessageTop("Auto Tuning");
        minSWRAuto = AutoTuneSWR();
        ShowSubmenuData(swr.ReadSWRValue(), dds.currentFrequency);
        //UpdateFrequency(frequency);  // Is this redundant???
        if (minSWRAuto < TARGETMAXSWR) {                   //Ignore values greater than Target Max
          data.bandLimitPositionCounts[i][j] = SWRMinPosition;
          tft.setCursor(0, 90 + whichLine * TEXTLINESPACING);
          if (frequency < 10000000) {
            tft.print(" ");
          }
          tft.print(dds.currentFrequency);
          tft.setCursor(150, 90 + whichLine * TEXTLINESPACING);
          tft.print(minSWRAuto);
          tft.setCursor(230, 90 + whichLine * TEXTLINESPACING);
          tft.print(SWRMinPosition);
          whichLine++;                          // Ready for next line of output
          //busy_wait_ms(200);

          break;                                // This sends control to next edge
        }
      }
      position = SWRFinalPosition + 50;
    }       // end for (j
    position = SWRFinalPosition + 50;
  }         // end for (i

  eeprom.WritePositionCounts();                 // Write values to EEPROM
  //busy_wait_us_32(100L);
  updateMessageTop("Press Exit");
  while (gpio_get(exitbutton) != LOW) { //Wait until exit button is pressed
    busy_wait_ms(50);
  }
  //ShowMainDisplay(menuIndex);       // Draws top menu line 
  //ShowSubmenuData(minSWRAuto, dds.currentFrequency);                                
  //loop();
  //busy_wait_us_32(100L);
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
  float autotune;
  long localPosition, minCount, frequency;
  float currentSWR;
  updateMessageBottom("Initial Calibrate");
  //tft.setCursor(250, 12);
  //tft.print("1st Cal ");
  bandBeingCalculated = 0;
  stepper.ResetStepperToZero();
  position = 0;
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
      frequency = data.bandEdges[i][j];     // Select a band edge to calibrate
      dds.SendFrequency(frequency);    // Tell the DDS the edge frequency...
      //UpdateFrequency(frequency);      // Change main display data.  Replace with ShowSubmenuData, or not needed here???
      updateMessageTop("Moving to Freq");
      while (true) {
 
    if(DetectMaxSwitch()) return;

        position = stepper.currentPosition();
        while (swr.ReadSWRValue() > 5) {     //Move stepper in CW direction in larger steps for SWR>5
          position = position + 40;  // Better to always get this from StepperManagement???
          ShowSubmenuData(swr.ReadSWRValue(), dds.currentFrequency);
          stepper.MoveStepperToPositionCorrected(position);
          if(DetectMaxSwitch()) return;
        }
        
        autotune = AutoTuneSWR();  // AutoTuneSWR() returns 0 if failure.
        if(autotune == 0) return;
        minSWRAuto = autotune;              // Minimum SWR is returned if success.
  
 //       ShowSubmenuData(swr.ReadSWRValue(), dds.currentFrequency);
        if (minSWRAuto < TARGETMAXSWR) {                   //Ignore values greater than Target Max
          data.bandLimitPositionCounts[i][j] = SWRMinPosition;
          tft.setCursor(0, 90 + whichLine * TEXTLINESPACING);
          if (dds.currentFrequency < 10000000) {
            tft.print(" ");
          }
          tft.print(dds.currentFrequency);
          tft.setCursor(150, 90 + whichLine * TEXTLINESPACING);
          tft.print(minSWRAuto);
          tft.setCursor(230, 90 + whichLine * TEXTLINESPACING);
          tft.print(SWRMinPosition);
          whichLine++;                          // Ready for next line of output
          break;                                // This sends control to next edge
        }
      }
      position = stepper.currentPosition() - 50;
    }       // end for (j
    position = stepper.currentPosition() - 50;
  }         // end for (i

  eeprom.WritePositionCounts();               // Write values to EEPROM buffer.  Must also write them to Flash!
  eeprom.write(eeprom.bufferUnion.buffer8);   // This writes a page to Flash memory.  This includes the position counts
                                              // and preset frequencies.
  updateMessageBottom("Cal Complete");
  updateMessageTop("Press Exit");
  while (gpio_get(exitbutton) != LOW) {
    busy_wait_ms(10);  // Slow this loop down a bit to save power.
  }
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
  long frequency;
  frequency = 7150000L;
  dds.SendFrequency(frequency);
  busy_wait_us_32(100L);
  int i;

  float VSWR;
  while (gpio_get(enterbutton) != LOW)
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
  Does not reset the stepper.

  Argument list:
    void

  Return value:
    void
*****/
void DisplayManagement::DoSingleBandCalibrate(int whichBandOption) { //Al Added 4-18-20
  int bandBeingCalculated;
  int i, j, whichLine;
  long localPosition, minCount, frequency;
  float currentSWR;
  bandBeingCalculated = 0;
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
    frequency = data.bandEdges[whichBandOption][j];     // Select a band edge
    position = data.bandLimitPositionCounts[whichBandOption][j] - 50;
    stepper.MoveStepperToPositionCorrected(position); //Al 4-20-20
    dds.SendFrequency(frequency);        // Tell the DDS the edge frequency...
    while (true) {
      if (gpio_get(MAXSWITCH) != HIGH) {           // At the end stop switch?
        stepper.ResetStepperToZero();                         // Yep, so leave.
        return;
      }
      currentSWR = swr.ReadSWRValue();
      updateMessageTop("Auto Tuning");
      minSWRAuto = AutoTuneSWR();
      ShowSubmenuData(minSWRAuto, dds.currentFrequency);  //Update SWR value 
      if (minSWRAuto < TARGETMAXSWR) {                   //Ignore values greater than Target Max
        data.bandLimitPositionCounts[whichBandOption][j] = SWRMinPosition;
        tft.setCursor(0, 90 + whichLine * TEXTLINESPACING);
        if (dds.currentFrequency < 10000000) {
          tft.print(" ");
        }
        tft.print(dds.currentFrequency);
        tft.setCursor(150, 90 + whichLine * TEXTLINESPACING);
        tft.print(minSWRAuto);
        tft.setCursor(230, 90 + whichLine * TEXTLINESPACING);
        tft.print(SWRMinPosition);
        whichLine++;                          // Ready for next line of output
        //busy_wait_ms(200);
        break;                                // This sends control to next edge
      }
    }
    position = data.bandLimitPositionCounts[whichBandOption][1] - 50;
  }       // end for (j
  position = SWRFinalPosition + 50;
  eeprom.WritePositionCounts();           // Write values to EEPROM
  //busy_wait_us_32(100L);
  updateMessageTop("Press Exit");
  while (gpio_get(exitbutton) != LOW) {
    busy_wait_ms(50);  //  Slow this loop down to save power.
  }
}

/*****
  Purpose: To execute the Button1 Preset

  Argument list:
    void
  Return value:
    void
*****/
//void DisplayManagement::executeButton1() {
//  whichBandOption = SelectBand();            // Select the band to be used
//  submenuIndex = 0;
//  ProcessPresets(whichBandOption, submenuIndex);         // Select a preselected frequency
//  menuIndex = FREQMENU;                         // When done, start over...
  //ShowMainDisplay(menuIndex, SWR);   TEMPORARILY COMMENTED
  //ShowSubmenuData(SWR);              TEMPORARILY COMMENTED
//  dds.SendFrequency(dds.currentFrequency);
  //SWR = ReadNewSWRValue();           TEMPORARILY COMMENTED
  //EraseBelowMenu();                  TEMPORARILY COMMENTED

//}

/*****
  Purpose: To execute the Button3 Preset Select Menu Option

  Argument list:
    void
  Return value:
    void
*****/
//void DisplayManagement::executeButton3() {
//  DoNewCalibrate2();
//}

/******
  Purpose: The ISR to detect the QuickCal Button push

  Parameter list:
    void

  Return value:
    void
*****/
//void DisplayManagement::quickCalISR() {
//  quickCalFlag = 1;
//}

/*****
  Purpose: Select a preset frequency and AutoTune.

  Parameter list:
    int whichBandOption
    int submenuIndex          which of presets

  Return value:
    void

  CAUTION:

*****/
void DisplayManagement::ProcessPresets()
{
  int i;
  int backCount = 0;
  long frequency;
  State state = State::state0;
  //tft.setFont();
  //tft.setTextSize(2);
  state = State::state1;  // Begin with Select Band state.
  while(true) {
     switch(state) {
       case State::state0:
        return;  // Return to top level.
       case State::state1:
        whichBandOption = SelectBand(band);  // Select the band to be used
        submenuIndex = 0;
        state = State::state2;
        break;
       case State::state2:
        frequency = SelectPreset();       
        state = State::state3;
        if(frequency == 0) state = State::state0;
        break;
       case State::state3:
        dds.SendFrequency(frequency);
    // Calculate the approximate position for the stepper and back off a bit.
        position = -25 + data.bandLimitPositionCounts[whichBandOption][0] + float((dds.currentFrequency - data.bandEdges[whichBandOption][0])) / float(data.hertzPerStepperUnitVVC[whichBandOption]);
        stepper.MoveStepperToPositionCorrected(position); //Al 4-20-20
        minSWRAuto = AutoTuneSWR();
        ShowSubmenuData(minSWRAuto, dds.currentFrequency);
        GraphAxis(whichBandOption);
        PlotSWRValueNew(whichBandOption, iMax, tempCurrentPosition, tempSWR, SWRMinPosition);
        frequency = manualTune();
        state = State::state2;  // Move to Select Preset state.
        break;
     }
  }
}

  //ShowSubmenuData(swr.ReadSWRValue(), frequency);          // Draws SWR and Freq info
 // while (gpio_get(exitbutton) != LOW) {
 //   if (menuEncoderMovement != 0) {
 //     ManualStepperControl();
 //   }
 //   if (frequencyEncoderMovement != 0) {
 //     ManualFrequencyControl(whichBandOption);  // In SWR.cpp
 //     frequencyEncoderMovement = 0;
 //   }
 //   if (gpio_get(autotunebutton) == LOW && gpio_get(MAXSWITCH) != LOW) {   //Redo the Autotune at new frequency/position
 //     position = -80 +  data.bandLimitPositionCounts[whichBandOption][0]  + float((dds.currentFrequency - data.bandEdges[whichBandOption][0])) / float(data.hertzPerStepperUnitVVC[whichBandOption]);
 //     stepper.MoveStepperToPositionCorrected(position); //Al 4-20-20
 //     minSWRAuto = AutoTuneSWR();   //Auto tune here 
      //GraphAxis(whichBandOption);                   TEMPORARILY COMMENTED
      //PlotSWRValueNew(whichBandOption);             TEMPORARILY COMMENTED
      //updateMessageTop("Freq: Adjust - ATune: Refine");  TEMPORARILY COMMENTED
 //   }
   /* if (digitalRead(AUTOTUNE) == LOW && digitalRead(MAXSWITCH) != LOW) {
      currPosition = -80 +  bandLimitPositionCounts[whichBandOption][0]  + float((currentFrequency - bandEdges[whichBandOption][0])) / float(hertzPerStepperUnitVVC[whichBandOption]);
      stepper.setMaxSpeed(1000);
      stepper.setAcceleration(1100);
      MoveStepperToPositionCorrected(currPosition); //Al 4-20-20
      AutoTuneSWR();   //Auto tune here
      GraphAxis(whichBandOption);
      PlotSWRValueNew(whichBandOption);
      updateMessageTop("Freq: Adjust - ATune: Refine");
    }*/
//  }
  //graphFlag == 0;
  //EraseBelowMenu();        TEMPORARILY COMMENTED
  //ShowMainDisplay(0, SWR);       // Draws top menu line  TEMPORARILY COMMENTED
  //ShowSubmenuData(SWR);  TEMPORARILY COMMENTED
//}

int DisplayManagement::SelectPreset() {
  int frequency;
  updateMessageTop("Select Preset Frequency");
  EraseBelowMenu();
  tft.setTextSize(1);
  tft.setFont(&FreeSerif12pt7b);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);       // Show presets for selected band
  for (int i = 0; i < PRESETSPERBAND; i++) {
    tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
    tft.setCursor(30, 70 + i * 30);
    tft.print(i + 1);
    tft.print(".");
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.setCursor(65, 70 + i * 30);
    tft.print(data.presetFrequencies[whichBandOption][i]);
  }
  tft.setTextColor(ILI9341_MAGENTA, ILI9341_WHITE);
  tft.setCursor(65, 70 + submenuIndex * 30);
  tft.print(data.presetFrequencies[whichBandOption][submenuIndex]);
  menuEncoderState = 0;
  //  Preset state selection machine
  while (gpio_get(autotunebutton) == true and gpio_get(enterbutton) == true) {   // Why 2 buttons???
    if(gpio_get(exitbutton) == false) return frequency = 0;  // Exit Preset Select if requested by user.
    if (menuEncoderMovement == 1) {                              // Turning clockwise
      RestorePreviousPresetChoice(submenuIndex, whichBandOption);
      submenuIndex++;
      if (submenuIndex > PRESETSPERBAND - 1)
        submenuIndex = 0;
      HighlightNewPresetChoice(submenuIndex, whichBandOption);
      menuEncoderMovement = 0;
    }
    if (menuEncoderMovement == -1) {                             // Tuning counter-clockwise
      RestorePreviousPresetChoice(submenuIndex, whichBandOption);
      submenuIndex--;
      if (submenuIndex < 0)
        submenuIndex = PRESETSPERBAND - 1;
      HighlightNewPresetChoice(submenuIndex, whichBandOption);
      menuEncoderMovement = 0;
    }
  }  // end while Preset state selection machine
  frequency = data.presetFrequencies[whichBandOption][submenuIndex];   //  Retrieve the selected frequency.
  return frequency;
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
  tft.print(data.presetFrequencies[whichBandOption][submenuIndex]);
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
  tft.print(data.presetFrequencies[whichBandOption][submenuIndex]);
}

//  This is the primary auto-tuning algorithm which minimizes VSWR.
float DisplayManagement::AutoTuneSWR() {
  float oldMinSWR;
  oldMinSWR = 100;
  minSWRAuto = 100;
  int i;
  long currPositionTemp;
  SWRMinPosition = 4000;
  updateMessageTop("Auto Tuning");
  for (int i = 0; i < MAXNUMREADINGS; i++) {   //reset temp arrays - used to plot SWR vs frequency
    tempSWR[i] = 0.0;           // Class member
    tempCurrentPosition[i] = 0; // Class member
  }
  for (i = 0; i < MAXNUMREADINGS; i++) {       // loop to increment and find min SWR and save values to plot

    if (gpio_get(MAXSWITCH) == LOW) break;  // Break from this for loop due to limit switch closure.
    
    minSWR = swr.ReadSWRValue();                   //Save minimum SWR value
    ShowSubmenuData(minSWR, dds.currentFrequency);
    tempSWR[i] = minSWR;                       //Array of SWR values
    tempCurrentPosition[i] = stepper.currentPosition();     //Array of Count position values
    if ( minSWR < minSWRAuto) {             // Test to find minimum SWR value
      minSWRAuto = minSWR;
      SWRMinPosition = stepper.currentPosition();
      stepper.MoveStepperToPositionCorrected(SWRMinPosition);  // Redundant???
    }
    if (minSWR > 3 and whichBandOption == 0) {   //Fast step for 40M band above SWR = 3
      position = position + 10;
      i = i + 10;
    }
    else
    {
      position++;
    }
    stepper.MoveStepperToPositionCorrected(position);
    if (stepper.currentPosition() > SWRMinPosition + 10 and minSWR > minSWRAuto + 1.5 and minSWRAuto < 3.5) {   //Test to find if position is after minimum
      break;                                                                                      //if after minimum break out of for loop
    }
    SWRFinalPosition = stepper.currentPosition(); // Save final value for calibrate to continue to find band end positions
    if (i > 498) {                                //Repeat loop if minimum is not found
      i = 1;
      position = stepper.currentPosition() - 50;
    }
    }  // end of min SWR search loop.
  
  // Break to here if MAXSWITCH state change detected.
  if(DetectMaxSwitch())  return 0;  // 0 indicates failed AutoTune.
  
  // To this else if AutoTune is successful.
      else {
  position = SWRMinPosition;
  stepper.MoveStepperToPositionCorrected(SWRMinPosition - 50);   // back up position to take out backlash
  busy_wait_us_32(200);
  stepper.MoveStepperToPositionCorrected(SWRMinPosition);        //Move to final position in CW direction
  minSWR = swr.ReadSWRValue();  //  Measure VSWR in the final position.
  iMax = i; //max value in array for plot
  // These lines moved to after each call of AutoTuneSWR in DisplayManagement.  PUT BACK
  ShowSubmenuData(minSWR, dds.currentFrequency);  //Update SWR value
  //tft.fillRect(0, PIXELHEIGHT - 47, 311, 29, ILI9341_BLACK);   //Clear lower screen
  tft.fillRect(90, 0, 300, 20, ILI9341_BLACK);  // Clear text "Auto Tuning"
  tft.drawFastHLine(0, 20, 320, ILI9341_RED);
      }
  return minSWR;
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
float DisplayManagement::AutoTuneSWRQuick() {    //Al Modified 9-14-19

  //tft.fillScreen(ILI9341_BLACK);  // TEMPORARY erase screen
  //tft.setTextSize(2);
  //tft.setCursor(10, 20);

  float oldMinSWR = minSWRAuto = 100;
  int i;
  long currPositionTemp;
  SWRMinPosition = 0;
  /*
  if (gpio_get(ACCURACYBUTTON) == LOW) {
    currPositionTemp = currPosition;
    steppermanage.ResetStepperToZero();
    setCurrentPosition(0);
    currPosition = currPositionTemp;
  }
  */
 stepper.ResetStepperToZero();
 //stepper.setCurrentPosition(0);
 positionTemp = 0;
 position = 0;
  updateMessageTop("Auto Quick Tuning");
  busy_wait_us_32(100);

  //stepper.setMaxSpeed(5000);
  //stepper.setAcceleration(1100);
  //MoveStepperToPositionCorrected(currPosition);
  //steppermanage.MoveStepperToPositionCorrected(2500);   //  Move stepper to mid-range.
  //stepper.setMaxSpeed(500);

  for (i = 0; i < MAXNUMREADINGS; i++) {       // loop to increment and find min SWR and save values to plot
    minSWR = swr.ReadSWRValue();
  
  tft.setCursor(90, 100);
  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds("100.00", 90, 100, &x1, &y1, &w, &h);
  tft.fillRect(x1, y1, w, h, ILI9341_BLACK);
  tft.print(minSWR);


  ShowSubmenuData(minSWR, dds.currentFrequency);
    if ( minSWR < minSWRAuto) {             // Test to find minimum SWR value
      minSWRAuto = minSWR;                  // Reset to the minimum SWR so far.
      SWRMinPosition = stepper.currentPosition();
      //MoveStepperToPositionCorrected(SWRMinPosition);
    }
    if (minSWR > 3.0 ) {
      //if (minSWR > 3 and whichBandOption == 0) {
      //stepper.setMaxSpeed(FASTMOVESPEED);
      position = position + 20;
      i = i + 20;
    }
    else
    {
      //stepper.setMaxSpeed(NORMALMOVESPEED);
      position++;
    }
    if (i > 498) {
      i = 1;
      position = position - 50;
    }
    stepper.MoveStepperToPositionCorrected(position);
    if ((position > (SWRMinPosition + 5)) and (minSWR > minSWRAuto + 1) and (minSWRAuto < 3.5)) { //Test to find if position is after minimum
      break;                                                             //if after minimum break out of for loop
    }

    SWRFinalPosition = stepper.currentPosition();// Save final value to continue to find band end positions
  } // for (true)
  minSWR = swr.ReadSWRValue();
  position = SWRMinPosition;
  stepper.MoveStepperToPositionCorrected(SWRMinPosition - 20);    // back up position to take out backlash
  busy_wait_us_32(100);
  stepper.MoveStepperToPositionCorrected(SWRMinPosition);         //Move to final position in CW direction 
  iMax = i; //max value in array for plot
  //  Call this line after every call of AutoTuneSWRQuick in DisplayManagment:
  ShowSubmenuData(minSWRAuto, dds.currentFrequency);  //Update SWR value

  return minSWR;
}

// Manual control functions were moved from SWR.
/*****
  Purpose: Manual Setting the Frequency

  Parameter list:

  Return value:
    void
*****/
void DisplayManagement::ManualFrequencyControl(int whichBandOption) {
  //Serial.print("ManualFrequencyControl  "); 
  updateMessageTop("Press Freq: Move to Freq");
  int i, k, yIncrement, xIncrement;
  int stepIncr;
  long frequency, frequencyOld;
  long tempTime;
  xIncrement = (XAXISEND - XAXISSTART ) / 3;
  yIncrement = (YAXISEND - YAXISSTART) / 3;
  int xDotIncrement = 10;
  int yTick = YAXISSTART + 5;
  frequencyEncoderMovement = 0;
  GraphAxis(whichBandOption);
  if (frequencyEncoderMovement2 != 0) {
    frequencyOld = dds.currentFrequency;
    while (gpio_get(exitbutton) != LOW) {
      
      if (frequencyEncoderMovement2 != 0) {
        frequency = dds.currentFrequency + frequencyEncoderMovement2 * 1000;
        UpdateFrequency(frequency);  // Updates display only.
        dds.SendFrequency(frequency);
        frequencyEncoderMovement2 = 0;
      }
   
    }
    updateMessageTop("Freq: Adjust - ATune: Refine");
    dds.SendFrequency(frequency);
    position = stepper.currentPosition() + ((frequency - frequencyOld) / (data.hertzPerStepperUnitVVC[whichBandOption]));
    stepper.MoveStepperToPositionCorrected(position); //Al 4-20-20
    // Backlash code removed.
    //position -= 20;
    //stepper.MoveStepperToPositionCorrected(position); //Al 4-20-20
    //position += 20;
    //stepper.MoveStepperToPositionCorrected(position); //Al 4-20-20
    //readSWRValue = swr.ReadSWRValue();
   // Serial.print("readSWRValue=  "); Serial.println(readSWRValue);
//    delay(100);
    int k = 0;
    frequencyEncoderMovement = 0;
    frequencyEncoderMovement2 = 0;
  }
  updateMessageTop("Freq: Adjust - ATune: Refine");
  ShowSubmenuData(readSWRValue, frequency);
  //readSWRValue = swr.ReadSWRValue();
  PlotNewStartingFrequency(whichBandOption);
  ShowSubmenuData(swr.ReadSWRValue(), frequency);
}

/*****
  Purpose: Manual Setting the Stepper

  Parameter list:

  Return value:
    void
*****/
void DisplayManagement::ManualStepperControl() {
  long position;
  position = stepper.currentPosition() + menuEncoderMovement;
  stepper.MoveStepperToPositionCorrected(position); //Al 4-20-20
  // Backlash code removed.
  //position -= 20;
  //stepper.MoveStepperToPositionCorrected(position); //Al 4-20-20
  //position += 20;
  //stepper.MoveStepperToPositionCorrected(position); //Al 4-20-20
  
  ShowSubmenuData(swr.ReadSWRValue(), dds.currentFrequency);
  UpdateFrequency(dds.currentFrequency);
  menuEncoderMovement = 0;
}


/*****
  Purpose: Detect state change of MAXSWITCH.  Warn the user.
           Back the stepper off the switch so that it goes back to normal state.

  Parameter list:
    

  Return value:
    int

  CAUTION:

*****/
int DisplayManagement::DetectMaxSwitch()
{
  if(gpio_get(MAXSWITCH) == LOW) {
stepper.move(-300);
stepper.runToPosition();
for(int i = 0; i < 10; i++) {
updateMessageTop("Upper Limit Hit!");
busy_wait_ms(1000);
tft.fillRect(90,0,300,20,ILI9341_BLACK);
busy_wait_ms(1000);
}
return 1;
  }
  return 0;
}
