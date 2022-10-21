/* A "proof of concept" project to replace the STM32F103 "Blue Pill"
   which is used in the "Magnetic Loop Controller" described in the book
   "Microcontroller Projects for Amateur Radio by Jack Purdum, W8TEE, and
   Albert Peter, AC8GY" with the Raspberry Pi Pico.
   Copyright (C) 2022  Gregory Raven

                                                    LICENSE AGREEMENT

  This program source code and its associated hardware design at subject to the GNU General Public License version 2,
                  https://opensource.org/licenses/GPL-2.0
  with the following additional conditions:
    1. Any commercial use of the hardware or software is prohibited without express, written, permission of the authors.
    2. This entire comment, unaltered, must appear at the top of the primary source file. In the Arduino IDE environemnt, this comment must
       appear at the top of the INO file that contains setup() and loop(). In any other environmentm, it must appear in the file containing
       main().
    3. This notice must appear in any derivative work, regardless of language used.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    A copy of the GPL-2.0 license is included in the repository as file LICENSE.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

#include "DisplayManagement.h"

DisplayManagement::DisplayManagement(Adafruit_ILI9341 &tft, DDS &dds, SWR &swr,
                                     StepperManagement &stepper, EEPROMClass &eeprom, Data &data, FrequencyInput &freqInput) : GraphPlot(tft, dds, data), tft(tft), dds(dds), swr(swr),
                                                     stepper(stepper), eeprom(eeprom), data(data), freqInput(freqInput)                                           
{
  enterbutton = Button(data.enterButton);
  autotunebutton = Button(data.autotuneButton);
  exitbutton = Button(data.exitButton);
  enterbutton.initialize();
  exitbutton.initialize();
  autotunebutton.initialize();
}

void DisplayManagement::Splash(std::string version, std::string releaseDate)
{
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_MAGENTA, ILI9341_BLACK);
  tft.setCursor(22, 20);
  tft.print("Loop Antenna Controller");
  tft.setCursor(37, 45);
  tft.print("by Gregory Raven KF5N");
  tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
  tft.setCursor(25, 73);
  tft.print("based on a project from");
  tft.setCursor(15, 93);
  tft.print("Microcontroller Projects");
  tft.setCursor(30, 110);
  tft.print("for Amateur Radio by");
  tft.setCursor(65, 142);
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  tft.println("Al Peter  AC8GY");
  tft.setCursor(45, 168);
  tft.print("Jack Purdum  W8TEE");
  tft.setCursor(75, 200);
  tft.print("Version ");
  tft.print(version.c_str());
  tft.setCursor(35, 220);
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
void DisplayManagement::frequencyMenuOption()
{
  int SWRFlag1;
  int backCount = 0;
  long aveMinPosition;
  long frequency;
  State state;           // State machine next state.
  state = State::state1; // Begin with Select Band state.
                         // A state machine follows.  Run AutoTune when AutoTune button pushed.  Exit switch leaves while loop.
  while (true)
  {
    switch (state)
    {
    case State::state0:
      return; //  Exit frequency selection and return to top menu Freq.
    case State::state1:
      whichBandOption = SelectBand(data.bands); // state1
      
      // If SelectBand returns 4, this means the menu was exited without selecting a band.  Move to the top level menu Freq.
      if (whichBandOption == 4)
      {
        state = State::state0;
        break;
      }
      if(whichBandOption == this->data.workingData.currentBand) frequency = data.workingData.currentFrequency;
      else frequency = data.workingData.presetFrequencies[whichBandOption][3]; // Set initial frequency for each band from Preset list
      this->data.workingData.currentBand = whichBandOption;  //  Update the current band.   
      state = State::state2;                                  // Proceed to manual frequency adjustment state.
      break;
    case State::state2:
      frequency = ChangeFrequency(whichBandOption, frequency); // Alter the frequency using encoders.  Enter button returns frequency.
      // Exit frequency change if a zero is returned.  The user pushed Exit.
      if (frequency == 0)
      {
        state = State::state1;
        break;
      }
      dds.SendFrequency(frequency); // Done in ChangeFrequency???
      data.workingData.currentFrequency = frequency;
      eeprom.put(0, data.workingData);
      eeprom.commit();  // Write to EEPROM.
      Power(true, true);                  // Power up circuits.
      SWRValue = swr.ReadSWRValue();
      readSWRValue = SWRValue; // Redundant???
      ShowSubmenuData(SWRValue, frequency);
      tft.fillRect(0, 100, 311, 150, ILI9341_BLACK); // ???
      // Backup 20 counts to approach from CW direction
      position = -50 + data.workingData.bandLimitPositionCounts[whichBandOption][0] + float((frequency - data.workingData.bandEdges[whichBandOption][0])) / float(data.hertzPerStepperUnitVVC[whichBandOption]);
      //  Move the stepper to the approximate location based on the current frequency:
      stepper.MoveStepperToPositionCorrected(position); // Al 4-20-20
      minSWRAuto = AutoTuneSWR();                       // Auto tune here
      // After AutoTune, do full update of display with SWR vs. frequency plot:
      ShowSubmenuData(minSWRAuto, dds.currentFrequency);
      GraphAxis(whichBandOption);
      PlotSWRValueNew(whichBandOption, iMax, tempCurrentPosition, tempSWR, SWRMinPosition);
      // Manual frequency and stepper position moved to different menu!  September 2022
      //updateMessageBottom("     Encoders to Adjust, Exit to Return");
      // Enter Manual frequency and position tune:
      //frequency = manualTune();  // Stuck here until Exit button is pushed.
      Power(false, false); // Power down circuits.
      // Pause and allow user to view plot of SWR versus frequency.
      busy_wait_ms(5000);
      break;        //  state is not changed; should go back to state2.
    }               // end switch
  }                 // end of while loop and state machine
  return;
}

/*****
  Purpose: Manage the manual frequency and stepper functions.
  Argument list:
    void
  Return value:
    int frequency
*****/
int DisplayManagement::manualTune()
{
  bool lastautotunebutton = true;
  bool lastexitbutton = true;
  // Power(true);
  while (true)
  {
    exitbutton.buttonPushed(); // Poll exitbutton.

    if (exitbutton.pushed and not lastexitbutton)
    {
      //  Power(false);
      return dds.currentFrequency; // Exit manual tuning.
    }
    if (menuEncoderMovement != 0)
    { // Allow stepper to be moved maually
      ManualStepperControl();
    }
    if (frequencyEncoderMovement != 0)
    { // Allow frequency to be changed maually.
      ManualFrequencyControl(whichBandOption);
      frequencyEncoderMovement = 0; // Doesn't reset to 0???
    }
    autotunebutton.buttonPushed(); // Poll autotunebutton.
                                   // Is this MAXSWITCH protection needed here???
    if (autotunebutton.pushed == true and (not lastautotunebutton) and gpio_get(MAXSWITCH))
    { // Redo the Autotune at new frequency/position
      position = -80 + data.workingData.bandLimitPositionCounts[whichBandOption][0] + float((dds.currentFrequency - data.workingData.bandEdges[whichBandOption][0])) / float(data.hertzPerStepperUnitVVC[whichBandOption]);
      stepper.MoveStepperToPositionCorrected(position); // Al 4-20-20
      minSWRAuto = AutoTuneSWR();                       // Auto tune here
      SWRMinPosition = stepper.currentPosition();       // Get the autotuned stepper position.
      GraphAxis(whichBandOption);
      PlotSWRValueNew(whichBandOption, iMax, tempCurrentPosition, tempSWR, SWRMinPosition);
      updateMessageTop("    Freq: Adjust - AutoTune: Refine");
      updateMessageBottom("                  Exit to Return");
    }
    lastautotunebutton = autotunebutton.pushed;
    lastexitbutton = exitbutton.pushed;

  } // end while
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
long DisplayManagement::ChangeFrequency(int bandIndex, long frequency) // Al Mod 9-8-19
{
  int i, changeDigit, digitSpacing, halfScreen, incrementPad, insetMargin, insetPad;
  long defaultIncrement;
  insetPad = 57; // Used to align digit indicator
  incrementPad = 05;
  digitSpacing = 10;
  insetMargin = 20;
  defaultIncrement = 1000L;
  halfScreen = PIXELHEIGHT / 2 - 25;
  bool lastexitbuttonPushed = true;
  bool lastautotunebuttonPushed = true;
  updateMessageTop("                 Enter Frequency");
  tft.drawFastHLine(0, 20, 320, ILI9341_RED);
  if (bandIndex == 0)
  {                // 40M
    insetPad = 32; // smaller number, so less spacing to a given digit
  }
  //  The following configures the display for frequency selection mode.
  EraseBelowMenu();
  tft.setTextSize(1);
  tft.setFont(&FreeSerif9pt7b);
  tft.setTextColor(ILI9341_WHITE); // Messages
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
  tft.print("_");                                                                                 // underline selected character position
  tft.setTextColor(ILI9341_GREEN);
  tft.setCursor(insetMargin, halfScreen);
  tft.setTextSize(1);
  tft.setFont(&FreeSerif24pt7b);
  tft.print(frequency);
  tft.setFont(&FreeSerif24pt7b);
  // Print the SWR limit frequencies to the display.
  PrintSWRlimits(fpair);

  // State Machine for frequency input with encoders.
  while (true)
  { // Update frequency until user pushes AutoTune button.
    // Poll autotunebutton and exitbutton.
    autotunebutton.buttonPushed();
    exitbutton.buttonPushed();
    if (autotunebutton.pushed & not lastautotunebuttonPushed)
      break;
    // Make sure there is a proper transition of the autotune button.
    lastautotunebuttonPushed = autotunebutton.pushed;
    //  Exit this menu, but make sure it is a proper edge transition:
    if (exitbutton.pushed & not lastexitbuttonPushed)
    {
      frequency = 0;
      return frequency;
    }
    lastexitbuttonPushed = exitbutton.pushed;
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.setFont(&FreeSerif24pt7b);
    if (digitEncoderMovement == 1)
    { // Change frequency digit increment
      tft.fillRect(0, halfScreen + 6, PIXELWIDTH * .90, 20, ILI9341_BLACK);
      defaultIncrement /= 10;
      if (defaultIncrement < 1)
      { // Don't go too far right
        defaultIncrement = 1L;
      }
      incrementPad += INCREMENTPAD;
      if (defaultIncrement > 1000000L)
      {
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
    else
    {
      if (digitEncoderMovement == -1)
      {
        tft.fillRect(0, halfScreen + 6, PIXELWIDTH * .90, 20, ILI9341_BLACK);
        defaultIncrement *= 10;
        if (defaultIncrement > 1000000)
        { // Don't go too far right
          defaultIncrement = 1000000L;
        }
        incrementPad -= INCREMENTPAD;
        if (incrementPad < -INCREMENTPAD * 3) // Don't undershoot either
          incrementPad += INCREMENTPAD;

        tft.setCursor(insetMargin + (insetPad + incrementPad) + digitSpacing * 6 - 28, halfScreen + 5); // Assume 1KHz increment
        tft.print("_");
        digitEncoderMovement = 0;
      }
    }
    tft.setTextColor(ILI9341_GREEN);
    digitEncoderMovement = 0;
    menuEncoderMovement = 0;
    if (frequencyEncoderMovement)
    { // Change digit value
      frequency += (long)(frequencyEncoderMovement * defaultIncrement);
      position = stepper.ConvertFrequencyToStepperCount(frequency);
      tft.fillRect(insetMargin, halfScreen - 35, PIXELWIDTH * .80, 40, ILI9341_BLACK);
      tft.setCursor(insetMargin, halfScreen);
      tft.setTextSize(1);
      tft.setFont(&FreeSerif24pt7b);
      tft.print(frequency);
      frequencyEncoderMovement = 0L; // Reset encoder flag
    }
  }                   // end while loop
  tft.setTextSize(2); // Back to normal
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
int DisplayManagement::MakeMenuSelection(int index) // Al Mod 9-8-19
{
  tft.setFont();
  tft.setTextSize(2);
  int i;
  bool lastPushed;

  // State Machine:
  while (true)
  {
    // Poll enterbutton.
    enterbutton.buttonPushed();
    //   if(enterbutton.pushed & not enterbutton.lastPushed) break;  // Looking for a low to high transition here!
    if (enterbutton.pushed)
      break; // Looking for a low to high transition here!
             //   lastPushed = enterbutton.pushed;
    if (menuEncoderMovement)
    { // Must be i (CW) or -1 (CCW)
      if (menuEncoderMovement == 1)
      {
        index++;
        if (index == MAXMENUES)
        { // wrap to first index
          index = 0;
        }
      }
      if (menuEncoderMovement == -1)
      {
        index--;
        if (index < 0)
        { // wrap to first index
          index = MAXMENUES - 1;
        }
      }
      menuEncoderMovement = 0;
      tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
      for (i = 0; i < MAXMENUES; i++)
      {
        tft.setCursor(i * 100, 0);
        tft.print(menuOptions[i].c_str());
      }
      tft.setTextColor(ILI9341_BLUE, ILI9341_WHITE);
      tft.setCursor(index * 100, 0);
      tft.print(menuOptions[index].c_str());
    }
  } // end State Machine

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
  updateMessageTop("       Choose using Menu Encoder");
  EraseBelowMenu();              // Redundant???
  //int currBand[] = {40, 30, 20}; // Used???
  int i, index, where = 0;
  bool enterLastPushed = true; // Must be set to true or a false exit could occur.
  bool exitLastPushed = true;
  updateMessageBottom("             Press Enter to Select");
  tft.setTextSize(1);
  tft.setFont(&FreeSerif12pt7b);
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  for (int i = 0; i < 3; i++)
  {
    tft.setCursor(110, 110 + i * 30);
    tft.print(bands[i].c_str());
  }
  tft.setCursor(110, 110);
  tft.setTextColor(ILI9341_BLUE, ILI9341_WHITE);
  tft.print(bands[0].c_str());
  index = 0;

  // State Machine.  Calling this function enters this loop and stays until Enter or Exit is pressed.
  while (true)
  {
    if (menuEncoderMovement)
    {
      if (menuEncoderMovement == 1)
      {
        index++;
        if (index == 3)
        { // wrap to first index
          index = 0;
        }
      }
      if (menuEncoderMovement == -1)
      {
        index--;
        if (index < 0)
        { // wrap to last index
          index = 2;
        }
      }
      menuEncoderMovement = 0;
      tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
      for (int i = 0; i < 3; i++)
      {
        tft.setCursor(110, 110 + i * 30);
        tft.print(bands[i].c_str());
      }
      tft.setTextColor(ILI9341_BLUE, ILI9341_WHITE);
      tft.setCursor(110, 110 + index * 30);
      tft.print(bands[index].c_str());
    }
    // Poll buttons.
    enterbutton.buttonPushed();
    exitbutton.buttonPushed();
    if (enterbutton.pushed & not enterLastPushed)
      break; // Exit the state machine if there was a false to true transition, return selected index.
    enterLastPushed = enterbutton.pushed;
    if (exitbutton.pushed & not exitLastPushed)
      return index = 4; // 4 is a signal that the menu was exited from without making a selection.
    exitLastPushed = exitbutton.pushed;
  } // end while

  //currentBand = currBand[index]; // Used???
//   eeprom.WriteCurrentBand(index);
 //  data.workingData.currentBand = index;               
 //  eeprom.commit();
  return index;
}

/*****
  Purpose: To erase the display below the top two menu lines
  Argument list:
    void
  Return value:
    void
*****/
void DisplayManagement::EraseBelowMenu() // al mod 9-8-19
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
  for (i = 0; i < 3; i++)
  {
    if (i == whichMenuPage)
    {
      tft.setTextColor(ILI9341_BLUE, ILI9341_WHITE);
    }
    else
    {
      tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
    }
    tft.setCursor(i * 100, 0);
    tft.print(menuOptions[i].c_str());
  }
}

/*****
  Purpose: To display the SWR and frequency data
  Argument list:
    float SWR, the current SWR value to be displayed
    int currentFrequency, the frequency to be displayed
  Return value:
    void
*****/
void DisplayManagement::ShowSubmenuData(float SWR, int currentFrequency) // al mod 9-8-19
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
  {                  // Real or bogus SWR?
    tft.print("??"); //...bogus
  }
  else
  {
    if (SWR > 9.9999)
    {
      tft.print(SWR, 2);
    }
    else
    {
      tft.print(SWR, 2); // real
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
  if (msg.size() > 0)
  {
    tft.print(msg.c_str());
  }
  else
  {
    if (SWR > .5 && SWR < 50.0)
    {
      tft.print(SWR);
    }
    else
    {
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
void DisplayManagement::updateMessageTop(std::string messageToPrint)
{
  tft.fillRect(0, 0, 320, 20, ILI9341_BLACK); // Erase top line.
  tft.drawFastHLine(0, 20, 320, ILI9341_RED);
  tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
  tft.setTextSize(1);
  tft.setFont(&FreeSerif9pt7b);
  tft.setCursor(10, 12);
  tft.print(messageToPrint.c_str());
}

/*****
  Purpose: Update Bottom Message Area

  Argument list:
    char message

  Return value:
    void
*****/
void DisplayManagement::updateMessageBottom(std::string messageToPrint)
{
  tft.fillRect(0, 200, 319, 240, ILI9341_BLACK); // Erase previous message.
  tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
  tft.setTextSize(1);
  tft.setFont(&FreeSerif9pt7b);
  tft.setCursor(10, 220);
  tft.print(messageToPrint.c_str());
}

/*****
  Purpose: To set the band end point counts
           Uses initial Band Edge counts for rapid re-calibrate.
  Argument list:
    void

  Return value:
    void
*****/
void DisplayManagement::DoNewCalibrate2() // Al modified 9-14-19.  Greg modified May 2022.
{
  int bandBeingCalculated;
  int i, j, whichLine;
  long localPosition, minCount, frequency;
  float currentSWR;
  updateMessageTop("            Full Calibrate");
  bandBeingCalculated = 0;
  Power(true, true);                  //  Power up circuits.
  stepper.ResetStepperToZero(); // Start off at zero
  tft.fillRect(0, 46, 340, 231, ILI9341_BLACK);
  updateMessageBottom("          Full Calibration in Progress");
  tft.drawFastHLine(0, 45, 320, ILI9341_RED);
  tft.setFont(&FreeSerif9pt7b);
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  tft.setCursor(0, 65);
  tft.print("Frequency               SWR       Count"); // Table header
  tft.setCursor(0, 90);                                 // Read to show mins...

  whichLine = 0;                                  // X coord for mins
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK); // Table data

  for (i = 0; i < MAXBANDS; i++)
  { // For the 3 bands...
    for (j = 0; j < 2; j++)
    {
      frequency = data.workingData.bandEdges[i][j];                    // Select a band edge
      position = data.workingData.bandLimitPositionCounts[i][j] - 200; // Set Band limit count -200 counts to approach band limit from CW direction
      stepper.MoveStepperToPositionCorrected(position);    // Move to the estimated position below the target.
      dds.SendFrequency(frequency);                        // Tell the DDS the edge frequency...
      UpdateFrequency(frequency);                          // Change main display data

      while (true)
      {
        if (gpio_get(MAXSWITCH) != HIGH)
        {                               // At the end stop switch?
          stepper.ResetStepperToZero(); // Reset back to zero
          return;
        }
        minSWRAuto = AutoTuneSWR();
        ShowSubmenuData(swr.ReadSWRValue(), dds.currentFrequency);
        // UpdateFrequency(frequency);  // Is this redundant???
        if (minSWRAuto < TARGETMAXSWR)
        { // Ignore values greater than Target Max
          data.workingData.bandLimitPositionCounts[i][j] = SWRMinPosition;
          tft.setCursor(0, 90 + whichLine * TEXTLINESPACING);
          if (frequency < 10000000)
          {
            tft.print(" ");
          }
          tft.print(dds.currentFrequency);
          tft.setCursor(150, 90 + whichLine * TEXTLINESPACING);
          tft.print(minSWRAuto);
          tft.setCursor(230, 90 + whichLine * TEXTLINESPACING);
          tft.print(SWRMinPosition);
          whichLine++; // Ready for next line of output
          break;       // This sends control to next edge
        }
      }
      position = SWRFinalPosition + 50;
    } // end for (j
    position = SWRFinalPosition + 50;
  } // end for (i

  eeprom.commit(); // Write values to EEPROM
  updateMessageTop("                    Press Exit");
  updateMessageBottom("         Full Calibration Complete");
  Power(false, false); //  Power down circuits.
  while (true)
  {
    exitbutton.buttonPushed(); // Poll exitbutton
    if (exitbutton.pushed)
      break;
  } // Wait until exit button is pressed
}

/*****
  Purpose: Do initial Calibrate.  Does not assume band edge counts to begin

  Argument list:
    void

  Return value:
    void
*****/

void DisplayManagement::DoFirstCalibrate() // Al modified 9-14-19
{
  int bandBeingCalculated;
  int i, j, whichLine;
  float autotune;
  long localPosition, minCount, frequency;
  float currentSWR;
  bool lastexitbutton = true;
  EraseBelowMenu();
  updateMessageBottom("                 Initial Calibration");
  bandBeingCalculated = 0;
  Power(true, true); //  Power up circuits.
  stepper.ResetStepperToZero();
  position = 0;
  tft.setFont(&FreeSerif9pt7b);
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  tft.setCursor(0, 65);
  tft.print("Frequency               SWR       Count"); // Table header
  tft.setCursor(0, 90);                                 // Read to show mins...

  whichLine = 0;                                  // X coord for mins
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK); // Table data

  for (i = 0; i < MAXBANDS; i = i + 1)
  { // For the 3 bands...
    for (j = 0; j < 2; j = j + 1)
    {
      this->data.workingData.currentBand = i;  // Used by SWRdataAnalysis()
      frequency = this->data.workingData.bandEdges[i][j]; // Select a band edge to calibrate
      this->data.workingData.currentFrequency = frequency;
      dds.SendFrequency(frequency);     // Tell the DDS the edge frequency...
      updateMessageTop("             Moving to Freq");
      while (true)
      {

        if (DetectMaxSwitch())
          return;

        position = stepper.currentPosition();
        while (swr.ReadSWRValue() > 5)
        {                           // Move stepper in CW direction in larger steps for SWR>5
          position = position + 20; // Reduced from 40 due to butterfly capacitor.
          ShowSubmenuData(swr.ReadSWRValue(), dds.currentFrequency);
          stepper.MoveStepperToPositionCorrected(position);
          if (DetectMaxSwitch())
            return;
        }

        autotune = AutoTuneSWR(); // AutoTuneSWR() returns 0 if failure.
        if (autotune == 0)
          return;
        minSWRAuto = autotune; // Minimum SWR is returned if success.

        if (minSWRAuto < TARGETMAXSWR)
        { // Ignore values greater than Target Max
          data.workingData.bandLimitPositionCounts[i][j] = SWRMinPosition;
          tft.setCursor(0, 90 + whichLine * TEXTLINESPACING);
          if (dds.currentFrequency < 10000000)
          {
            tft.print(" ");
          }
          tft.print(dds.currentFrequency);
          tft.setCursor(150, 90 + whichLine * TEXTLINESPACING);
          tft.print(minSWRAuto);
          tft.setCursor(230, 90 + whichLine * TEXTLINESPACING);
          tft.print(SWRMinPosition);
          whichLine++; // Ready for next line of output
          break;       // This sends control to next edge
        }
      }
      position = stepper.currentPosition() - 50;
    } // end for (j
    position = stepper.currentPosition() - 50;
  } // end for (i
  //  Set the calibrated flag in workingData.
  data.workingData.calibrated = 1;
  eeprom.put(0, data.workingData);
  eeprom.commit(); // This writes a page to Flash memory.  This includes the position counts
                                            // and preset frequencies.
  //  Now get the newly written values into the current session:
  //  Read the position counts and presets into the EEPROM object's buffer.
  //eeprom.get(0, data.workingData);
  //  Overwrite the position counts and preset frequencies:
  //eeprom.ReadPositionCounts();
  // Slopes can't be computed until the actual values are loaded from flash:
  data.computeSlopes();

  updateMessageBottom("        Initial Calibration Complete");
  updateMessageTop("                    Press Exit");
  Power(false, false); //  Power down circuits.
  while (true)
  {
    exitbutton.buttonPushed(); // Poll exitbutton.
    if (exitbutton.pushed and not lastexitbutton)
      break; // Check for positive edge.
    lastexitbutton = exitbutton.pushed;
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
void DisplayManagement::DoSingleBandCalibrate(int whichBandOption)
{ // Al Added 4-18-20
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
  tft.print("Frequency               SWR       Count"); // Table header
  tft.setCursor(0, 90);                                 // Read to show mins...

  whichLine = 0;                                  // X coord for mins
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK); // Table data
  updateMessageBottom("            Single Band Calibrate");
  for (j = 0; j < 2; j++)
  {                                                 // For each band edge...
    frequency = data.workingData.bandEdges[whichBandOption][j]; // Select a band edge
    position = data.workingData.bandLimitPositionCounts[whichBandOption][j] - 50;
    Power(true, true);                              //  Power up circuits.
    stepper.MoveStepperToPositionCorrected(position); // Al 4-20-20
    dds.SendFrequency(frequency);                     // Tell the DDS the edge frequency...
    while (true)
    {
      if (gpio_get(MAXSWITCH) != HIGH)
      {                               // At the end stop switch?
        stepper.ResetStepperToZero(); // Yep, so leave.
        return;
      }
      currentSWR = swr.ReadSWRValue();
      updateMessageTop("Auto Tuning");
      minSWRAuto = AutoTuneSWR();
      ShowSubmenuData(minSWRAuto, dds.currentFrequency); // Update SWR value
      if (minSWRAuto < TARGETMAXSWR)
      { // Ignore values greater than Target Max
        data.workingData.bandLimitPositionCounts[whichBandOption][j] = SWRMinPosition;
        tft.setCursor(0, 90 + whichLine * TEXTLINESPACING);
        if (dds.currentFrequency < 10000000)
        {
          tft.print(" ");
        }
        tft.print(dds.currentFrequency);
        tft.setCursor(150, 90 + whichLine * TEXTLINESPACING);
        tft.print(minSWRAuto);
        tft.setCursor(230, 90 + whichLine * TEXTLINESPACING);
        tft.print(SWRMinPosition);
        whichLine++; // Ready for next line of output
        break;       // This sends control to next edge
      }
    }
    position = data.workingData.bandLimitPositionCounts[whichBandOption][1] - 50;
  } // end for (j
  position = SWRFinalPosition + 50;
  eeprom.commit(); // Write values to EEPROM
  updateMessageTop("                     Press Exit");
  updateMessageBottom("     Single Band Calibrate Complete");
  Power(false, false); // Power down circuits.
  while (exitbutton.pushed == false)
  {
    exitbutton.buttonPushed(); // Poll exitbutton.
    if (exitbutton.pushed)
      break;
  }
}

/*****
  Purpose: State machine to select a preset frequency and then AutoTune to that frequency.

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
  state = State::state1; // Begin with Select Band state.
  while (true)
  {
    switch (state)
    {
    case State::state0:
      return; // Return to top level.
    case State::state1:
      whichBandOption = SelectBand(data.bands); // Select the band to be used
      this->data.workingData.currentBand = whichBandOption;
      // If SelectBand returns 4, the user exited before selecting a band.  Return to top menu.
      if (whichBandOption == 4)
      {
        state = State::state0;
        break;
      }
      submenuIndex = 0;
      state = State::state2;
      break;
    case State::state2:
      frequency = SelectPreset();  // This method contains code to modify and save the presets.
      state = State::state3;       // Return to this point and you MUST go to state3 (autotune)!
      if (frequency == 0)
        state = State::state1; // User pushed exit, return to band select.
      break;
    case State::state3: // Run AutoTuneSWR() at the selected preset frequency.
      Power(true, true);      // Power up circuits.
      dds.SendFrequency(frequency);
      this->data.workingData.currentFrequency = frequency;
      eeprom.put(0, data.workingData);
      eeprom.commit();
      // Calculate the approximate position for the stepper and back off a bit.
      position = -25 + data.workingData.bandLimitPositionCounts[whichBandOption][0] + float((dds.currentFrequency - data.workingData.bandEdges[whichBandOption][0])) / float(data.hertzPerStepperUnitVVC[whichBandOption]);
      stepper.MoveStepperToPositionCorrected(position); // Al 4-20-20
      minSWRAuto = AutoTuneSWR();
      ShowSubmenuData(minSWRAuto, dds.currentFrequency);
      GraphAxis(whichBandOption);
      PlotSWRValueNew(whichBandOption, iMax, tempCurrentPosition, tempSWR, SWRMinPosition);
      //frequency = manualTune();
      Power(false, false);          //  Power down circuits.
      busy_wait_ms(5000);
      state = State::state2; // Move to Select Preset state.
      break;
 //     case State::state4:  // This state adjusts and saves the preset frequency.

      default:  // Should never go here!
      return;   // Return nothing.  Should break things.
    }
  }
}

// Used in ProcessPreset to select a particular preset frequency.
// If the Enter button is pressed, the preset frequency can be changed and saved.
// This must be translated into a state machine!
int DisplayManagement::SelectPreset()
{
  int frequency;
  bool lastexitbutton = true;
  bool lastenterbutton = true;
  bool lastautotunebutton = true;
  menuEncoderState = 0;
  state = State::state0;  // Enter state0 which does graphics.
  
//  Preset state selection machine
  while (true)
  { 
    // Poll 3 buttons:
    autotunebutton.buttonPushed();
    enterbutton.buttonPushed();
    exitbutton.buttonPushed();

  switch(state) {
  case State::state0:    // This state does the graphics.
  updateMessageTop("Menu Encoder to select, push AutoTune");
  EraseBelowMenu();
  tft.setTextSize(1);
  tft.setFont(&FreeSerif12pt7b);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK); // Show presets for selected band
  for (int i = 0; i < PRESETSPERBAND; i++)
  {
    tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
    tft.setCursor(30, 70 + i * 30);
    tft.print(i + 1);
    tft.print(".");
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.setCursor(65, 70 + i * 30);
    tft.print(data.workingData.presetFrequencies[whichBandOption][i]);
  }
  tft.setTextColor(ILI9341_MAGENTA, ILI9341_WHITE);
  tft.setCursor(65, 70 + submenuIndex * 30);
  tft.print(data.workingData.presetFrequencies[whichBandOption][submenuIndex]);
  state = State::state1;
  break;

      case State::state1:  // This state reads the encoders and button pushes.
    if (menuEncoderMovement == 1)
    { // Turning clockwise
      RestorePreviousPresetChoice(submenuIndex, whichBandOption);
      submenuIndex++;
      if (submenuIndex > PRESETSPERBAND - 1)
        submenuIndex = 0;
      HighlightNewPresetChoice(submenuIndex, whichBandOption);
      menuEncoderMovement = 0;
    }
    if (menuEncoderMovement == -1)
    { // Tuning counter-clockwise
      RestorePreviousPresetChoice(submenuIndex, whichBandOption);
      submenuIndex--;
      if (submenuIndex < 0)
        submenuIndex = PRESETSPERBAND - 1;
      HighlightNewPresetChoice(submenuIndex, whichBandOption);
      menuEncoderMovement = 0;
    }
        if (exitbutton.pushed & not lastexitbutton)
      return frequency = 0; // Exit Preset Select if requested by user.
      lastexitbutton = exitbutton.pushed;
    if (enterbutton.pushed & not lastenterbutton) {
      frequency = data.workingData.presetFrequencies[whichBandOption][submenuIndex];
      frequency = freqInput.ChangeFrequency(data.workingData.currentBand, frequency); // This will return the current or modified preset frequency.
      // Save the preset to the EEPROM.
      data.workingData.presetFrequencies[whichBandOption][submenuIndex] = frequency;
      eeprom.put(0, data.workingData);
      eeprom.commit();
      //break;  // Want to stay within SelectPreset; do not leave this method!  Already within while loop here.
      // Need to refresh graphics, because they were changed by ChangeFrequency!  
      state = State::state0;  // Refresh the graphics.
      
    //  lastenterbutton = enterbutton.pushed;
    }
    lastenterbutton = enterbutton.pushed;
    if (autotunebutton.pushed & not lastautotunebutton)
      break; // Exit preset select, return the frequency and proceed to AutoTune.
      lastautotunebutton = autotunebutton.pushed;
    //  state = State::state1;
    break;
    default:
    break;
  }     // end switch of state machine
  }     // end while Preset state selection machine
  frequency = data.workingData.presetFrequencies[whichBandOption][submenuIndex]; //  Retrieve the selected frequency.
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
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK); // restore old background
  tft.setCursor(65, 70 + submenuIndex * 30);
  tft.print(data.workingData.presetFrequencies[whichBandOption][submenuIndex]);
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
  tft.setTextColor(ILI9341_MAGENTA, ILI9341_WHITE); // HIghlight new preset choice
  tft.setCursor(65, 70 + submenuIndex * 30);
  tft.print(data.workingData.presetFrequencies[whichBandOption][submenuIndex]);
}

//  This is the primary auto-tuning algorithm which minimizes VSWR.
//  The stepper should be positioned below the minimum SWR frequency.
//  This is done either by starting at stepper position 0, or using a calculated estimation.
float DisplayManagement::AutoTuneSWR()
{
  float oldMinSWR;
  oldMinSWR = 100;
  minSWRAuto = 100;
  int i;
  int32_t currPositionTemp;
  //  Store the band and frequency in the dataStruct:
  //data.workingData.currentBand = whichBandOption;
  //data.workingData.currentFrequency = dds.currentFrequency;
  //eeprom.put(0, data.workingData);
  //eeprom.commit();
  SWRMinPosition = 6000;
  position = stepper.currentPosition(); // Retrieve the entry position of the stepper.
  updateMessageTop("                    Auto Tuning");
  for (int i = 0; i < MAXNUMREADINGS; i++)
  {                             // reset temp arrays - used to plot SWR vs frequency
    tempSWR[i] = 0.0;           // Class member
    tempCurrentPosition[i] = 0; // Class member
  }
  // Main loop to sweep for minimum SWR and save data for plotting.
  for (i = 0; i < MAXNUMREADINGS; i++)
  {

    if (gpio_get(MAXSWITCH) == LOW)
      break; // Break from this for loop due to limit switch closure.

    minSWR = swr.ReadSWRValue();                        // Save SWR value
    ShowSubmenuData(minSWR, dds.currentFrequency);      // Update display during sweep.
    tempSWR[i] = minSWR;                                // Array of SWR values saved for plotting.
    tempCurrentPosition[i] = stepper.currentPosition(); // Array of Count position values saved for plotting.
    if (minSWR < minSWRAuto)
    {                                             // Test to find minimum SWR value
      minSWRAuto = minSWR;                        // If this measurement is lower, save it.
      SWRMinPosition = stepper.currentPosition(); // Save the stepper position.
      SWRMinIndex = i;                            // Save the array index of the minimum.
    }
    if (minSWR > 3 and whichBandOption == 0)
    {                          // Fast step for 40M band above SWR = 3
      position = position + 5; // This was reduced from 10 due to butterfly capacitor.
      i = i + 5;               // Skip forward by 10.
    }
    else
    {
      position = position + 1; // Increment forward by 1 step.
    }
    stepper.MoveStepperToPositionCorrected(position);
    if (stepper.currentPosition() > (SWRMinPosition + 10) and minSWR > (minSWRAuto + 1.5) and minSWRAuto < 3.5)
    {        // Test to find if position is after minimum
      break; // if after minimum break out of for loop
    }
    SWRFinalPosition = stepper.currentPosition(); // Save final value for calibrate to continue to find band end positions.  Needed???
    if (i > 498)
    { // Repeat loop if minimum is not found
      i = 1;
      position = stepper.currentPosition() - 50;
    }
  } // end of min SWR search loop.

  // Break to here if MAXSWITCH state change detected.
  if (DetectMaxSwitch())
    return 0; // 0 indicates failed AutoTune.

  // To this else if AutoTune is successful.
  else
  {
    stepper.MoveStepperToPositionCorrected(SWRMinPosition - 50); // back up position to take out backlash
    stepper.MoveStepperToPositionCorrected(SWRMinPosition);      // Move to final position in CW direction
    minSWR = swr.ReadSWRValue();                                 //  Measure VSWR in the final position.
    iMax = i;                                                    // max value in array for plot
    ShowSubmenuData(minSWR, dds.currentFrequency);               // Update SWR value
    updateMessageTop("               AutoTune Success");
    //  Compute the upper and lower frequencies at which SWR ~2:1.
    SWRdataAnalysis();
  }
  return minSWR;
}

// Manual control functions were moved from SWR.
/*****
  Purpose: Manual Setting the Frequency

  Parameter list:

  Return value:
    void
*****/
void DisplayManagement::ManualFrequencyControl(int whichBandOption)
{
  updateMessageTop("     Press Enter: Move to Freq");
  int i, k, yIncrement, xIncrement;
  int stepIncr;
  int frequency = dds.currentFrequency;
  int frequencyOld = frequency;
  long tempTime; // Used???
  xIncrement = (XAXISEND - XAXISSTART) / 3;
  yIncrement = (YAXISEND - YAXISSTART) / 3;
  int xDotIncrement = 10;
  int yTick = YAXISSTART + 5;
  bool lastenterbutton = true;
  frequencyEncoderMovement = 0;
  GraphAxis(whichBandOption);
  if (frequencyEncoderMovement2 != 0)
  {
    frequencyOld = dds.currentFrequency;
    // Enter this loop until enterbutton is pushed.
    while (true)
    {
      enterbutton.buttonPushed(); // Poll enterbutton.
      if (enterbutton.pushed and not lastenterbutton)
        break;
      lastenterbutton = enterbutton.pushed; // Used to make sure enterbutton uses edge.
      if (frequencyEncoderMovement2 != 0)
      {
        frequency = dds.currentFrequency + frequencyEncoderMovement2 * 1000;
        dds.SendFrequency(frequency);
        ShowSubmenuData(swr.ReadSWRValue(), frequency); // Updates display only.
        frequencyEncoderMovement2 = 0;
      }
    }
    updateMessageTop("                  Exit to Return");
    updateMessageBottom("     Freq: Adjust - AutoTune: Refine");
    dds.SendFrequency(frequency); // Redundant???
    position = stepper.currentPosition() + ((frequency - frequencyOld) / (data.hertzPerStepperUnitVVC[whichBandOption]));
    stepper.MoveStepperToPositionCorrected(position); // Al 4-20-20
    int k = 0;
    frequencyEncoderMovement = 0;
    frequencyEncoderMovement2 = 0;
  }
  PlotNewStartingFrequency(whichBandOption);
  ShowSubmenuData(swr.ReadSWRValue(), frequency);
}

/*****
  Purpose: Manual Setting the Stepper

  Parameter list:

  Return value:
    void
*****/
void DisplayManagement::ManualStepperControl()
{
  long position;
  position = stepper.currentPosition() + menuEncoderMovement;
  stepper.MoveStepperToPositionCorrected(position); // Al 4-20-20
  ShowSubmenuData(swr.ReadSWRValue(), dds.currentFrequency);
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
  if (gpio_get(MAXSWITCH) == LOW)
  {
    stepper.move(-300);
    stepper.runToPosition();
    for (int i = 0; i < 10; i++)
    {
      updateMessageTop("                  Upper Limit Hit!");
      busy_wait_ms(1000);
      tft.fillRect(90, 0, 300, 20, ILI9341_BLACK);
      busy_wait_ms(1000);
    }
    return 1;
  }
  return 0;
}

/*****
  Purpose: Select and execute user selected Calibration algorithm.
           Manage the display, Enter and Exit buttons

  Parameter list:


  Return value:
    void

  CAUTION:

*****/
void DisplayManagement::CalibrationMachine()
{
  int i;
  bool lastexitbutton = true;
  std::string cals[] = {"Full Cal", "Band Cal", "Initial Cal"};
  EraseBelowMenu();
  state = State::state0; // Enter state0.
  //menuIndex = mode::PRESETSMENU;         // Superfluous???
  while (true)
  {
    switch (state)
    {
    case State::state0:         // Select Calibration algorithm.
      i = SelectBand(cals) + 1; // Calibration states are 1,2,3.
      if (i == 5)
        return;         // No selection in Calibrate menu; exit machine and return to top level.
      state = (State)i; // Cast i to State enum type.
      break;
    case State::state1: // Full Calibration
      DoNewCalibrate2();
      state = State::state0;
      lastexitbutton = true; // Must set true here, or will jump to top level.
      break;
    case State::state2:           // Band Calibration
      i = SelectBand(data.bands); // Select band
      if (i == 4)
        break;                  // No selection in Band menu; return to Calibration select without calibrating.
      DoSingleBandCalibrate(i); // Band Calibration
      state = State::state0;    // Return to Calibration select after exiting state2.
      lastexitbutton = true;    // Must set true here, or will jump to top level.
      break;
    case State::state3: // Initial Calibration
      DoFirstCalibrate();
      state = State::state0;
      lastexitbutton = true; // Must set true here, or will jump to top level.
      break;
    }                          // end switch
    exitbutton.buttonPushed(); // Poll exitbutton.
    if (exitbutton.pushed and not lastexitbutton)
      break;                            // Break from while if exit button is pushed.
    lastexitbutton = exitbutton.pushed; // Make sure exit happens on edge.
  }                                     // end while
}

/*****
  Purpose: This function sets power on or off to all circuits, including the DDS.

  Parameter list:
    bool setpower

  Return value:
    void

  CAUTION:

*****/
void DisplayManagement::Power(bool setpower,bool relayPower)
{
  gpio_put(data.STEPPERSLEEPNOT, setpower); //  Deactivating the stepper driver is important to reduce RFI.
  // Power down the DDS or set frequency.
  if (setpower)
    dds.SendFrequency(dds.currentFrequency); // Redundant?
  else
    dds.SendFrequency(0);
  // Power down RF amplifier and SWR circuits.
  gpio_put(data.OPAMPPOWER, setpower);
  gpio_put(data.RFAMPPOWER, setpower);
  gpio_put(data.RFRELAYPOWER, relayPower);
  busy_wait_ms(500); //  Wait for relay to switch.
}

/*****
  Purpose: This function sets power on or off to only SWR measuring circuits, including the DDS.
           It does not affect the stepper driver sleep.

  Parameter list:
    bool setpower

  Return value:
    void

  CAUTION:

*****/
void DisplayManagement::PowerSWR(bool setpower)
{
  // Power down the DDS or set frequency.
  if (setpower)
    dds.SendFrequency(dds.currentFrequency); // Redundant?
  else
    dds.SendFrequency(0);
  // Power down RF amplifier and SWR circuits.
  gpio_put(data.OPAMPPOWER, setpower);
  gpio_put(data.RFAMPPOWER, setpower);
  gpio_put(data.RFRELAYPOWER, setpower);
  busy_wait_ms(500); //  Wait for relay to switch.
}


/*****
  Purpose: This function analyzes the SWR data and determines the frequency
           limits at which SWR is approximately 2:1.

  Parameter list:
    float SWRarray[500], uint32_t position[500] (array of stepper positions)
SWRdataAnalysis
  Return value:
    std::pair<uint32_t, uint32_t>

  CAUTION:

*****/
void DisplayManagement::SWRdataAnalysis()
{
  //long flow, fhigh;
  int32_t fcenter;
  int32_t flow = 0;
  int32_t fhigh = 0; 
  //Data data2 = Data();
  //data2.computeSlopes();
  float horseshit;
  horseshit = this->data.hertzPerStepperUnitVVC[0];
  int posLowIndex = 0;
  int posHighIndex = 0;
  for(int i = 0; i < 500; i = i + 1) {
   if((tempSWR[i] < 2.0) and (tempSWR[i] > 0.9)) {
    posLowIndex = i;  // This is a stepper position!
    break;  // Exit, and proceed to process upper half.
   }
   else posLowIndex = 0;  // For case where all values < 2.0.
   }
  for(int i = posLowIndex + 1; i < 500; i = i + 1) {
   if((tempSWR[i] > 2.0) and (tempSWR[i] > 0.9)) {
    posHighIndex = i;  // This is a stepper position!
    break;  // Exit, found upper 2:1 position.
  }
  else posHighIndex = 499;  // For case where all values < 2.0.
  }
  // Now calculate the end frequencies over which SWR is <2.0:1.
  // SWRMinPosition is the index the desired center frequency for AutoTune.
  // Need the band index to retrieve the slope.
  fcenter = dds.currentFrequency;
  flow    = fcenter - (SWRMinPosition - tempCurrentPosition[posLowIndex]) * static_cast<int32_t>(this->data.hertzPerStepperUnitVVC[this->data.workingData.currentBand]);
  fhigh   = fcenter + (tempCurrentPosition[posHighIndex] - SWRMinPosition) * static_cast<int32_t>(this->data.hertzPerStepperUnitVVC[this->data.workingData.currentBand]);
 //  flow    = SWRMinPosition - tempCurrentPosition[posLowIndex];
 // fhigh   = tempCurrentPosition[posHighIndex] - SWRMinPosition;
  fpair = {flow, fhigh};
//  return fpair;
}


/*****
  Purpose: This function analyzes the SWR data and determines the frequency
           limits at which SWR is approximately 2:1.

  Parameter list:
    float SWRarray[500], long position[500]

  Return value:
    void

  CAUTION:

*****/
void DisplayManagement::PrintSWRlimits(std::pair<uint32_t, uint32_t> fpair)
{
  tft.setFont(&FreeSerif9pt7b);
  tft.setCursor(20, 135);
  tft.print(fpair.first);
  tft.setCursor(150, 135);
  tft.print(fpair.second);  
}