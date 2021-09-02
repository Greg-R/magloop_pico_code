#pragma once
#include <stdint.h>
#include "pico/stdlib.h"
#include "DisplayManagement.h"
#include "AccelStepper.h"
#include "../magloop/StepperManagement.h"
#include "../Adafruit_ILI9341/Adafruit_ILI9341.h"
#include "Arduino.h"
#include "DDS.h"
#include "SWR.h"
#include "AutoTune.h"

#define MAXBANDS   3 
#define MAXSWITCH 10
#define TARGETMAXSWR 2.5
#define TEXTLINESPACING 20
#define MENUENCODERSWITCH 25

class DisplayManagement;

class Calibrate {
public:

DisplayManagement & display;
AccelStepper & stepper;
StepperManagement & steppermanage;
Adafruit_ILI9341 & tft;
DDS & dds;
SWR & swr;
AutoTune & autotune;
long SWRMinPosition;

long currPosition;
long currentFrequency;
long bandEdges[10][2];
long bandLimitPositionCounts[10][10];
int quickCalFlag;
//float SWR;

Calibrate(DisplayManagement & display, AccelStepper & stepper, StepperManagement & steppermanage, Adafruit_ILI9341 & tft, DDS & dds, SWR & swr, AutoTune & autotune);

void DoNewCalibrate2();

void DoFirstCalibrate();

void CalSWR();

void DoSingleBandCalibrate(int whichBandOption);

};