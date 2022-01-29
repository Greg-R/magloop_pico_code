
#include "StepperManagement.h"


//StepperManagement::StepperManagement(AccelStepper::MotorInterfaceType interface, uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, bool enable): stepper(interface, pin1, pin2, pin3, pin4, enable) {
//currPosition = 2500;  // Default to approximately midrange.
//setCurrentPosition(5000);  //
//}

StepperManagement::StepperManagement(AccelStepper::MotorInterfaceType interface, uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, bool enable):AccelStepper(interface, pin1, pin2) {
position = 2500;  // Default to approximately midrange.
setCurrentPosition(5000);  //
}


// Revised version which is simplified to use function from AccelStepper.
// This uses a blocking function.  However, I can't think of a reason it won't work here.
void StepperManagement::MoveStepperToPositionCorrected(uint32_t position) {
moveTo(position);
runToPosition();
}

/*  Original function.  Did not understand why this function was required.???
// This function was in the Encoders file???
void StepperManagement::MoveStepperToPositionCorrected(uint32_t position) {
  int stepperDirection;
  long stepperDistance;
  while (1)
  {
    stepperDistanceOld = stepperDistance;
    moveTo(position);
    run();
    stepperDistance = distanceToGo();
    if (distanceToGo() == 0)
    {
      if (stepperDistanceOld >= 0)
      {
        stepperDirection = 1;
      }
      else
      {
        if (stepperDistanceOld < 0)
        {
          stepperDirection = -1;
        }
      }
      break;
    }
  }
  if (stepperDirection != stepperDirectionOld)
  {
  //  stepper.setCurrentPosition(currentPosition - 1);
              setCurrentPosition(position - 1);
  }
  stepperDirectionOld = stepperDirection;
}


/*****
  Purpose: Resets stepper motor to 0 position

  Parameter list:
    void
  Return value:
    void

  CAUTION:
*****/
void StepperManagement::ResetStepperToZero()
{
  //updateMessage("Resetting to Zero");
  setCurrentPosition(0);
  setMaxSpeed(500);
  setAcceleration(110);
  position = 0;  // Assume the default position is zero.
  // This loop moves the stepper in a negative direction until
  // the zero switch closes.
  while (gpio_get(ZEROSWITCH) != 0) {      // move to zero position 
    moveTo(position);
    run();
    position--;  // Rotate towards zero stop switch.
  }
  // Reset to zero:
  setCurrentPosition(0);
  setMaxSpeed(500);  // Note: setCurrentPosition sets max speed to 0.
  // Now bump the stepper off the zero limit switch enough to open the switch.
  position = 0;
  //MoveStepperToPositionCorrected(position); //Al 4-20-20
  while (true) {
    position++;    // Rotate away from zero stop switch.
    move(position);
    run();  
    if (gpio_get(ZEROSWITCH) == 1)  // When the zero stop switch opens, break from loop.
      break;
  }
  // Bump it up just a little more for margin.
  runToNewPosition(position + 20);
}


/*****
  Purpose: To move the capacitor to the approximate location via the stepper motor

  Parameter list:
  int whichBandOption           the band selected (probably don't need since currentFrequency tells us

  Return value:
    void

  CAUTION:

  int hertzPerStepperUnitVVC[] = {909,              // 40M 909Hz move per stepper revolution with VVC
                                761,                // 30M
                                614                 // 20M
                          };

*****/
void StepperManagement::DoFastStepperMove(uint32_t whichBandOption)
{
  int yAxisPixelPerUnit, totalPixels, yDotIncrement;
  int startOffset;
  float pixelsPerTenth, swr;
  //stepper.setMaxSpeed(FASTMOVESPEED);       // Get ready for a fast move
            setMaxSpeed(FASTMOVESPEED);
  //stepper.setAcceleration(1100);
  setAcceleration(1100);
  startOffset = 0;
  moveToStepperIndex = bandLimitPositionCounts[whichBandOption][0];
#ifdef DEBUG1
  Serial.print("currentFrequency = ");
  Serial.print(currentFrequency);
  Serial.print("   moveToStepperIndex = ");
  Serial.println(moveToStepperIndex);
  Serial.print("   bandLimitPositionCounts = ");
  Serial.println(bandLimitPositionCounts[whichBandOption][0]);
  Serial.print("   bandEdges[whichBandOption][0] = ");
  Serial.println(bandEdges[whichBandOption][0]);
  Serial.print("   hertzPerStepperUnitAir[whichBandOption] = ");
  Serial.println(hertzPerStepperUnitAir[whichBandOption]);
#endif
  while (1) {
    //stepper.moveTo(moveToStepperIndex);
              moveTo(moveToStepperIndex);
    //stepper.run();
              run();
    //if (stepper.distanceToGo() == 0) {c
      if (        distanceToGo() == 0) {
      break;
    }
  }

#ifdef DEBUG1
  Serial.print("Move complete ");
  Serial.println(swr);
#endif

  totalPixels = YAXISEND - YAXISSTART;
  pixelsPerTenth = totalPixels / 50.0;      // HIghest SWR is 5, so 50 tenths.
  //stepper.setMaxSpeed(NORMALMOVESPEED);
            setMaxSpeed(NORMALMOVESPEED);
}


/*****
  Purpose: Allow the user to change frequency and have the stepper automatically follow frequency change

  Parameter list:
    long presentFrequency     the present frequency of the DDS

  Return value:
    void

  CAUTION:

*****/
long StepperManagement::ConvertFrequencyToStepperCount(long presentFrequency)
{
  long count;
  switch (currentBand) {
    case 40:       //   intercept                  + slopeCoefficient * newFrequency
      count = bandLimitPositionCounts[0][0] + (long) (countPerHertz[0] * ((float) (presentFrequency - LOWEND40M)));
      break;

    case 30:
      count = bandLimitPositionCounts[1][0] + (long) (countPerHertz[1] * ((float) (presentFrequency - LOWEND30M)));
      break;

    case 20:
      count = bandLimitPositionCounts[2][0] + (long) (countPerHertz[2] * ((float) (presentFrequency - LOWEND20M)));
      //Serial.print("       count = ");
      //Serial.println(count);
      break;

    default:
      break;
  }
  position = count;
  return count;
}


