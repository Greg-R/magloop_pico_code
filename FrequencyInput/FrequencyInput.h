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

#pragma once
#include <stdint.h>
#include <string>
#include <utility>
#include <array>
#include <vector>
#include "pico/stdlib.h"
#include "Adafruit_ILI9341.h"
#include "Arduino.h"
#include "DDS.h"
#include "SWR.h"
#include "StepperManagement.h"
#include "EEPROM.h"
#include "GraphPlot.h"
#include "Data.h"
#include "Button.h"
#include "FreeSerif9pt7b.h"
#include "FreeSerif12pt7b.h"
#include "FreeSerif24pt7b.h"

#define PRESETSPERBAND 6
#define PIXELWIDTH 320
#define INCREMENTPAD 22 // Used to display increment cursor
#define MAXMENUES 3     // The menu selections are: Freq, Presets, 1st Cal
#define MAXBANDS 3
#define TARGETMAXSWR 5.5 // Originally set to 2.5, increased for debugging.
#define TEXTLINESPACING 20
#define MAXNUMREADINGS 500
#define PIXELHEIGHT 240

extern int menuEncoderMovement;
extern int frequencyEncoderMovement;
extern int frequencyEncoderMovement2;
extern int digitEncoderMovement;

//  DisplayManagement inherits from class GraphPlot.
class FrequencyInput
{

public:
    Adafruit_ILI9341 &tft;
    EEPROMClass &eeprom;
    Data &data;
    Button &enterbutton;
    Button &autotunebutton;
    Button &exitbutton;
    int whichBandOption;  // This indicates the current band in use.
    float SWRValue;
    float SWRcurrent;
    float readSWRValue;
    int position;
    int menuIndex;
    int submenuIndex;
    volatile int menuEncoderState;
    const int arraySize = 500;
    enum class State
    {
        state0,
        state1,
        state2,
        state3
    }; // Used to move between states in state machines.
    State state;

    FrequencyInput(Adafruit_ILI9341 &tft, EEPROMClass &eeprom, Data &data, Button &enterbutton, Button &autotunebutton, Button &exitbutton);

    long ChangeFrequency(int bandIndex, long frequency);

    void EraseBelowMenu();

    void updateMessageTop(std::string messageToPrint);

    void updateMessageBottom(std::string messageToPrint);

};