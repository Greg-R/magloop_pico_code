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

#include "Data.h"

Data::Data() {

};

void Data::computeSlopes() {
countPerHertz[0] =  ((float) workingData.bandLimitPositionCounts[0][1] - (float) workingData.bandLimitPositionCounts[0][0]) / ((float) HIGHEND40M - (float) LOWEND40M);
countPerHertz[1] =  ((float) workingData.bandLimitPositionCounts[1][1] - (float) workingData.bandLimitPositionCounts[1][0]) / ((float) HIGHEND30M - (float) LOWEND30M);
countPerHertz[2] =  ((float) workingData.bandLimitPositionCounts[2][1] - (float) workingData.bandLimitPositionCounts[2][0]) / ((float) HIGHEND20M - (float) LOWEND20M);
hertzPerStepperUnitVVC[0]= ((float) HIGHEND40M - (float) LOWEND40M)/((float) workingData.bandLimitPositionCounts[0][1] - (float) workingData.bandLimitPositionCounts[0][0]);
hertzPerStepperUnitVVC[1]= ((float) HIGHEND30M - (float) LOWEND30M)/((float) workingData.bandLimitPositionCounts[1][1] - (float) workingData.bandLimitPositionCounts[1][0]);
hertzPerStepperUnitVVC[2]= ((float) HIGHEND20M - (float) LOWEND20M)/((float) workingData.bandLimitPositionCounts[2][1] - (float) workingData.bandLimitPositionCounts[2][0]);
}