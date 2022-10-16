/* A "proof of concept" project to replace the STM32F103 "Blue Pill"
   which is used in the "Magnetic Loop Controller" described in the book
   "Microcontroller Projects for Amateur Radio by Jack Purdum, W8TEE, and
   Albert Peter, AC8GY" with the Raspberry Pi Pico.
   Copyright (C) 2022  Gregory Raven

                                                    LICENSE AGREEMENT

  This program source code and its associated hardware design at subject to the GNU General Public License version 2,
                  https://opensource.org/licenses/GPL-2.0
  with the following additional conditions:
    1. Any commercial use of the hardware or software is prohibited without express, written, permission of the authors.
    2. This entire comment, unaltered, must appear at the top of the primary source file. In the Arduino IDE environemnt, this comment must
       appear at the top of the INO file that contains setup() and loop(). In any other environmentm, it must appear in the file containing
       main().
    3. This notice must appear in any derivative work, regardless of language used.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    A copy of the GPL-2.0 license is included in the repository as file LICENSE.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

#include "StepperManagement.h"

StepperManagement::StepperManagement(Data &data, AccelStepper::MotorInterfaceType interface, uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, bool enable) : data(data), AccelStepper(interface, pin1, pin2)
{
  position = 2500;          // Default to approximately midrange.
  setCurrentPosition(5000); //
}

// Revised version which includes MAXSWITCH detection.
//
void StepperManagement::MoveStepperToPositionCorrected(uint32_t position)
{
  // Acceleration needs to be set high, with maximum speed limit.
  setAcceleration(2000);
  setSpeed(500);
  setMaxSpeed(500);
  moveTo(position);

  while (distanceToGo() != 0)
  {
    run();
    if ((gpio_get(MAXSWITCH) == 0) or (gpio_get(ZEROSWITCH) == 0))
    {         // Protect limit switches.
      stop(); // Properly decelerate and stop the stepper.
      runToPosition();
      break;
    }
  }
}


void StepperManagement::ResetStepperToZero()
{
  // Acceleration needs to be set high, with maximum speed limit.
  setCurrentPosition(0);
  setAcceleration(2000);
  setSpeed(500);
  setMaxSpeed(500);
  // If ZEROSWITCH already low, move stepper forward a bit?
  if (gpio_get(ZEROSWITCH) == 0)
  {
    moveTo(500);
    runToPosition();
  }
  // Move in negative direction until ZEROSWITCH state changes.
  moveTo(-100000);
  while (distanceToGo() != 0)
  {
    run();
    // Detect zero switch, protect max switch.  Note that switch closure pulls the GPI low.
    if ((gpio_get(MAXSWITCH) == false) or (gpio_get(ZEROSWITCH) == false))
    {
      stop(); // Properly decelerate and stop the stepper.
      runToPosition();
      break;
    }
  }
  //  If MAXSWITCH switch closed, bail out!
  if (gpio_get(MAXSWITCH) == false)
    return;
  //  Set the zero calibration step.
  setCurrentPosition(0);
  moveTo(320); //  Move the stepper off the zero switch.  The number of steps required is dependent on the mechanics.
  runToPosition();
  setCurrentPosition(0); //  The stepper is now calibrated!
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
  switch (currentBand)
  {
  case 40: //   intercept                  + slopeCoefficient * newFrequency
    count = data.workingData.bandLimitPositionCounts[0][0] + (long)(countPerHertz[0] * ((float)(presentFrequency - data.LOWEND40M)));
    break;

  case 30:
    count = data.workingData.bandLimitPositionCounts[1][0] + (long)(countPerHertz[1] * ((float)(presentFrequency - data.LOWEND30M)));
    break;

  case 20:
    count = data.workingData.bandLimitPositionCounts[2][0] + (long)(countPerHertz[2] * ((float)(presentFrequency - data.LOWEND20M)));
    break;

  default:
    break;
  }
  position = count;
  return count;
}
