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
//#include "arduino_includes/Arduino.h"
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
const std::string version = "1.01";
const std::string releaseDate = "3-15-21";

//  Instantiate the Stepper object:
//#define STEPPERDIR 9
//#define STEPPERPUL 12

//  Interface for the DDS object.
#define DDS_RST   4
#define DDS_DATA  5
#define DDS_FQ_UD 12
#define WLCK      22

#define PRESETSMENU 1
#define CALIBRATEMENU 2

// +12V and +5V power switch GPIO:
//#define POWER_SWITCH 28

#define PRESETSPERBAND              6                   // Allow this many preset frequencies on each band
#define MAXBANDS                    3                   // Can only process this many frequency bands

int currentBand;      // Should be 40, 30, or 20
int currentFrequency;
int whichBandOption;

volatile uint8_t result;
volatile uint32_t countEncoder;
Rotary menuEncoder = Rotary(20, 18);   // Swap if encoder works in wrong direction.
Rotary frequencyEncoder = Rotary(17, 21);
extern int menuEncoderMovement;
extern int frequencyEncoderMovement;
extern int frequencyEncoderMovement2;
extern int digitEncoderMovement;
extern int quickCalFlag;

void encoderCallback(uint gpio, uint32_t events) {
if((gpio == 18) || (gpio == 20)) {
  result = menuEncoder.process();
  if (result != 0) {
    switch (result) {
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
else if((gpio == 17) || (gpio == 21)) {
result = frequencyEncoder.process();
  if (result != 0) {
    switch (result) {
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
if(result == DIR_CW ) countEncoder = countEncoder + 1;
if(result == DIR_CCW ) countEncoder = countEncoder - 1;
}

int main()
{
  stdio_init_all();

  // Initialize stepper and limit switch GPIOs:
  
  gpio_set_function( 0, GPIO_FUNC_SIO);  // Stepper Step
  gpio_set_function( 1, GPIO_FUNC_SIO);  // Stepper Dir

  gpio_set_function( 2, GPIO_FUNC_SIO);  // RF Amp Power
  gpio_set_function( 3, GPIO_FUNC_SIO);  // Op Amp Power
  gpio_set_function( 9, GPIO_FUNC_SIO);  // Stepper Sleep Not

  gpio_set_function(10, GPIO_FUNC_SIO);  // Limit switch
  gpio_set_function(11, GPIO_FUNC_SIO);  // Limit switch
  gpio_set_function(19, GPIO_FUNC_SIO);
  
  gpio_set_dir( 0, GPIO_OUT);  // Stepper Step
  gpio_set_dir( 1, GPIO_OUT);  // Stepper Dir

gpio_set_dir( 2, GPIO_OUT);  // RF Amp Power
gpio_put(2, false);
gpio_set_dir( 3, GPIO_OUT);  // Op Amp Power
gpio_put(3, false);
gpio_set_dir( 9, GPIO_OUT);  // Stepper Sleep Not
gpio_put(9, false);

  gpio_set_dir(10, GPIO_IN);  // Limit switch
  gpio_set_dir(11, GPIO_IN);  // Limit switch
  gpio_set_dir(19, GPIO_OUT);  // RF Relay
  gpio_put(19, false);
  //  The limit switch inputs need pull-ups:
  gpio_pull_up(10);
  gpio_pull_up(11);

  //  The data object manages constants and variables involved with frequencies, stepper motor positions,
  //  and GPIOs.
  Data data = Data();

  // Initialize power switch GPIO, and then set to ON.
  // Subsequent power control will be done with the Power method in the DisplayManagement class.
  gpio_set_function(data.STEPPERSLEEPNOT, GPIO_FUNC_SIO);
  gpio_set_dir(data.STEPPERSLEEPNOT, GPIO_OUT);
  gpio_put(data.STEPPERSLEEPNOT, true);  // Stepper sleep control not currently used.

  //  Instantiate the display object.  Note that the SPI is handled in the display object.
  Adafruit_ILI9341 tft = Adafruit_ILI9341(PIN_CS, DISP_DC, -1);
  //  Configure the display object.
  tft.initSPI();
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);
  

  uint32_t eeprom_data;
  EEPROM eeprom = EEPROM(data);
  //  Use this method one time only and then comment out!
  //eeprom.WriteDefaultEEPROMValues();
  //  Read the position counts and presets into the EEPROM object's buffer.
  eeprom.ReadEEPROMValuesToBuffer();
  //  Overwrite the position counts and preset frequencies:
  eeprom.ReadPositionCounts();
  // Slopes can't be computed until the actual values are loaded from flash:
  data.computeSlopes();
  
  //  Instantiate the Stepper Manager:
  StepperManagement stepper = StepperManagement(data, AccelStepper::MotorInterfaceType::DRIVER, 0, 1);

  //  Next instantiate the DDS.
  DDS dds = DDS(DDS_RST, DDS_DATA, DDS_FQ_UD, WLCK);
  dds.DDSWakeUp();  // This resets the DDS and it will have no output.

// Instantiate SWR object.  Read bridge offsets later.
SWR swr = SWR();

DisplayManagement display = DisplayManagement(tft, dds, swr, stepper, eeprom, data);

// Power on circuits.
display.Power(true);

// Show "Splash" screen for 5 seconds.
display.Splash(version, releaseDate);
busy_wait_ms(5000);
tft.fillScreen(ILI9341_BLACK);

// Set up the Menu and Frequency encoders:
menuEncoder.begin(true, false);
frequencyEncoder.begin(true, false);
// Encoder interrupts:
uint32_t events = 0x0000000C;  // Rising and falling edges.
countEncoder = 0;
gpio_set_irq_enabled_with_callback(17, events, 1, &encoderCallback);
gpio_set_irq_enabled_with_callback(18, events, 1, &encoderCallback);
gpio_set_irq_enabled_with_callback(20, events, 1, &encoderCallback);
gpio_set_irq_enabled_with_callback(21, events, 1, &encoderCallback);
//gpio_set_irq_enabled_with_callback( 6, events, 1, &encoderCallback);

// The default band is read from Flash.
currentBand = eeprom.ReadCurrentBand();

  switch (currentBand) {     // Set the frequency default as 1st preset frequency
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

//  Set to zero and calibrate stepper:
  display.updateMessageTop("                Resetting to Zero");
  stepper.ResetStepperToZero();

  //  Now measure the ADC (SWR bridge) offsets with the DDS inactive.
  //  Note that this should be done as late as possible for circuits to stabilize.
  dds.SendFrequency(0);
  swr.ReadADCoffsets();
  
  dds.SendFrequency(currentFrequency);    // Set the DDS

  display.menuIndex = FREQMENU;  // Begin in Frequency menu.
  whichBandOption = 0;

// Main loop/state machine:
while(true) {
  int i, submenuIndex;
  // Turn on power.
  display.Power(true);
  //  Refresh display:
  display.ShowMainDisplay(display.menuIndex);
  display.ShowSubmenuData(swr.ReadSWRValue(), dds.currentFrequency);
  // Turn off power.
  display.Power(false);
  display.menuIndex = display.MakeMenuSelection(display.menuIndex);  // Select one of the three top menu choices: Freq, Presets, 1st Cal

  switch (display.menuIndex) {
    case FREQMENU:             //  Manual frequency selection selection and AutoTune.
      display.frequencyMenuOption();
      break;

    case PRESETSMENU:          //Preset frequencies by band - set in .ino file, variable: presetFrequencies[0][2];
      display.ProcessPresets(); // Select a preselected frequency.  This should return a frequency???
      break;

    case CALIBRATEMENU:       //  Run calibration routines.
      display.CalibrationMachine();
      break;

    default:
      break;
  } // switch (menuIndex)
}  // while(1)  (end of main loop)

  return 0;
}
