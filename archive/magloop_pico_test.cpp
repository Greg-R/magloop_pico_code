// A "proof of concept" project to replace the STM32F103 "Blue Pill"
// which is used in the "Magnetic Loop Controller" described in the book
// "Microcontroller Projects for Amateur Radio by Jack Purdum, W8TEE, and
// Albert Peter, AC8GY" with the Raspberry Pi Pico.
// The project is to modify the Adafruit_ILI9341 to work
// with the Pi Pico as a dedicated library, and then demonstrate the
// loading of the "Splash" screen display which appears at power-up
// in the original project.
// The original library:
// https://github.com/adafruit/Adafruit_ILI9341

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "Adafruit_ILI9341/Adafruit_ILI9341.h"
#include "DisplayManagement/DisplayManagement.h"
#include "arduino_includes/Arduino.h"
#include "AccelStepper/AccelStepper.h"
//#include "Calibrate/Calibrate.h"
//#include "Presets/Presets.h"
#include "StepperManagement/StepperManagement.h"


#define PIXELWIDTH 320  // Display limits
#define PIXELHEIGHT 240 // These are the post-rotation dimensions.
const std::string version = "1.01";
const std::string releaseDate = "3-15-21";

//  Instantiate the display object.  Note that the SPI is handled in the display object.
// Adafruit_ILI9341 tft = Adafruit_ILI9341(PIN_CS, DISP_DC, -1);

//  Instantiate the Stepper object:
#define STEPPERDIR 9
#define STEPPERPUL 12


// +12V and +5V power switch GPIO:
#define POWER_SWITCH 28


void SplashTest(Adafruit_ILI9341 tft)
{
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_MAGENTA, ILI9341_BLACK);
  tft.setCursor(10, 20);
  tft.print("Microcontroller Projects");
  tft.setCursor(40, 45);
  tft.print("for Amateur Radio");
  tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
  tft.setCursor(PIXELWIDTH / 2 - 30, PIXELHEIGHT / 4 + 20);
  tft.print("by");
  tft.setCursor(65, PIXELHEIGHT - 100);
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  tft.println("Al Peter  AC8GY");
  tft.setCursor(45, PIXELHEIGHT - 70);
  tft.print("Jack Purdum  W8TEE");
  tft.setCursor(65, PIXELHEIGHT - 40);
  tft.print("Version ");
  tft.print(version.c_str());
  tft.setCursor(65, PIXELHEIGHT - 20);
  tft.print("Release Date ");
  tft.print(releaseDate.c_str());
  tft.setTextSize(2);
}


int main()
{
  stdio_init_all();

  // Initialize stepper GPIOs:
  gpio_set_function( 9, GPIO_FUNC_SIO);
  gpio_set_function(10, GPIO_FUNC_SIO);
  gpio_set_function(11, GPIO_FUNC_SIO);
  gpio_set_function(12, GPIO_FUNC_SIO);
  gpio_set_dir( 9, GPIO_OUT);  // Stepper Dir
  gpio_set_dir(12, GPIO_OUT);  // Stepper Step
  gpio_set_dir(10, GPIO_IN);  // Limit switch
  gpio_set_dir(11, GPIO_IN);  // Limit switch
//  The limit switch inputs need pull-ups:
  gpio_put( 9, 0);
  gpio_put(12, 0);
  gpio_pull_up(10);
  gpio_pull_up(11);

  // Initialize power switch GPIO, and then set to ON:
  gpio_set_function(POWER_SWITCH, GPIO_FUNC_SIO);
  gpio_set_dir(POWER_SWITCH, GPIO_OUT);
  gpio_put(POWER_SWITCH, 1);

  // Initialize 7 buttons and pull-ups.  Buttons are normally open.
  gpio_set_function( 4, GPIO_FUNC_SIO);  // FULLCAL
  gpio_set_function( 5, GPIO_FUNC_SIO);  // PRESETS
  gpio_set_function( 6, GPIO_FUNC_SIO);  // ACCURACY
  gpio_set_function( 7, GPIO_FUNC_SIO);  // AUTOTUNE
  gpio_set_function( 8, GPIO_FUNC_SIO);  // BANDCAL
  gpio_set_function(19, GPIO_FUNC_SIO);  // Encoder 1 pushbutton
  gpio_set_function(20, GPIO_FUNC_SIO);  // Encoder 2 pushbutton
  gpio_set_dir( 4, GPIO_IN);
  gpio_set_dir( 5, GPIO_IN);
  gpio_set_dir( 6, GPIO_IN);
  gpio_set_dir( 7, GPIO_IN);
  gpio_set_dir( 8, GPIO_IN);
  gpio_set_dir(19, GPIO_IN);
  gpio_set_dir(20, GPIO_IN);
  gpio_pull_up( 4);
  gpio_pull_up( 5);
  gpio_pull_up( 6);
  gpio_pull_up( 7);
  gpio_pull_up( 8);
  gpio_pull_up(19);
  gpio_pull_up(20);

  //  Instantiate the display object.  Note that the SPI is handled in the display object.
  Adafruit_ILI9341 tft = Adafruit_ILI9341(PIN_CS, DISP_DC, -1);
  //  Configure the display object.
  tft.initSPI();
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);
  //  Run the same Splash function as in the Mag Loop Controller project.
  
  StepperManagement stepper = StepperManagement(AccelStepper::MotorInterfaceType::DRIVER, STEPPERPUL, STEPPERDIR);
 
//DisplayManagement display = DisplayManagement(tft, dds, swr, stepper, eeprom, data);

//Calibrate calibrate = Calibrate(display, stepper, tft, dds, swr, autotune);
//Presets presets = Presets(tft, stepper, dds, autotune, swr, display);
//Buttons buttons = Buttons(display, presets, dds, calibrate);
//display.Splash(version, releaseDate);

// Power down
//dds.SendFrequency(0);
//gpio_put(POWER_SWITCH, 0);

//stepper.setCurrentPosition(0);
//stepper.setMaxSpeed(100);
//stepper.setAcceleration(110);
//stepper.move(500);
//stepper.runToPosition();
//stepper.move(-250);
//stepper.runToPosition();
//for(int i = 0; i < 20; i++) {
stepper.ResetStepperToZero();
//stepper.MoveStepperToPositionCorrected(1000);
//}
unsigned long interval;
stepper.setMaxSpeed(500.0);
  stepper.setAcceleration(2000);
  stepper.setSpeed(500.0);
  stepper.setMinPulseWidth(100);
  interval = stepper._stepInterval;
        tft.fillScreen(ILI9341_BLACK);
      tft.setCursor(20, 90);
      tft.print(interval);
//stepper.moveTo(4000);
//stepper.runToPosition();
//  for (int i = 0; i < 1000000; i++)
//while(stepper.distanceToGo() != 0)
stepper.moveTo(1000);
stepper.setSpeed(500.0);
while(gpio_get(MAXSWITCH) != 0)
  {
   if(stepper.distanceToGo() == 0) break;
   stepper.runSpeed();
//   

  }

  busy_wait_ms(5000);
//  Set speed negative to reverse direction.
stepper.setSpeed(-500.0);

while(gpio_get(ZEROSWITCH) != 0)
  {
   
   stepper.runSpeed();
//   

  }

  return 0;
}
