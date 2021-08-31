//#ifndef BEENHERE
//#include "MagLoop.h"
//#endif

#include "StepperManagement.h"

#define FASTMOVESPEED               1000
#define NORMALMOVESPEED             100
#define MAXBUMPCOUNT                2                   // Detent pulses to get "real" bump
#define ZEROSWITCH                  10
#define MAXSWITCH                   11
#define YAXISSTART                  55                  // For graphing purposes
#define YAXISEND                    210
#define XAXISSTART                  25
#define XAXISEND                    315
#define LOWEND40M                   7000000L            // Define these frequencies for your licensing authority
#define HIGHEND40M                  7300000L            // The 'L' helps document that these are long data types
#define LOWEND30M                  10100000L
#define HIGHEND30M                 10150000L
#define LOWEND20M                  14000000L
#define HIGHEND20M                 14350000L




StepperManagement::StepperManagement(AccelStepper & stepper): stepper(stepper) {
//  stepper = stepper;
}

//AccelStepper& stepper;
// This function was in the Encoders file!?
void StepperManagement::MoveStepperToPositionCorrected(uint32_t currentPosition) {
  int stepperDirection;
  long stepperDistance;
  while (1)
  {
    stepperDistanceOld = stepperDistance;
    stepper.moveTo(currentPosition);
    stepper.run();
    stepperDistance = stepper.distanceToGo();
    if (stepper.distanceToGo() == 0)
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
    stepper.setCurrentPosition(currentPosition - 1);
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
  stepper.setMaxSpeed(10000);
  stepper.setAcceleration(1100);
  while (gpio_get(ZEROSWITCH) != 0) {      // move to zero position
    stepper.moveTo(currPosition);
    stepper.run();
    currPosition--;
  }
  stepper.setCurrentPosition(0);
  stepper.setMaxSpeed(100);
  currPosition = 0;
  gpio_put(ZEROSWITCH, 1);
  currPosition = 50;
  //Serial.print("before 200 move = ");
  MoveStepperToPositionCorrected(currPosition); //Al 4-20-20
  while (true) {      // move to zero position
    stepper.setMaxSpeed(100);
    currPosition--;
    stepper.moveTo(currPosition);
    stepper.run();
    if (gpio_get(ZEROSWITCH) == 0)
      break;
  }
  stepper.setCurrentPosition(0);
  currPosition = 200;
  while (1) {
    stepper.moveTo(currPosition);
    stepper.run();
    if (stepper.distanceToGo() == 0) {
      break;
    }
  }
  stepper.setMaxSpeed(100);
}


/*****
  Purpose: To move the cap[acitor to the approximate location via the stepper motor

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
  stepper.setMaxSpeed(FASTMOVESPEED);       // Get ready for a fast move
  stepper.setAcceleration(1100);
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
    stepper.moveTo(moveToStepperIndex);
    stepper.run();
    if (stepper.distanceToGo() == 0) {
      break;
    }
  }

#ifdef DEBUG1
  Serial.print("Move complete ");
  Serial.println(swr);
#endif

  totalPixels = YAXISEND - YAXISSTART;
  pixelsPerTenth = totalPixels / 50.0;      // HIghest SWR is 5, so 50 tenths.
  stepper.setMaxSpeed(NORMALMOVESPEED);
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
  currPosition = count;
  return count;
}


