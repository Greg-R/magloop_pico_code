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
#include "pico/stdlib.h"
#include "AccelStepper.h"
#include "SWR.h"
#include "Adafruit_ILI9341.h"
#include "StepperManagement.h"

#define FASTMOVESPEED               1000
//#define NORMALMOVESPEED             500  // Was 100, changed to speed up AutoTune.
#define MAXNUMREADINGS              500
const int PIXELHEIGHT        =         240;
#define ACCURACYBUTTON         6

class DisplayManagement;

class AutoTune {

public:

SWR & swr;
Adafruit_ILI9341 & tft;
StepperManagement & steppermanage;
DisplayManagement & display;
int stepperDirectionOld;
uint32_t currPosition;
uint32_t stepperDistanceOld;
int iMax;
float tempSWR[500];
int tempCurrentPosition[500];
long  SWRMinPosition;
float minSWRAuto;
float minSWR;
int whichBandOption;
long  SWRFinalPosition;


AutoTune(SWR & swr, Adafruit_ILI9341 & tft, StepperManagement & steppermanage, DisplayManagement & display);

float AutoTuneSWR();

float AutoTuneSWRQuick();

//void MoveStepperToPositionCorrected(long currentPosition);

};