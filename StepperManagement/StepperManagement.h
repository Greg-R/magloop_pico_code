#pragma once
#include <stdint.h>
#include "pico/stdlib.h"
#include "AccelStepper.h"

// This class inherits from AccelStepper, which is an Arduino library.
class StepperManagement : public AccelStepper {

public:

uint32_t currentBand;
uint32_t stepperDirectionOld;
float countPerHertz[100];
uint32_t currPosition;
uint32_t stepperDistanceOld;
uint32_t bandLimitPositionCounts[100][2];
uint32_t moveToStepperIndex;
//AccelStepper stepper;

//StepperManagement(AccelStepper & stepper);
//  This duplicates the parameters of the AccelStepper constructor.
//  The actual parameters when instantiating:  DRIVER, STEPPERPUL, STEPPERDIR
//StepperManagement(uint8_t interface = AccelStepper::DRIVER, uint8_t pin1 = 2, uint8_t pin2 = 3, uint8_t pin3 = 4, uint8_t pin4 = 5, bool enable = true);
StepperManagement(AccelStepper::MotorInterfaceType interface, uint8_t pin1 = 2, uint8_t pin2 = 3, uint8_t pin3 = 4, uint8_t pin4 = 5, bool enable = true);

// This function was in the Encoders file!?
void MoveStepperToPositionCorrected(uint32_t currentPostion); 

void ResetStepperToZero();

void DoFastStepperMove(uint32_t whichBandOption);

long ConvertFrequencyToStepperCount(long presentFrequency);

};