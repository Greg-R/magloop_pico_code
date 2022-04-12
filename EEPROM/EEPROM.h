#pragma once
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "Data.h"

#define OFFSETTODEFAULTBAND         0
#define OFFSETTOPOSITIONCOUNTS      1     // The start of the stepper positions for the band edges.
#define OFFSETTOPRESETS             7     // 1 default band + 6 stepper positions so offset is 7.
#define LASTBANDUSED                0     // This number is the byte-offset into the EEPROM memory space
#define MAXBANDS                    3  
#define PRESETSPERBAND              6                   // Allow this many preset frequencies on each band
// We're going to erase and reprogram a region 256k from the start of flash.
// Once done, we can access this at XIP_BASE + 256k.
#define FLASH_TARGET_OFFSET (256 * 1024)

class EEPROM {

    public:

    Data & data;  // Used to test EEPROM functions.
    union {
    uint8_t buffer8[256];
    uint32_t buffer32[64];;
    } bufferUnion;

    uint32_t defaultBand;
    uint32_t countPerHertzArray[3];

    const uint32_t *flash_target_contents = (const uint32_t *) (XIP_BASE + FLASH_TARGET_OFFSET);

EEPROM(Data & data);

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