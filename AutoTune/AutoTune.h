#pragma once
#include <stdint.h>
#include "pico/stdlib.h"
#include "AccelStepper.h"
#include "SWR.h"
#include "Adafruit_ILI9341.h"
#include "StepperManagement.h"
//#include "DisplayManagement.h"

#define FASTMOVESPEED               1000
#define NORMALMOVESPEED             100
#define MAXNUMREADINGS              500
#define PIXELHEIGHT                 240
#define ACCURACYBUTTON         6

class DisplayManagement;

class AutoTune {

public:

SWR & swr;
Adafruit_ILI9341 & tft;
StepperManagement & steppermanage;
//DisplayManagement & display;
int stepperDirectionOld;
uint32_t currPosition;
uint32_t stepperDistanceOld;
int iMax;
float tempSWR[500];
int tempCurrentPosition[500];
long  SWRMinPosition;
float minSWRAuto;
float minSWR;
int whichBandOption;
long  SWRFinalPosition;


AutoTune(SWR & swr, Adafruit_ILI9341 & tft, StepperManagement & steppermanage);

void AutoTuneSWR();

void AutoTuneSWRQuick();

void MoveStepperToPositionCorrected(long currentPosition);

};