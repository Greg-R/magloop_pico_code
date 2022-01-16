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
#include "arduino_includes/Arduino.h"
#include "AccelStepper/AccelStepper.h"
#include "StepperManagement/StepperManagement.h"
#include "DDS/DDS.h"
#include "SWR/SWR.h"
#include "AutoTune/AutoTune.h"
#include "Rotary/Rotary.h"

//#include "Adafruit-GFX-Library/gfxfont.h"
#include "Adafruit-GFX-Library/Fonts/FreeSerif24pt7b.h"
#include "Adafruit-GFX-Library/Fonts/FreeSerif9pt7b.h"
#include "Adafruit-GFX-Library/Fonts/FreeSerif12pt7b.h"

#define PIXELWIDTH 320  // Display limits
#define PIXELHEIGHT 240 // These are the post-rotation dimensions.
#define VERSION 1.01
#define RELEASEDATE "3-15-21"

//  Instantiate the display object.  Note that the SPI is handled in the display object.
// Adafruit_ILI9341 tft = Adafruit_ILI9341(PIN_CS, DISP_DC, -1);

//  Instantiate the Stepper object:
#define STEPPERDIR 9
#define STEPPERPUL 12
#define ZEROSWITCH 11
#define MAXSWITCH 10
//AccelStepper stepper = AccelStepper(1, STEPPERPUL, STEPPERDIR);
//  Instantiate the Stepper Manager:
//StepperManagement steppermanage = StepperManagement(stepper);

//  Interface for the DDS object.
#define DDS_RST   3
#define DDS_DATA  2
#define DDS_FQ_UD 1
#define WLCK      0

// +12V and +5V power switch GPIO:
#define POWER_SWITCH 28

// The Splash function from the Mag Loop Arduino .ino file.

void Splash(Adafruit_ILI9341 tft)
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
  tft.print(VERSION);
  tft.setCursor(65, PIXELHEIGHT - 20);
  tft.print("Release Date ");
  tft.print(RELEASEDATE);
  tft.setTextSize(2);
}

    enum class MotorInterfaceType: uint8_t
    {
	FUNCTION  = 0, ///< Use the functional interface, implementing your own driver functions (internal use only)
	DRIVER    = 1, ///< Stepper Driver, 2 driver pins required
	FULL2WIRE = 2, ///< 2 wire stepper, 2 motor pins required
	FULL3WIRE = 3, ///< 3 wire stepper, such as HDD spindle, 3 motor pins required
    FULL4WIRE = 4, ///< 4 wire full stepper, 4 motor pins required
	HALF3WIRE = 6, ///< 3 wire half stepper, such as HDD spindle, 3 motor pins required
	HALF4WIRE = 8  ///< 4 wire half stepper, 4 motor pins required
    };

    #define DIR_CW 0x10

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

  // Initialize power switch GPIO, and then set to OFF:
  gpio_set_function(POWER_SWITCH, GPIO_FUNC_SIO);
  gpio_set_dir(POWER_SWITCH, GPIO_OUT);
  gpio_put(POWER_SWITCH, 0);

  // Initialize 7 buttons and pull-ups.  Buttons are normally open.
  gpio_set_function( 4, GPIO_FUNC_SIO);  // FULLCAL
  gpio_set_function( 5, GPIO_FUNC_SIO);  // PRESETS
  gpio_set_function( 6, GPIO_FUNC_SIO);  // ACCURACY
  gpio_set_function( 7, GPIO_FUNC_SIO);  // AUTOTUNE
  gpio_set_function( 8, GPIO_FUNC_SIO);  // BANDCAL
  gpio_set_function(19, GPIO_FUNC_SIO);  // Encoder 1
  gpio_set_function(20, GPIO_FUNC_SIO);  // Encoder 2
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

  // Initialize the Encoders:
/*
  gpio_set_function(18, GPIO_FUNC_SIO);  // Encoder 1 Data
  gpio_set_function(17, GPIO_FUNC_SIO);  // Encoder 1 Clock
  gpio_set_function(22, GPIO_FUNC_SIO);  // Encoder 2 Data
  gpio_set_function(21, GPIO_FUNC_SIO);  // Encoder 2 Clock
  gpio_set_dir(18, GPIO_IN);
  gpio_set_dir(17, GPIO_IN);
  gpio_set_dir(22, GPIO_IN);
  gpio_set_dir(21, GPIO_IN);
*/

  //  Instantiate the display object.  Note that the SPI is handled in the display object.
  Adafruit_ILI9341 tft = Adafruit_ILI9341(PIN_CS, DISP_DC, -1);
  //  Configure the display object.
  tft.initSPI();
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(ILI9341_WHITE);
  //  Run the same Splash function as in the Mag Loop Controller project.
  Splash(tft);

  // Test a GFX graphics primitive by drawing a border:
  tft.drawRect(1, 1, 318, 238, ILI9341_WHITE);

  //  Turn on the power!
  gpio_put(POWER_SWITCH, 1);

//  Instantiate the Stepper Manager: 
  StepperManagement stepper = StepperManagement(AccelStepper::MotorInterfaceType::DRIVER, STEPPERPUL, STEPPERDIR);

//AccelStepper stepper =AccelStepper(1, STEPPERPUL, STEPPERDIR);

//  Examples of Stepper control functions:
//  steppermanage.ResetStepperToZero();
//  steppermanage.setCurrentPosition(0);  //  Sets max speed to zero!
//  steppermanage.setMaxSpeed(500);
//  steppermanage.setAcceleration(110);
//  steppermanage.runToNewPosition(2675);
//  steppermanage.disableOutputs();

//  Next test the DDS.
  DDS dds = DDS(DDS_RST, DDS_DATA, DDS_FQ_UD, WLCK);
  dds.DDSWakeUp();
  dds.SendFrequency(0);
  
// Instantiate SWR object.
  SWR swr = SWR(stepper, tft);
//  Now measure the ADC offsets before the DDS is active.
  swr.ReadADCoffsets();
  dds.SendFrequency(8045000);
  dds.SendFrequency(8045000);
  
AutoTune autotune = AutoTune(swr, tft, stepper);
//Calibrate calibrate = Calibrate(display, stepper, steppermanage, tft, dds, swr, autotune);
//Presets presets = Presets(tft, steppermanage, stepper, dds, autotune, swr, display);
//Buttons buttons = Buttons(display, presets, dds, calibrate);
//DisplayManagement display = DisplayManagement(tft, calibrate, dds, swr, stepper, autotune, steppermanage, buttons);

tft.fillScreen(ILI9341_BLACK);
tft.setTextSize(4);
tft.setCursor(80, 40);
tft.print("TUNING!");
//autotune.AutoTuneSWRQuick();

float VSWR;
VSWR = swr.ReadSWRValue();
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(50, 10);
  tft.print("TUNED SWR:");
  tft.setCursor(110, 50);
  tft.print(VSWR, 3);
//busy_wait_ms(5000);
/*  Troubleshoot VSWR Bridge
  tft.setTextSize(3);
  tft.setCursor(20, 90);
  tft.print(swr.forward_offset);
  tft.setCursor(110, 90);
  tft.print(swr.forward_voltage);
  tft.setCursor(20, 120);
  tft.print(swr.reverse_offset);
  tft.setCursor(110, 120);
  tft.print(swr.reverse_voltage);
  int forward, reverse;
  adc_select_input(0);
  reverse = adc_read();
  busy_wait_ms(1000);
  adc_select_input(0);
  forward = adc_read();
  tft.setCursor(20, 150);
  tft.print(reverse);
  tft.setCursor(160, 150);
  tft.print(forward);
*/

// Power down
dds.SendFrequency(0);
gpio_put(POWER_SWITCH, 0);

/* Test the 5 pushbottons:
tft.fillScreen(ILI9341_BLACK);
tft.setTextSize(2);
while(1) {
if(!gpio_get(4)) tft.print("Button 4");
else if(!gpio_get(5)) tft.print("Button 5");
else if(!gpio_get(6)) tft.print("Button 6");
else if(!gpio_get(7)) tft.print("Button 7");
else if(!gpio_get(8)) tft.print("Button 8");
else if(!gpio_get(19)) tft.print("Encoder 1");
else if(!gpio_get(20)) tft.print("Encoder 2");
else tft.print("No button pressed");
busy_wait_ms(200);
tft.fillScreen(ILI9341_BLACK);
tft.setCursor(20, 90);
}
*/

// Test the rotary encoders:

Rotary menuEncoder = Rotary(17, 18);
menuEncoder.begin(true, false);
   tft.fillScreen(ILI9341_BLACK);
   tft.setTextSize(2);
unsigned char result;
while(1) {
    result = menuEncoder.process();
  //  Serial.println(result == DIR_CW ? "Right" : "Left");
    tft.print(result == DIR_CW ? "Right" : "Left");
  //  tft.print(result);
    busy_wait_ms(100);
    tft.fillScreen(ILI9341_BLACK);
    tft.setTextSize(2);
    tft.setCursor(20, 90);
}

  return 0;
}
