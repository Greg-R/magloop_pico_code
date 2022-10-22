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

#include "FrequencyInput.h"

FrequencyInput::FrequencyInput(Adafruit_ILI9341 &tft,
                               EEPROMClass &eeprom, Data &data, Button &enterbutton, Button &autotunebutton, Button &exitbutton)
                                : tft(tft), eeprom(eeprom), data(data), enterbutton(enterbutton), autotunebutton(autotunebutton), exitbutton(exitbutton)
{
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
long FrequencyInput::ChangeFrequency(int bandIndex, long frequency) // Al Mod 9-8-19
{
  int i, changeDigit, digitSpacing, halfScreen, incrementPad, insetMargin, insetPad;
  long defaultIncrement;
  insetPad = 57; // Used to align digit indicator
  incrementPad = 05;
  digitSpacing = 10;
  insetMargin = 20;
  defaultIncrement = 1000L;
  halfScreen = PIXELHEIGHT / 2 - 25;
  bool lastexitbutton = true;
  bool lastenterbutton = true;
  updateMessageTop("          Enter New Preset Frequency");
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
  //tft.print("Tune:");
  tft.setCursor(insetMargin + 90, halfScreen + 100);
  //tft.print("AutoTune Button");
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

  // State Machine for frequency input with encoders.
  while (true)
  { // Update frequency until user pushes AutoTune button.
    // Poll autotunebutton and exitbutton.
    enterbutton.buttonPushed();
    exitbutton.buttonPushed();
    if (enterbutton.pushed & not lastenterbutton) {
      lastenterbutton = enterbutton.pushed;
      break;  // Break out of the while loop.
    }
    // Make sure there is a proper transition of the enter button.
      lastenterbutton = enterbutton.pushed;
    //  Exit this menu, but make sure it is a proper edge transition:
    if (exitbutton.pushed & not lastexitbutton)
    {
      frequency = 0;
      return frequency;
    }
    lastexitbutton = exitbutton.pushed;
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
     // position = stepper.ConvertFrequencyToStepperCount(frequency);
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
int FrequencyInput::MakeMenuSelection(int index) // Al Mod 9-8-19
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
  Purpose: To erase the display below the top two menu lines
  Argument list:
    void
  Return value:
    void
*****/
void FrequencyInput::EraseBelowMenu() // al mod 9-8-19
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
void FrequencyInput::ErasePage()
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
void FrequencyInput::ShowMainDisplay(int whichMenuPage)
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
  Purpose: Update Top Message Area

  Argument list:
    char message

  Return value:
    void
*****/
void FrequencyInput::updateMessageTop(std::string messageToPrint)
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
void FrequencyInput::updateMessageBottom(std::string messageToPrint)
{
  tft.fillRect(0, 200, 319, 240, ILI9341_BLACK); // Erase previous message.
  tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
  tft.setTextSize(1);
  tft.setFont(&FreeSerif9pt7b);
  tft.setCursor(10, 220);
  tft.print(messageToPrint.c_str());
}


int FrequencyInput::SelectPreset()
{
  int frequency;
  bool lastexitbutton = true;
  bool lastenterbutton = true;
  bool lastautotunebutton = true;
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
  menuEncoderState = 0;
  //  Preset state selection machine
  while (true)
  { // Why 2 buttons???
    // Poll 3 buttons:
    autotunebutton.buttonPushed();
    enterbutton.buttonPushed();  // Use the Enter button to change and save the Preset frequency.
    exitbutton.buttonPushed();
    if (exitbutton.pushed & not lastexitbutton)
      return frequency = 0; // Exit Preset Select if requested by user.
      lastexitbutton = exitbutton.pushed;
    // Use the Enter button to change and save the Preset frequency.
    // A FrequencyInput object is used for this task.
    if (enterbutton.pushed & not lastenterbutton)

      lastenterbutton = enterbutton.pushed;

    if (autotunebutton.pushed & not lastautotunebutton)
      break; // Exit preset select and AutoTune.
      lastautotunebutton = autotunebutton.pushed;
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
  }                                                                  // end while Preset state selection machine
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
void FrequencyInput::RestorePreviousPresetChoice(int submenuIndex, int whichBandOption)
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
void FrequencyInput::HighlightNewPresetChoice(int submenuIndex, int whichBandOption)
{
  tft.setTextColor(ILI9341_MAGENTA, ILI9341_WHITE); // HIghlight new preset choice
  tft.setCursor(65, 70 + submenuIndex * 30);
  tft.print(data.workingData.presetFrequencies[whichBandOption][submenuIndex]);
}
