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
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "Data.h"

//#define OFFSETTODEFAULTBAND 0
//#define OFFSETTOPOSITIONCOUNTS 1 // The start of the stepper positions for the band edges.
//#define OFFSETTOPRESETS 7        // 1 default band + 6 stepper positions so offset is 7.
//#define LASTBANDUSED 0           // This number is the byte-offset into the EEPROM memory space
//#define MAXBANDS 3
//#define PRESETSPERBAND 6 // Allow this many preset frequencies on each band
// We're going to erase and reprogram a region 256k from the start of flash.
// Once done, we can access this at XIP_BASE + 256k.
//#define FLASH_TARGET_OFFSET (256 * 1024)

class EEPROM
{

public:
    Data &data; // Used to test EEPROM functions.
    //  The union is used so that byte operations can be used.
    //  The useful data is in the uint32_t array.
    union
    {
        uint8_t buffer8[256];
        uint32_t buffer32[64];
        ;
    } bufferUnion;

    // This struct holds the useful data after being read from the FLASH.
    // After this struct is read from FLASH, the information is used to update the Data object.
    struct dataStruct {
           uint32_t band = 0;
           uint32_t frequency = 0;
           uint32_t endPositions[3][2];
           uint32_t presets[3][6];
           uint32_t initialized = 0x55555555;
    };

    uint32_t defaultBand;
    uint32_t countPerHertzArray[3];

    const uint32_t OFFSETTODEFAULTBAND = 0;
    const uint32_t OFFSETTOPOSITIONCOUNTS = 1;
    const uint32_t OFFSETTOPRESETS = 7;  // Presets has 3 bands times 6 frequencies for 18 elements.
    const uint32_t OFFSETTOFREQUENCY = 25;
    const uint32_t LASTBANDUSED = 0;
    const uint32_t MAXBANDS = 3;
    const uint32_t PRESETSPERBAND = 6;
    const uint32_t FLASH_TARGET_OFFSET = 262144;  // (256 * 1024)
    // Pointer to the FLASH memory.
    dataStruct *eepromData = (dataStruct *)(XIP_BASE + FLASH_TARGET_OFFSET);

    EEPROM(Data &data);

    void initialize();

    void write();

    void read();

    //void WriteDefaultEEPROMValues();

    void ReadEEPROMValuesToBuffer();
    void ReadEEPROMValuesToBuffer2();

    void WritePositionCounts();

    void ReadPositionCounts();

    void ShowSlopeCoefficients();

    void WriteBandPresets();

    void ReadBandPresets();

    uint32_t ReadCurrentBand();

    uint32_t ReadCurrentFrequency();

    void WriteCurrentBand(uint32_t band);

    void WriteCurrentFrequency(uint32_t frequency);

    void ShowPositionCounts();

    void ReadEEPROMData();
};