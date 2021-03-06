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

//  This class is intended to manage various frequency and position related constants and variables.
//  The single object will be referenced by most or maybe all of the other class objects.

class Data
{

public:
  // Set these frequencies for your band limits and desired presets.
  /*
  std::string bands[3] = {"30M", "20M", "15M"};
  const uint32_t LOWEND40M =  10100000;
  const uint32_t HIGHEND40M = 10150000;
  const uint32_t LOWEND30M =  14000000;
  const uint32_t HIGHEND30M = 14350000;
  const uint32_t LOWEND20M =  21000000;
  const uint32_t HIGHEND20M = 21450000;
  const uint32_t presetFrequencies[3][6] =
  {
    {10106000L, 10116000L, 10120000L, 10130000L, 10140000L, 10145000L},   // 30M
    {14030000L, 14060000L, 14100000L, 14200000L, 14250000L, 14285000L},   // 20M
    {21030000L, 21040000L, 21100000L, 21150000L, 21250000L, 21285000L},   // 15M
  };
  */
  // Bands used:

  std::string bands[3] = {"40M", "30M", "20M"};
  const uint32_t LOWEND40M = 7000000;
  const uint32_t HIGHEND40M = 7300000;
  const uint32_t LOWEND30M = 10100000;
  const uint32_t HIGHEND30M = 10150000;
  const uint32_t LOWEND20M = 14000000;
  const uint32_t HIGHEND20M = 14350000;
  const uint32_t presetFrequencies[3][6] =
      {
          {7030000L, 7040000L, 7100000L, 7150000L, 7250000L, 7285000L},       // 40M
          {10106000L, 10116000L, 10120000L, 10130000L, 10140000L, 10145000L}, // 30M
          {14030000L, 14060000L, 14100000L, 14200000L, 14250000L, 14285000L}  // 20M
  };

  long bandLimitPositionCounts[3][2];

  const uint32_t bandEdges[3][2] = { // Band edges in Hz
      {LOWEND40M, HIGHEND40M},
      {LOWEND30M, HIGHEND30M},
      {LOWEND20M, HIGHEND20M}};

  //  This should be made variable length arrays.
  float countPerHertz[3];
  float hertzPerStepperUnitVVC[3]; // Voltage Variable Cap
  int currentBand;

  const int STEPPERSLEEPNOT = 9;
  const int OPAMPPOWER = 3;
  const int RFAMPPOWER = 2;
  const int RFRELAYPOWER = 19;

  Data();

  void computeSlopes();
};