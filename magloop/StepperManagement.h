#pragma once
#include <stdint.h>
#include "pico/stdlib.h"
#include "AccelStepper.h"

//uint32_t currPosition;
//uint32_t stepperDistanceOld;
//uint32_t bandLimitPositionCounts[100][2];
//uint32_t moveToStepperIndex;
//uint32_t currentBand;
//uint32_t stepperDirectionOld;
//float countPerHertz[100];


class StepperManagement {

public:

uint32_t currentBand;
uint32_t stepperDirectionOld;
float countPerHertz[100];
uint32_t currPosition;
uint32_t stepperDistanceOld;
uint32_t bandLimitPositionCounts[100][2];
uint32_t moveToStepperIndex;

StepperManagement(AccelStepper & stepper);

AccelStepper stepper;
// This function was in the Encoders file!?
void MoveStepperToPositionCorrected(uint32_t currentPostion); 

void ResetStepperToZero();

void DoFastStepperMove(uint32_t whichBandOption);

long ConvertFrequencyToStepperCount(long presentFrequency);

};