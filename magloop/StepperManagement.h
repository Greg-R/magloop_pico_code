#include <stdint.h>
#include "pico/stdlib.h"
#include "AccelStepper.h"


class StepperManagement {

public:

StepperManagement(AccelStepper & stepper) : stepper(stepper) {}

AccelStepper stepper;
// This function was in the Encoders file!?
void MoveStepperToPositionCorrected(uint32_t currentPostion); 

void ResetStepperToZero();

void DoFastStepperMove(uint32_t whichBandOption);

long ConvertFrequencyToStepperCount(long presentFrequency);

};