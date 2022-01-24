#pragma once
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/flash.h"

#define OFFSETTOPOSITIONCOUNTS      sizeof(int)         // The start of the stepper positions for the band edges defined above
#define OFFSETTOPRESETS             sizeof(int) * 6     // There are 8 bands edges
#define LASTBANDUSED                0                   // This number is the byte-offset into the EEPROM memory space
#define MAXBANDS                    3  
#define PRESETSPERBAND              6                   // Allow this many preset frequencies on each band
// We're going to erase and reprogram a region 256k from the start of flash.
// Once done, we can access this at XIP_BASE + 256k.
#define FLASH_TARGET_OFFSET (256 * 1024)
#define EEPROM_OK 1
#define FLASH_COMPLETE 1

#define LOWEND40M                   7000000L            // Define these frequencies for your licensing authority
#define HIGHEND40M                  7300000L            // The 'L' helps document that these are long data types
#define LOWEND30M                  10100000L
#define HIGHEND30M                 10150000L
#define LOWEND20M                  14000000L
#define HIGHEND20M                 14350000L

extern int currentBand;
extern long bandLimitPositionCounts[][2];
extern float countPerHertz[];
long presetFrequencies[3][PRESETSPERBAND];
extern float hertzPerStepperUnitAir[];
int PageBase0 = 0x801f000;     // EEPROM base address. Everything indexed from this address
int PageSize  = 0x400;         // 1024 bytes of EEPROM

class EEPROM {

    public:

    union {
    uint8_t bytes[4];
    uint32_t val;
    } myUnion;

    const uint8_t *flash_target_contents = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);

EEPROM();

void DefineEEPROMPage();

void write(uint32_t flash_offs, const uint8_t *data, size_t count);

uint8_t read(uint8_t * offset);

void WriteDefaultEEPROMValues();

void ShowEEPROMValues();

void WritePositionCounts();

void ReadPositionCounts();

void ShowSlopeCoefficients();

int WriteBandPresets();

void ReadBandPresets();

void ReadCurrentBand();

void WriteCurrentBand();

void ShowPositionCounts();

void ReadEEPROMData();

};