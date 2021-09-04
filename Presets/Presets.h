#pragma once
#include <stdint.h>
#include "pico/stdlib.h"
#include "Adafruit_ILI9341.h"
#include "Arduino.h"
#include "Calibrate.h"
#include "DDS.h"
#include "SWR.h"
#include "AutoTune.h"
#include "AccelStepper.h"
#include "SWR.h"
#include "DisplayManagement.h"

#define PRESETSPERBAND  6 // Allow this many preset frequencies on each band

class Presets {

public:

Adafruit_ILI9341 & tft;
StepperManagement & steppermanage;
AccelStepper & stepper;
DDS & dds;
AutoTune & autotune;
SWR & swr;
DisplayManagement & display;
long presetFrequencies[10][PRESETSPERBAND];
volatile int menuEncoderState;
long currentFrequency;
volatile int menuEncoderMovement;
long currPosition;
long bandLimitPositionCounts[10][2];
long bandEdges[10][2];
float hertzPerStepperUnitVVC[100];

Presets(Adafruit_ILI9341 & tft, StepperManagement & steppermanage, AccelStepper & stepper, DDS & dds, AutoTune & autotune, SWR & swr, DisplayManagement & display);

void ProcessPresets(int whichBandOption, int submenuIndex);

void RestorePreviousPresetChoice(int submenuIndex, int whichBandOption);

void HighlightNewPresetChoice(int submenuIndex, int whichBandOption);

};