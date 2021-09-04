#pragma once
#include <stdint.h>
#include "pico/stdlib.h"
#include "AccelStepper.h"
#include "SWR.h"
#include "Adafruit_ILI9341.h"
#include "StepperManagement.h"
#include "DisplayManagement.h"


#define FASTMOVESPEED               1000
#define NORMALMOVESPEED             100
#define MAXNUMREADINGS              500
#define PIXELHEIGHT                 240
#define ACCURACYBUTTON         6

//long  SWRMinPosition;
//float tempSWR[10];
//int tempCurrentPosition[10];
//float readSWRValue;
//int iMax;
//uint32_t stepperDistanceOld;
//int stepperDirectionOld;

class DisplayManagement;

class AutoTune {

public:

AccelStepper & stepper;
SWR & swr;
Adafruit_ILI9341 & tft;
StepperManagement & steppermanage;
DisplayManagement & display;
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

//float minSWRAuto;

AutoTune(AccelStepper & stepper, SWR & swr, Adafruit_ILI9341 & tft, StepperManagement & steppermanage, DisplayManagement & display);

void AutoTuneSWR();

void AutoTuneSWRQuick();

void MoveStepperToPositionCorrected(long currentPosition);


};