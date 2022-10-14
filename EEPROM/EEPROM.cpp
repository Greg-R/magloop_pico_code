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

#include "EEPROM.h"

/*****

  The EEPROM memory map for the remote unit is as follows:

    Address                                             Description
      0             Band. On virgin chip, this value is likely 0xFF, in which case Calibrate option is automatically run. Otherwise, 40, 30, oe 20.
                    Keep in mind that an int is 4 bytes on the STM32.
      4  thru 24    These are the stepper position counts for each of the 3 band edges, lowest first (40Mlow, 40Mhigh, 30Mlow, 30Mhigh, 20Mlow,
                    20Mhigh). Variabke holding these is bandLimitPositionCounts[MAXBANDS][2]. These are a long data type
      28 thru 100   These are the stepper positions for the presets, using same order as the band edges (40M, 30M, 20M), each with 6 values.
                    Variable holding these is presetFrequencies[MAXBANDS][PRESETSPERBAND];

  Note that EEPROM is 16-bit oriented and writes can only be on word boundaries.

  From STM32 EEPROM.cpp library file:

  On writes, the possible return values are:
    enum : uint16_t
      {
        EEPROM_OK       = ((uint16_t)0x0000),
        EEPROM_OUT_SIZE     = ((uint16_t)0x0081),
        EEPROM_BAD_ADDRESS    = ((uint16_t)0x0082),
        EEPROM_BAD_FLASH    = ((uint16_t)0x0083),
        EEPROM_NOT_INIT     = ((uint16_t)0x0084),
        EEPROM_SAME_VALUE   = ((uint16_t)0x0085),
        EEPROM_NO_VALID_PAGE  = ((uint16_t)0x00AB)
      };

  On reads, the possible return values are:

        - EEPROM_OK: if variable was found
        - EEPROM_BAD_ADDRESS: if the variable was not found
        - EEPROM_NO_VALID_PAGE: if no valid page was found.

*****/

EEPROM::EEPROM(Data &data) : data(data)
{
  initialize();
}

//  Wrap the Pi Pico SDK write function.
//  Note that this function must write in 256 byte chunks.
//  The count parameter is the number of 256 byte chunks to be written.
//  The data should be an array with multiples of 256 elements.
void EEPROM::write(const uint8_t *data)
{
  uint32_t ints = save_and_disable_interrupts();
  flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
  flash_range_program(FLASH_TARGET_OFFSET, data, FLASH_PAGE_SIZE);
  restore_interrupts(ints);
}

//  Wrap the Pi Pico SDK read function.  This reads a single uint32_t value.
uint32_t EEPROM::read(uint32_t index)
{

  return flash_target_contents[index]; //  Return the value at the address.
}

/*****
  Purpose: The Pi Pico does not actually have any EEPROM, so we have to fake it with flash memory.
           This method initializes a 256 element array which is used to temporarily store
           the default band, preset frequencies, and position counts.
           A 32 bit elements are used, although 16 bits is used in the original STM32 project.
           The entire buffer is written to flash when using the write method.
           This is because you must write a single "page" to Flash.
           There is a Union which was both 8 and 32 bit buffers which overlap.
           The 32 bit buffer is manipulated by the methods, but the 8 bit buffer
           is used to write to flash.

           The default values should be read only once prior to initial calibration.
           After that, the WriteDefaultEEPROMValues() should be commented out.

  Parameter list:
    void

  Return value:
    void
*****/
void EEPROM::initialize()
{
  for (int i = 0; i < 64; i = i + 1)
    bufferUnion.buffer32[i] = 0x00000000;
  //  These overwrite the first 4 values for testing purposes.
  // bufferUnion.buffer32[0] = 0x10000000;
  // bufferUnion.buffer32[1] = 0x00000002;
  // bufferUnion.buffer32[2] = 0x00000003;
  // bufferUnion.buffer32[3] = 0x00000004;
}

/*****
  Purpose: To write default values to Flash memory.
  This takes the defaults which are hand-typed into magloop_pico.cpp
  and writes them to Flash memory.
  The position counts are estimates only; the calibration routine
  will over-write them with the values found by the algorithm.
  The method should be run once, and then commented out.
  Parameter list:
    void

  Return value:
    void

void EEPROM::WriteDefaultEEPROMValues()
{
  // int i, j, k, status;
  int index;

  //  Write the default band to the 0th element:
  bufferUnion.buffer32[OFFSETTODEFAULTBAND] = 40;

  //  Write the band limit positions to the buffer.
  //  There are 3 bands, and an upper and lower limit for each band.
  //  So there are a total of 6 limits.
  index = OFFSETTOPOSITIONCOUNTS;
  for (uint8_t i = 0; i < MAXBANDS; i++)
  { // This increments the row.
    for (uint8_t k = 0; k < 2; k++)
    { //  Upper and lower limits
      bufferUnion.buffer32[index] = data.bandLimitPositionCounts[i][k];
      index = index + 1;
    }
  }
  index = OFFSETTOPRESETS;
  //  Write the presets to the array:
  for (uint8_t i = 0; i < MAXBANDS; i++)
  {
    for (uint8_t k = 0; k < PRESETSPERBAND; k++)
    {
      bufferUnion.buffer32[index] = data.presetFrequencies[i][k];
      index = index + 1;
    }
  }
  write(bufferUnion.buffer8); // Writing to Flash must be done with a uint8_t array.
}
*/

/*****
  Purpose: Show EEPROM values, mainly a debugging tool
  Load the values into a different buffer viewbuffer[] array.
  Parameter list:
    void

  Return value:
    void
*****/
void EEPROM::ReadEEPROMValuesToBuffer()
{
  int index;
  //  First, make sure the buffer is initialized to zeros.
  initialize();

  //  Default band goes in index 0 of the buffer.
  bufferUnion.buffer32[0] = read(OFFSETTODEFAULTBAND);
  index = OFFSETTOPOSITIONCOUNTS;
  for (int i = 0; i < MAXBANDS; i++)
  {
    for (int k = 0; k < 2; k++)
    {
      bufferUnion.buffer32[index] = read(index);
      index = index + 1;
    }
  }
  index = OFFSETTOPRESETS;
  for (int i = 0; i < MAXBANDS; i++)
  {
    for (int k = 0; k < PRESETSPERBAND; k++)
    {
      bufferUnion.buffer32[index] = data.presetFrequencies[i][k];
      index = index + 1;
    }
  }
}

/*****
  Purpose: Save band edges position counts
  This writes to the buffer only.
  To save to flash, use the write function.
  Parameter list:
    void

  Return value:
    void
*****/
void EEPROM::WritePositionCounts()
{
  uint32_t index = OFFSETTOPOSITIONCOUNTS;

  for (int i = 0; i < MAXBANDS; i++)
  {
    for (int k = 0; k < 2; k++)
    {
      bufferUnion.buffer32[index] = data.bandLimitPositionCounts[i][k];
      index = index + 1;
    }
  }
}

/*****
  Purpose: Read configuration data from EEPROM
  Read the position counts and overwrite the default values.
  Calculate countPerHertz and hertzPerStepperUnitAir arrays.
  Parameter list:
    void

  Return value:
    void
*****/
void EEPROM::ReadPositionCounts()
{
  uint16_t index;
  index = OFFSETTOPOSITIONCOUNTS;
  for (int i = 0; i < MAXBANDS; i++)
  {
    for (int k = 0; k < 2; k++)
    {
      data.bandLimitPositionCounts[i][k] = read(index);
      index = index + 1;
    }
  }
}

/*****
  Purpose: Show slope coefficients for frequency calcs
  The array is stored in the EEPROM object as countPerHertzArray.
  Parameter list:
    void

  Return value:
    void
*****/
void EEPROM::ShowSlopeCoefficients()
{

  for (int i = 0; i < 3; i++)
  {
    countPerHertzArray[i] = data.countPerHertz[i];
  }
}

/*****
  Purpose: Write the presets for each band
  This writes to the buffer only.
  To save to flash, use the write function.
  Parameter list:
    void

  Return value:
    void
*****/

void EEPROM::WriteBandPresets()
{
  uint16_t index;
  index = OFFSETTOPRESETS;
  for (int i = 0; i < MAXBANDS; i++)
  {
    for (int k = 0; k < PRESETSPERBAND; k++)
    {
      bufferUnion.buffer32[index] = data.presetFrequencies[i][k];
      index = index + 1;
    }
  }
}

/*****
  Purpose: Read the presets for each band from EEPROM

  Parameter list:
  void

  Return value:
  void
*****/
void EEPROM::ReadBandPresets()
{
  uint16_t index;

  index = OFFSETTOPRESETS; // Starting EEPROM address for Bnd presets

  for (int i = 0; i < MAXBANDS; i++)
  {
    for (int k = 0; k < PRESETSPERBAND; k++)
    {
      bufferUnion.buffer32[index] = read(index);
      index = index + 1;
    }
  }
}

/*****
  Purpose: Read the value for currentBand from EEPROM.  Write to the data object.
           This function looks to see if the currentBand is either 40, 30, or 20.
           If it is not, then it is assumed this is a first-time usage.
           The currentBand is set to 40.
  Parameter list:
  void

  Return value:
  void
*****/
uint32_t EEPROM::ReadCurrentBand()
{
  // Set the band to 40M for first-time usage.
 // if((read(0) != 40) & (read(0) != 30) & (read(0) != 20)) 
 // else data.currentBand = read(0);  // Need to "dereference" here?
//  this->data.currentBand = read(0);
  return read(0);
}

/*****
  Purpose: Read the value for currentBand from EEPROM.  Write to the data object.
           This function looks to see if the currentBand is either 40, 30, or 20.
           If it is not, then it is assumed this is a first-time usage.
           The currentBand is set to 40.
  Parameter list:
  void

  Return value:
  void
*****/
uint32_t EEPROM::ReadCurrentFrequency()
{
  return read(25);
}

/*****
  Purpose: Write the value for currentBand to EEPROM
  This writes to the buffer only.
  The functionality of this is not currently implemented.  Will default to 40M for now.
  To save to flash, use the write function.
  Parameter list:
  void

  Return value:
  void
*****/

void EEPROM::WriteCurrentFrequency(uint32_t frequency)
{
  bufferUnion.buffer32[OFFSETTOFREQUENCY] = frequency;
}

/*****
  Purpose: Write the value for currentBand to EEPROM
  This writes to the buffer only.
  The functionality of this is not currently implemented.  Will default to 40M for now.
  To save to flash, use the write function.
  Parameter list:
  void

  Return value:
  void
*****/

void EEPROM::WriteCurrentBand(uint32_t band)
{

  //  bufferUnion.buffer32[OFFSETTODEFAULTBAND] = read(OFFSETTODEFAULTBAND);
  bufferUnion.buffer32[OFFSETTODEFAULTBAND] = band;
}

/*****
  Purpose: Reads all of the values stored in EEPROM

  Parameter list:
    void

  Return value:
    void
*****/
void EEPROM::ReadEEPROMData()
{
  ReadCurrentBand();    // Get last band used
  ReadPositionCounts(); // Get band edge positions
  ReadBandPresets();    // Get preset frequencies
}
