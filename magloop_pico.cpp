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


#define PIXELWIDTH 320  // Display limits
#define PIXELHEIGHT 240 // These are the post-rotation dimensions.
#define VERSION 1.01
#define RELEASEDATE "3-15-21"

//  Instantiate the display object.  Note that the SPI is handled in the display object.
Adafruit_ILI9341 tft = Adafruit_ILI9341(PIN_CS, DISP_DC, -1);
//  Instantiate the Stepper object:
#define STEPPERDIR                  13
#define STEPPERPUL                  12
#define ZEROSWITCH                  10
#define MAXSWITCH                   11
//AccelStepper stepper = AccelStepper(1, STEPPERPUL, STEPPERDIR);
//  Instantiate the Stepper Manager:
//StepperManagement steppermanage = StepperManagement(stepper);

void ErasePage()
{
  tft.fillScreen(ILI9341_BLACK);
}

// The Splash function from the Mag Loop Arduino .ino file.
void Splash()
{
  ErasePage();
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
  //  Configure the display object.
  tft.initSPI();
  tft.begin();
  tft.setRotation(3);

// Initialize stepper GPIOs:
  gpio_init(STEPPERDIR);
  gpio_init(STEPPERPUL);
  gpio_set_dir(STEPPERDIR, GPIO_OUT);
  gpio_set_dir(STEPPERPUL, GPIO_OUT);
  gpio_put(STEPPERDIR, 0);
  gpio_put(STEPPERPUL, 0);
  //  Initialize limit switch inputs:
  gpio_init(ZEROSWITCH);
  gpio_init(MAXSWITCH);
  gpio_set_dir(ZEROSWITCH, GPIO_IN);
  gpio_set_dir(MAXSWITCH, GPIO_IN);
  //  The limit switch inputs need pull-ups:
  gpio_pull_up(ZEROSWITCH);
  gpio_pull_up(MAXSWITCH);

  //  Run the same Splash function as in the Mag Loop Controller project.
  Splash();

  // Test a GFX graphics primitive by drawing a border:
  tft.drawRect(1, 1, 318, 238, ILI9341_WHITE);

  //gpio_put(ZEROSWITCH, 1);
  //gpio_put(MAXSWITCH, 1);
  //steppermanage.ResetStepperToZero();

  return 0;
}
