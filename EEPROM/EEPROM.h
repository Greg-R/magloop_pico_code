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

#define OFFSETTODEFAULTBAND 0
#define OFFSETTOPOSITIONCOUNTS 1 // The start of the stepper positions for the band edges.
#define OFFSETTOPRESETS 7        // 1 default band + 6 stepper positions so offset is 7.
#define LASTBANDUSED 0           // This number is the byte-offset into the EEPROM memory space
#define MAXBANDS 3
#define PRESETSPERBAND 6 // Allow this many preset frequencies on each band
// We're going to erase and reprogram a region 256k from the start of flash.
// Once done, we can access this at XIP_BASE + 256k.
#define FLASH_TARGET_OFFSET (256 * 1024)

class EEPROM
{

public:
    Data &data; // Used to test EEPROM functions.
    union
    {
        uint8_t buffer8[256];
        uint32_t buffer32[64];
        ;
    } bufferUnion;

    uint32_t defaultBand;
    uint32_t countPerHertzArray[3];

    const uint32_t *flash_target_contents = (const uint32_t *)(XIP_BASE + FLASH_TARGET_OFFSET);

    EEPROM(Data &data);

    void initialize();

    void write(const uint8_t *data);

    uint32_t read(uint32_t index);

    void WriteDefaultEEPROMValues();

    void ReadEEPROMValuesToBuffer();

    void WritePositionCounts();

    void ReadPositionCounts();

    void ShowSlopeCoefficients();

    void WriteBandPresets();

    void ReadBandPresets();

    uint32_t ReadCurrentBand();

    void WriteCurrentBand();

    void ShowPositionCounts();

    void ReadEEPROMData();
};