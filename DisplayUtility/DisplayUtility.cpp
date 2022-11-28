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

#include "DisplayUtility.h"

//DisplayUtility::DisplayUtility(Adafruit_ILI9341 &tft, DDS &dds, SWR &swr,
//                                     EEPROMClass &eeprom, Data &data, Button &enterbutton, Button &autotunebutton, Button &exitbutton, FrequencyInput &freqInput, TuneInputs &tuneInputs) : GraphPlot(tft, dds, data), tft(tft), dds(dds), swr(swr),
//                                    eeprom(eeprom), data(data), enterbutton(enterbutton), autotunebutton(autotunebutton), exitbutton(exitbutton), freqInput(freqInput), tuneInputs(tuneInputs)

DisplayUtility::DisplayUtility(Adafruit_ILI9341 &tft, DDS &dds, SWR &swr, StepperManagement &stepper,
                                     EEPROMClass &eeprom, Data &data, Button &enterbutton, Button &autotunebutton, Button &exitbutton, FrequencyInput &freqInput, TuneInputs &tuneInputs) : tft(tft), dds(dds), swr(swr), stepper(stepper),
                                    eeprom(eeprom), data(data), enterbutton(enterbutton), autotunebutton(autotunebutton), exitbutton(exitbutton), freqInput(freqInput), tuneInputs(tuneInputs)
{
  startUpFlag = false;
  calFlag = false;
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

int32_t DisplayUtility::ChangeFrequency(int bandIndex, long frequency) // Al Mod 9-8-19
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
*/

/*****
  Purpose: To get a main menu choice:  Freq, Presets, or 1st Cal.

  Argument list:

  Return value:
    int          The menu selected

  Dependencies:  Adafruit_ILI9341

int DisplayUtility::MakeMenuSelection(int index) // Al Mod 9-8-19
{
  int currentFrequency;
  tft.setFont();
  tft.setTextSize(2);
  int i;
  bool lastPushed;
  bool autotuneLastPushed = true;

  // Check if initial calibration has been run.  Inform user if not.
  if (data.workingData.calibrated == 0)
  {
    //  Inform user to run Initial Calibration.
    tft.setCursor(55, 75);
    tft.setFont(&FreeSerif9pt7b);
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_RED);
    tft.print("Please run Initial Calibration!");
    tft.setTextColor(ILI9341_WHITE);
    tft.setCursor(20, 125);
    tft.print("Use the Menu encoder to highlight");
    tft.setCursor(20, 150);
    tft.print("Calibrate and push Enter. Using the");
    tft.setCursor(20, 175);
    tft.print("menu encoder again, highlight Initial");
    tft.setCursor(20, 200);
    tft.print("Cal, and push Enter.");
  }
  else
  {
    //  Retrieve the last used frequency and autotune if the user pushes the AutoTune button.
    currentFrequency = data.workingData.currentFrequency;
    // Display a message to the user that the AutoTune button will tune to the
    // last used frequency prior to power-off.  This is a one-time event at power-up.
    if (startUpFlag == false)
    {
      tft.setCursor(15, 135);
      tft.setFont(&FreeSerif9pt7b);
      tft.setTextSize(1);
      tft.print("Press AutoTune for last used frequency.");
    }
  }
  tft.setFont();
  tft.setTextSize(2);
  // State Machine:
  while (true)
  {
    // Poll enterbutton.
    enterbutton.buttonPushed();
    autotunebutton.buttonPushed();
    if (autotunebutton.pushed & not autotuneLastPushed & not startUpFlag)
    {
      currentFrequency = data.workingData.currentFrequency;

      if (currentFrequency != 0)
      {

        dds.SendFrequency(currentFrequency); // Set the DDSs
                                             // Retrieve the last used frequency and autotune.
        // int32_t position = -25 + data.workingData.bandLimitPositionCounts[data.workingData.currentBand][0] + static_cast<int>(static_cast<float>(dds.currentFrequency - data.workingData.bandEdges[data.workingData.currentBand][0]) / data.hertzPerStepperUnitVVC[data.workingData.currentBand]);
        // Power(true, true);
        // stepper.MoveStepperToPositionCorrected(position);
        AutoTuneSWR(data.workingData.currentBand, dds.currentFrequency);
        // Set startUpFlag to true.  This is used to skip this process after one-time use.
        startUpFlag = true;
        return 3; // This will go to default in the state machine, causing a refresh of the main display.
      }
    }
    autotuneLastPushed = autotunebutton.pushed;
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
  startUpFlag = true; // Set this flag on first time use and exit from this function.
  return index;
}
*/

/*****
  Purpose: To get a band menu choice
  Argument list:
    const std::string bands[3].  Example: {"40M", "30M", "20M"}
  Return value:
    int                       the menu selected

  Dependencies:  Adafruit_ILI9341 tft
*****/
int DisplayUtility::SelectBand(const std::string bands[3])
{
  updateMessageTop("       Choose using Menu Encoder");
  EraseBelowMenu(); // Redundant???
  // int currBand[] = {40, 30, 20}; // Used???
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

  // currentBand = currBand[index]; // Used???
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
void DisplayUtility::EraseBelowMenu() // al mod 9-8-19
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
void DisplayUtility::ErasePage()
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
void DisplayUtility::ShowMainDisplay(int whichMenuPage)
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
void DisplayUtility::ShowSubmenuData(float SWR, int currentFrequency) // al mod 9-8-19
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
  if (SWR > 50.0 or SWR < .5)
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
void DisplayUtility::UpdateFrequency(int frequency)
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
void DisplayUtility::UpdateSWR(float SWR, std::string msg)
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
void DisplayUtility::updateMessageTop(std::string messageToPrint)
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
void DisplayUtility::updateMessageBottom(std::string messageToPrint)
{
  tft.fillRect(0, 200, 319, 240, ILI9341_BLACK); // Erase previous message.
  tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
  tft.setTextSize(1);
  tft.setFont(&FreeSerif9pt7b);
  tft.setCursor(10, 220);
  tft.print(messageToPrint.c_str());
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
void DisplayUtility::RestorePreviousPresetChoice(int submenuIndex, int whichBandOption)
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
void DisplayUtility::HighlightNewPresetChoice(int submenuIndex, int whichBandOption)
{
  tft.setTextColor(ILI9341_MAGENTA, ILI9341_WHITE); // HIghlight new preset choice
  tft.setCursor(65, 70 + submenuIndex * 30);
  tft.print(data.workingData.presetFrequencies[whichBandOption][submenuIndex]);
}


/*****
  Purpose: Detect state change of data.maxswitch.  Warn the user.
           Back the stepper off the switch so that it goes back to normal state.

  Parameter list:


  Return value:
    int

  CAUTION:

*****/
int DisplayUtility::DetectMaxSwitch()
{
  if (gpio_get(data.maxswitch) == LOW)
  {
  //  stepper.move(-300);
  //  stepper.runToPosition();
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
  Purpose: This function sets power on or off to all circuits, including the DDS.

  Parameter list:
    bool setpower

  Return value:
    void

  CAUTION:

*****/
void DisplayUtility::PowerStepDdsCirRelay(bool stepperPower, uint32_t frequency, bool circuitPower, bool relayPower)
{
  gpio_put(data.STEPPERSLEEPNOT, stepperPower); //  Deactivating the stepper driver is important to reduce RFI.
                                                // Power down the DDS or set frequency.
                                                // if (dds)
  dds.SendFrequency(frequency);                 // Redundant?
  // else
  //   dds.SendFrequency(0);
  //  Power down RF amplifier and SWR circuits.
  gpio_put(data.OPAMPPOWER, circuitPower);
  gpio_put(data.RFAMPPOWER, circuitPower);
  gpio_put(data.RFRELAYPOWER, relayPower);
  busy_wait_ms(500); //  Wait for relay to switch.
}
