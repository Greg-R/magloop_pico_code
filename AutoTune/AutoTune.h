#pragma once
#include <stdint.h>
#include "pico/stdlib.h"
#include "../AccelStepper/AccelStepper.h"
#include "../SWR/SWR.h"
#include "../Adafruit_ILI9341/Adafruit_ILI9341.h"
#include "../magloop/StepperManagement.h"


#define FASTMOVESPEED               1000
#define NORMALMOVESPEED             100
#define MAXNUMREADINGS              500
#define PIXELHEIGHT                 240
#define ACCURACYBUTTON         6

//long  SWRMinPosition;
float minSWRAuto;
float minSWR;
//float tempSWR[10];
//int tempCurrentPosition[10];
//float readSWRValue;
int whichBandOption;
long  SWRFinalPosition;
//int iMax;
//uint32_t stepperDistanceOld;
//int stepperDirectionOld;



class AutoTune {

public:

AccelStepper stepper;
SWR swr;
Adafruit_ILI9341 tft;
StepperManagement steppermanage;
int stepperDirectionOld;
uint32_t currPosition;
uint32_t stepperDistanceOld;
int iMax;
float tempSWR[500];
int tempCurrentPosition[500];
long  SWRMinPosition;

//float minSWRAuto;

AutoTune(AccelStepper stepper, SWR swr, Adafruit_ILI9341 tft, StepperManagement steppermanage);

void AutoTuneSWR();

void AutoTuneSWRQuick();

void MoveStepperToPositionCorrected(long currentPosition);


};