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
#include "AccelStepper/AccelStepper.h"
#include "magloop/StepperManagement.h"
#include "DDS/DDS.h"
#include "SWR/SWR.h"
#include "AutoTune/AutoTune.h"

#include <Fonts/FreeSerif24pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/FreeSerif12pt7b.h>

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


void ErasePage(Adafruit_ILI9341 tft)
{
  tft.fillScreen(ILI9341_BLACK);
}

// The Splash function from the Mag Loop Arduino .ino file.

void Splash(Adafruit_ILI9341 tft)
{
  ErasePage(tft);
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

int main()
{
  stdio_init_all();


  // Initialize stepper GPIOs:
  gpio_set_function( 9, GPIO_FUNC_SIO);
  gpio_set_function(10, GPIO_FUNC_SIO);
  gpio_set_function(11, GPIO_FUNC_SIO);
  gpio_set_function(12, GPIO_FUNC_SIO);
  gpio_set_dir( 9, GPIO_OUT);
  gpio_set_dir(12, GPIO_OUT);
  gpio_set_dir(10, GPIO_IN);  // Limit switch
  gpio_set_dir(11, GPIO_IN);  // Limit switch
  gpio_put( 9, 0);
  gpio_put(12, 0);

  //  The limit switch inputs need pull-ups:
  gpio_pull_up(10);
  gpio_pull_up(11);

  //gpio_set_function( 0, GPIO_FUNC_SIO);
  //gpio_set_dir( 0, GPIO_OUT);
  //gpio_put( 0, 0);
  //gpio_put( 0, 1);

  //  Instantiate the display object.  Note that the SPI is handled in the display object.
  Adafruit_ILI9341 tft = Adafruit_ILI9341(PIN_CS, DISP_DC, -1);
  //  Configure the display object.
  tft.initSPI();
  tft.begin();
  tft.setRotation(3);

  //  Run the same Splash function as in the Mag Loop Controller project.
  Splash(tft);

  // Test a GFX graphics primitive by drawing a border:
  tft.drawRect(1, 1, 318, 238, ILI9341_WHITE);

  //  Create the stepper object:
  AccelStepper stepper = AccelStepper(1, STEPPERPUL, STEPPERDIR);
  //  Instantiate the Stepper Manager:
  StepperManagement steppermanage = StepperManagement(stepper);

  steppermanage.ResetStepperToZero();

  stepper.setMaxSpeed(5000);
  stepper.setAcceleration(1100);

  stepper.runToNewPosition(2500);

  //stepper.disableOutputs();

  //  Next test the DDS.
  DDS dds = DDS(DDS_RST, DDS_DATA, DDS_FQ_UD, WLCK);
  dds.DDSWakeUp();
  dds.SendFrequency(7045000);

  // Instantiate SWR object.
  
  SWR swr = SWR(steppermanage, tft);
  /*
  const float conversion_factor = 3.3f/(1 << 12);
  double VSWR;
  VSWR = swr.ReadSWRValue();
  ErasePage(tft);
  tft.setTextSize(2);
  while(1){
  ErasePage(tft);
  tft.setCursor(10, 20);
  adc_select_input(0);
  tft.print(adc_read() * conversion_factor);
  tft.setCursor(10, 50);
  adc_select_input(1);
  tft.print(adc_read() * conversion_factor);
  busy_wait_us_32(20000);
  }
  */
 
 
 
AutoTune autotune = AutoTune(stepper, swr, tft, steppermanage, display);
Calibrate calibrate = Calibrate(display, stepper, steppermanage, tft, dds, swr, autotune);
Presets presets = Presets(tft, steppermanage, stepper, dds, autotune, swr, display);
Buttons buttons = Buttons(display, presets, dds, calibrate);
DisplayManagement display = DisplayManagement(tft, calibrate, dds, swr, stepper, autotune, steppermanage, buttons);

 
 //autotune.AutoTuneSWRQuick();
autotune.MoveStepperToPositionCorrected(2000);


  return 0;
}
