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
#define PICO_STACK_SIZE _u(0x1000)
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "Adafruit_ILI9341/Adafruit_ILI9341.h"
#include "DisplayManagement/DisplayManagement.h"
#include "AccelStepper/AccelStepper.h"
#include "StepperManagement/StepperManagement.h"
#include "DDS/DDS.h"
#include "SWR/SWR.h"
#include "Rotary/Rotary.h"
#include "EEPROM/EEPROM.h"
#include "Data/Data.h"
#include "Button/Button.h"

#define PIXELWIDTH 320  // Display limits
#define PIXELHEIGHT 240 // These are the post-rotation dimensions.

const std::string version = "main";
const std::string releaseDate = "10-12-22";

//  Interface for the DDS object.
#define DDS_RST 4
#define DDS_DATA 5
#define DDS_FQ_UD 12
#define WLCK 22

#define PRESETSMENU 1
#define CALIBRATEMENU 2
#define PRESETSPERBAND 6 // Allow this many preset frequencies on each band
#define MAXBANDS 3       // Can only process this many frequency bands

//int currentBand; // Should be 40, 30, or 20
//int currentFrequency;
//int whichBandOption;

volatile uint8_t result;
volatile uint32_t countEncoder;
//  Instantiate the rotary encoder objects.
Rotary menuEncoder = Rotary(20, 18); // Swap if encoder works in wrong direction.
Rotary frequencyEncoder = Rotary(21, 17);
extern int menuEncoderMovement;
extern int frequencyEncoderMovement;
extern int frequencyEncoderMovement2;
extern int digitEncoderMovement;

void encoderCallback(uint gpio, uint32_t events)
{
  if ((gpio == 18) || (gpio == 20))
  {
    result = menuEncoder.process();
    if (result != 0)
    {
      switch (result)
      {
      case DIR_CW:
        menuEncoderMovement = 1;
        digitEncoderMovement = 1;
        break;
      case DIR_CCW:
        menuEncoderMovement = -1;
        digitEncoderMovement = -1;
        break;
      }
    }
  }
  else if ((gpio == 17) || (gpio == 21))
  {
    result = frequencyEncoder.process();
    if (result != 0)
    {
      switch (result)
      {
      case DIR_CW:
        frequencyEncoderMovement++;
        frequencyEncoderMovement2 = 1;
        break;
      case DIR_CCW:
        frequencyEncoderMovement--;
        frequencyEncoderMovement2 = -1;
        break;
      }
    }
  }
  if (result == DIR_CW)
    countEncoder = countEncoder + 1;
  if (result == DIR_CCW)
    countEncoder = countEncoder - 1;
}

int main()
{
  stdio_init_all();

  // Initialize stepper and limit switch GPIOs:

  gpio_set_function(0, GPIO_FUNC_SIO); // Stepper Step
  gpio_set_function(1, GPIO_FUNC_SIO); // Stepper Dir

  gpio_set_function(2, GPIO_FUNC_SIO); // RF Amp Power
  gpio_set_function(3, GPIO_FUNC_SIO); // Op Amp Power

  gpio_set_function(10, GPIO_FUNC_SIO); // Limit switch
  gpio_set_function(11, GPIO_FUNC_SIO); // Limit switch
  gpio_set_function(19, GPIO_FUNC_SIO); // RF relay

  gpio_set_dir(0, GPIO_OUT); // Stepper Step
  gpio_set_dir(1, GPIO_OUT); // Stepper Dir

  gpio_set_dir(2, GPIO_OUT); // RF Amp Power
  gpio_put(2, false);        // RF Amp Power off
  gpio_set_dir(3, GPIO_OUT); // Op Amp Power
  gpio_put(3, false);        // Op Amp Power off

  gpio_set_dir(10, GPIO_IN);  // Limit switch
  gpio_set_dir(11, GPIO_IN);  // Limit switch
  gpio_set_dir(19, GPIO_OUT); // RF Relay
  gpio_put(19, false);
  //  The limit switch inputs need pull-ups:
  gpio_pull_up(10);
  gpio_pull_up(11);

  int currentFrequency;

  //  The data object manages constants and variables involved with frequencies, stepper motor positions,
  //  and GPIOs.
  Data data = Data();

  // The stepper drive generates pulses unless put in sleep mode.
  // The pulses will cause HF RF interference.  Therefore, the sleep mode must be used.
  // Subsequent power control will be done with the Power method in the DisplayManagement class.
  gpio_set_function(data.STEPPERSLEEPNOT, GPIO_FUNC_SIO);
  gpio_set_dir(data.STEPPERSLEEPNOT, GPIO_OUT);
  gpio_put(data.STEPPERSLEEPNOT, true); // Stepper set to active to allow reset to zero.

  //  Instantiate the display object.  Note that the SPI is handled in the display object.
  Adafruit_ILI9341 tft = Adafruit_ILI9341(PIN_CS, DISP_DC, -1);
  //  Configure the display object.
  tft.initSPI();
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);

  //  Instantiate the EEPROM object, which is actually composed of FLASH.
  EEPROM eeprom = EEPROM(data);
  //  Read the position counts and presets into the EEPROM object's buffer.
  eeprom.ReadEEPROMValuesToBuffer();
  //  Overwrite the position counts and preset frequencies:
  eeprom.ReadPositionCounts();
  // Slopes can't be computed until the actual values are loaded from FLASH:
  data.computeSlopes();

  //  Instantiate the Stepper Manager:
  uint32_t position;
  StepperManagement stepper = StepperManagement(data, AccelStepper::MotorInterfaceType::DRIVER, 0, 1);

  //  Next instantiate the DDS.
  DDS dds = DDS(DDS_RST, DDS_DATA, DDS_FQ_UD, WLCK);
  dds.DDSWakeUp(); // This resets the DDS, and it will have no output.

  // Instantiate SWR object.  Read bridge offsets later when other circuits are active.
  SWR swr = SWR();

  // Instantiate the DisplayManagement object.  This object has many important methods.
  DisplayManagement display = DisplayManagement(tft, dds, swr, stepper, eeprom, data);

  // Power on all circuits.  This is done early to allow circuits to stabilize before calibration.
  display.Power(true);

  // Show "Splash" screen for 5 seconds.  This also allows circuits to stabilize.
  display.Splash(version, releaseDate);
  busy_wait_ms(3000);
  tft.fillScreen(ILI9341_BLACK); // Clear display.

  // Set up the Menu and Frequency encoders:
  menuEncoder.begin(true, false);
  frequencyEncoder.begin(true, false);
  // Encoder interrupts:
  uint32_t events = 0x0000000C; // Rising and falling edges.
  countEncoder = 0;
  gpio_set_irq_enabled_with_callback(17, events, 1, &encoderCallback);
  gpio_set_irq_enabled_with_callback(18, events, 1, &encoderCallback);
  gpio_set_irq_enabled_with_callback(20, events, 1, &encoderCallback);
  gpio_set_irq_enabled_with_callback(21, events, 1, &encoderCallback);

  // The default band is read from Flash and stored in the data object.  Default band save is not currently implemented.
  // Defaults to 40 meters.
  data.currentBand = eeprom.ReadCurrentBand();
/*
  switch (data.currentBand)
  { // Set the frequency default as 1st preset frequency
  case 40:
    currentFrequency = data.presetFrequencies[0][2];
    break;
  case 30:
    currentFrequency = data.presetFrequencies[1][0];
    break;
  case 20:
    currentFrequency = data.presetFrequencies[2][0];
    break;
  default:
    break;
  }
*/
  //  Set stepper to zero:
  display.updateMessageTop("                Resetting to Zero");
  stepper.ResetStepperToZero();

  //  Now measure the ADC (SWR bridge) offsets with the DDS inactive.
  //  Note that this should be done as late as possible for circuits to stabilize.
  dds.SendFrequency(0); // Is this redundant?
  swr.ReadADCoffsets();

  //  Retrieve the last used frequency.
  currentFrequency = eeprom.ReadCurrentFrequency();
  if(currentFrequency != 0) {
  dds.SendFrequency(currentFrequency); // Set the DDSs
  // Retrieve the last used frequency and autotune.
  position = -25 + data.bandLimitPositionCounts[data.currentBand][0] + float((dds.currentFrequency - data.bandEdges[data.currentBand][0])) / float(data.hertzPerStepperUnitVVC[data.currentBand]);
  stepper.MoveStepperToPositionCorrected(position); // Al 4-20-20
  display.AutoTuneSWR();
  }

  display.menuIndex = FREQMENU; // Begin in Frequency menu.
  //whichBandOption = 0;

  // Main loop state machine:
  while (true)
  {
    int i, submenuIndex;
    // Turn on power.
    // display.Power(true);
    //  Refresh display:
    display.ShowMainDisplay(display.menuIndex); //  This function erases the entire display.
    //display.PowerSWR(true);                     //  Power up only SWR circuits.  This is done here to show accurate SWR in the top level menu.
    //display.ShowSubmenuData(swr.ReadSWRValue(), dds.currentFrequency);
    display.ShowSubmenuData(display.minSWR, dds.currentFrequency);
    display.Power(false);                                             //  Power down all circuits.  This function is used since stepper will be active at start-up.
    display.menuIndex = display.MakeMenuSelection(display.menuIndex); // Select one of the three top menu choices: Freq, Presets, 1st Cal.

    switch (display.menuIndex)
    {
    case FREQMENU: //  Manual frequency selection selection and AutoTune.
      display.frequencyMenuOption();
      break;

    case PRESETSMENU:           // Preset frequencies by band - set in .ino file, variable: presetFrequencies[0][2];
      display.ProcessPresets(); // Select a preselected frequency.  This should return a frequency???
      break;

    case CALIBRATEMENU: //  Run calibration routines.
      display.CalibrationMachine();
      break;

    default:
      break;
    } // switch (menuIndex)
  }   // while(1)  (end of main loop)

  return 0; // Program should never reach this statement.
}
