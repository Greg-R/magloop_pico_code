#pragma once
#include <stdint.h>
#include "pico/stdlib.h"
#include "DisplayManagement.h"
#include "AccelStepper.h"
#include "../magloop/StepperManagement.h"
#include "../Adafruit_ILI9341/Adafruit_ILI9341.h"
#include "Arduino.h"
#include "DDS.h"

#define MAXBANDS                    3 

class Calibrate {
public:

DisplayManagement & display;
AccelStepper & stepper;
StepperManagement & steppermanage;
Adafruit_ILI9341 & tft;
Calibrate & calibrate;
DDS & dds;

long currPosition;
long currentFrequency;
long bandEdges[10][2];
long bandLimitPositionCounts[10][10];

Calibrate(DisplayManagement & display, AccelStepper & stepper, StepperManagement & steppermanage, Adafruit_ILI9341 & tft, Calibrate & calibrate, DDS & dds);

void DoNewCalibrate2();

void DoFirstCalibrate();

void CalSWR();

void DoSingleBandCalibrate(int whichBandOption);

};