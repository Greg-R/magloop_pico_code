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

#define PIXELWIDTH 320  // Display limits
#define PIXELHEIGHT 240 // These are the post-rotation dimensions.
const std::string version = "1.01";
const std::string releaseDate = "3-15-21";

//  Instantiate the Stepper object:
#define STEPPERDIR 9
#define STEPPERPUL 12

//  Interface for the DDS object.
#define DDS_RST   3
#define DDS_DATA  2
#define DDS_FQ_UD 1
#define WLCK      0

#define PRESETSMENU 1
#define CALIBRATEMENU 2

// +12V and +5V power switch GPIO:
#define POWER_SWITCH 28

#define PRESETSPERBAND              6                   // Allow this many preset frequencies on each band
#define MAXBANDS                    3                   // Can only process this many frequency bands

int currentBand;      // Should be 40, 30, or 20
int currentFrequency;
int whichBandOption;
//float countPerHertz[3];
//float hertzPerStepperUnitAir[3];

volatile uint8_t result;
volatile uint32_t countEncoder;
Rotary menuEncoder = Rotary(18, 17);
Rotary frequencyEncoder = Rotary(22, 21);
extern int menuEncoderMovement;
extern int frequencyEncoderMovement;
extern int frequencyEncoderMovement2;
extern int digitEncoderMovement;

void encoderCallback(uint gpio, uint32_t events) {
if((gpio == 17) || (gpio == 18)) {
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
else if((gpio == 21) || (gpio == 22)) {
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
  

  //  The data object manages constants and variables involved with frequencies and stepper motor positions.
  Data data = Data();
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
StepperManagement stepper = StepperManagement(data, AccelStepper::MotorInterfaceType::DRIVER, STEPPERPUL, STEPPERDIR);

//  Next instantiate the DDS.
  DDS dds = DDS(DDS_RST, DDS_DATA, DDS_FQ_UD, WLCK);
  dds.DDSWakeUp();  // This resets the DDS and it will have no output.

// Instantiate SWR object.
SWR swr = SWR();
//  Now measure the ADC offsets before the DDS is active.
swr.ReadADCoffsets();

DisplayManagement display = DisplayManagement(tft, dds, swr, stepper, eeprom, data);
// Show "Splash" screen for 5 seconds.
display.Splash(version, releaseDate);
busy_wait_ms(2000);
tft.fillScreen(ILI9341_BLACK);

// Set up the Menu and Frequency encoders:
menuEncoder.begin(true, false);
frequencyEncoder.begin(true, false);
// Encoder interrupts:
uint32_t events = 0x0000000C;  // Rising and falling edges.
countEncoder = 0;
gpio_set_irq_enabled_with_callback(18, events, 1, &encoderCallback);
gpio_set_irq_enabled_with_callback(17, events, 1, &encoderCallback);
gpio_set_irq_enabled_with_callback(21, events, 1, &encoderCallback);
gpio_set_irq_enabled_with_callback(22, events, 1, &encoderCallback);

// The default band is read from Flash.
currentBand = eeprom.ReadCurrentBand();

  switch (currentBand) {              // Set the frequency default as 1st preset frequency
    case 40:
      currentFrequency = data.presetFrequencies[0][2];
      //currentFrequency = 7150000L;
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
  dds.SendFrequency(currentFrequency);    // Set the DDS
  busy_wait_ms(100);                      // Let DDS stabilize

//  Set to zero and calibrate stepper:
  display.updateMessage("Resetting to Zero");
  stepper.ResetStepperToZero();

  display.menuIndex = FREQMENU;
  whichBandOption = 0;

display.UpdateFrequency(dds.currentFrequency);  //  Updates position after calibrating stepper.
display.ShowMainDisplay();
display.ShowSubmenuData(swr.ReadSWRValue(), currentFrequency); 

// Main state machine:
while(1) {
  if (display.quickCalFlag == 1) {
    display.DoSingleBandCalibrate(display.whichBandOption);
    display.quickCalFlag = 0;
  }
  std::string band[] = {"40M", "30M", "20M"};
  int i, submenuIndex;
  long minCount;
  int currPosIndexStart;
  display.menuIndex = display.MakeMenuSelection();  // Select one of the three top menu choices: Freq, Presets, 1st Cal
  busy_wait_ms(200);                                // Crude debounce
  switch (display.menuIndex) {
    case FREQMENU:
      display.frequencyMenuOption();
      break;

    case PRESETSMENU:                        //Preset frequencies by band - set in .ino file, variable: presetFrequencies[0][2];
      display.whichBandOption = display.SelectBand();            // Select the band to be used
      submenuIndex = 0;
      display.ProcessPresets(display.whichBandOption, submenuIndex);         // Select a preselected frequency.  This should return a frequency???
      display.menuIndex = FREQMENU;                         // When done, start over...
      display.ShowMainDisplay();
      display.ShowSubmenuData(swr.ReadSWRValue(), dds.currentFrequency);
      dds.SendFrequency(dds.currentFrequency);
      break;

    case CALIBRATEMENU:             //Run first time Calibration routine  Takes longer - use to initialize band edge parameters
      display.EraseBelowMenu();                             // Clear work area
      //DoNewCalibrate2();
      display.DoFirstCalibrate();
      //display.EraseBelowMenu();
      //MyDelay(200L);
      //display.ShowMainDisplay();
      //display.ShowSubmenuData(0.0, dds.currentFrequency);
      break;

    default:
      break;
  } //switch (menuIndex)

}  // while(1)  (end of loop)

  return 0;
}
