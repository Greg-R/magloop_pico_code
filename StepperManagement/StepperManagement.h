#pragma once
#include <stdint.h>
#include "pico/stdlib.h"
#include "AccelStepper.h"
#include "Data.h"

#define FASTMOVESPEED               1000
#define NORMALMOVESPEED             100
#define MAXBUMPCOUNT                2                   // Detent pulses to get "real" bump
#define ZEROSWITCH                  11
#define MAXSWITCH                   10
#define YAXISSTART                  55                  // For graphing purposes
#define YAXISEND                    210
#define XAXISSTART                  25
#define XAXISEND                    315

// This class inherits from AccelStepper, which is an Arduino library.
class StepperManagement : public AccelStepper {

public:

uint32_t currentBand;
uint32_t stepperDirectionOld;
float countPerHertz[3];
float hertzPerStepperUnitAir[3];
uint32_t position;
uint32_t stepperDistanceOld;
uint32_t bandLimitPositionCounts[3][2];
uint32_t moveToStepperIndex;
Data & data;

//StepperManagement(AccelStepper & stepper);
//  This duplicates the parameters of the AccelStepper constructor.
//  The actual parameters when instantiating:  DRIVER, STEPPERPUL, STEPPERDIR
//StepperManagement(uint8_t interface = AccelStepper::DRIVER, uint8_t pin1 = 2, uint8_t pin2 = 3, uint8_t pin3 = 4, uint8_t pin4 = 5, bool enable = true);
StepperManagement(Data & data, AccelStepper::MotorInterfaceType interface, uint8_t pin1 = 2, uint8_t pin2 = 3, uint8_t pin3 = 4, uint8_t pin4 = 5, bool enable = true);

// This function was in the Encoders file!?
void MoveStepperToPositionCorrected(uint32_t position); 

void ResetStepperToZero();

void DoFastStepperMove(uint32_t whichBandOption);

long ConvertFrequencyToStepperCount(long presentFrequency);

};