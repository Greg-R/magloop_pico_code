#pragma once
#include <stdint.h>
#include "pico/stdlib.h"
#include "Adafruit_ILI9341.h"
#include "Arduino.h"
#include "Calibrate.h"
#include "DDS.h"
#include "SWR.h"
#include "AutoTune.h"
#include "Presets.h"
#include "DisplayManagement.h"
#include "Calibrate.h"

#define FREQMENU  0  // Menuing indexes

class DisplayManagement;

class Buttons {

    public:

    int whichBandOption;
    int submenuIndex;
    int menuIndex;
    long currentFrequency;
    int quickCalFlag;

    DisplayManagement & display;
    Presets & presets;
    DDS & dds;
    Calibrate & calibrate;

    Buttons(DisplayManagement & display, Presets & presets, DDS & dds, Calibrate & calibrate);

void executeButton1();

void executeButton3();

void quickCalISR();

};