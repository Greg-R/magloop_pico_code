#pragma once
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "StepperManagement.h"
#include "Adafruit_ILI9341.h"

#define XAXISEND                    315
#define XAXISSTART                  25
#define YAXISEND                    210
#define YAXISSTART                  55
#define FREQUENCYENCODERSWITCH      20
#define ANALOGFORWARD               0
#define MAXPOINTSPERSAMPLE          2
#define ANALOGREFLECTED             1
#define SWRREVOFFSET                75

//volatile long frequencyEncoderMovement2;
//volatile long frequencyEncoderMovement;
//long currentFrequency;
//long currPosition;
//float hertzPerStepperUnitVVC[10];
//float readSWRValue;
//volatile int menuEncoderMovement;
//int iMax;
//float tempSWR[10];
//long bandLimitPositionCounts[3][2];  // extern, defined in magloop_pico.cpp
//int tempCurrentPosition[10];
//long  SWRMinPosition;
//int tempCurrentPosition[10];

class SWR {

public:

StepperManagement steppermanage;
Adafruit_ILI9341 tft;
uint32_t currPosition;
//uint32_t bandLimitPositionCounts[100][2];
volatile long frequencyEncoderMovement2;
volatile long frequencyEncoderMovement;
long currentFrequency;
float hertzPerStepperUnitVVC[10];
float readSWRValue;
volatile int menuEncoderMovement;
//int iMax;
float tempSWR[10];
int tempCurrentPosition[10];
long  SWRMinPosition;
int forward_offset, reverse_offset;
float forward_voltage, reverse_voltage;

SWR(StepperManagement & steppermanage, Adafruit_ILI9341 & tft);

void ReadADCoffsets();

void ManualFrequencyControl(int whichBandOption);

void ManualStepperControl();

float ReadSWRValue();

float ReadNewSWRValue();

};