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
#include "DDS/DDS.h"
#include "SWR/SWR.h"
#include "AutoTune/AutoTune.h"
#include "Rotary/Rotary.h"
#include "EEPROM/EEPROM.h"
//#include "hardware/flash.h"
//#include "hardware/sync.h"

//#include "Adafruit-GFX-Library/gfxfont.h"
//#include "Adafruit-GFX-Library/Fonts/FreeSerif24pt7b.h"
//#include "Adafruit-GFX-Library/Fonts/FreeSerif9pt7b.h"
//#include "Adafruit-GFX-Library/Fonts/FreeSerif12pt7b.h"

#define PIXELWIDTH 320  // Display limits
#define PIXELHEIGHT 240 // These are the post-rotation dimensions.
const std::string version = "1.01";
const std::string releaseDate = "3-15-21";

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

#define PRESETSMENU 1
#define CALIBRATEMENU 2

//#define FLASH_TARGET_OFFSET (256 * 1024)

// +12V and +5V power switch GPIO:
#define POWER_SWITCH 28

#define PRESETSPERBAND              6                   // Allow this many preset frequencies on each band
#define MAXBANDS                    3                   // Can only process this many frequency bands

// Initial Band edge counts from Calibrate routine.
// These are initial guesses and will be overwritten by the Calibration algorithm.
long bandLimitPositionCounts[3][2] = {
  {  4083L,  5589L},
  {13693L, 13762L},
  {18319L, 18691L}
};

extern const uint32_t presetFrequencies[3][6] =
{
  { 7030000L,  7040000L,  7100000L,  7150000L,  7250000L,  7285000L},   // 40M
  {10106000L, 10116000L, 10120000L, 10130000L, 10140000L, 10145000L},   // 30M
  {14030000L, 14060000L, 14100000L, 14200000L, 14250000L, 14285000L}    // 20M
};

#define LOWEND40M                   7000000L            // Define these frequencies for your licensing authority
#define HIGHEND40M                  7300000L            // The 'L' helps document that these are long data types
#define LOWEND30M                  10100000L
#define HIGHEND30M                 10150000L
#define LOWEND20M                  14000000L
#define HIGHEND20M                 14350000L

extern const uint32_t bandEdges[3][2] = {   // Band edges in Hz
  {LOWEND40M, HIGHEND40M},
  {LOWEND30M, HIGHEND30M},
  {LOWEND20M, HIGHEND20M}
};

// The Splash function from the Mag Loop Arduino .ino file.

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
/*
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
*/

int currentBand;      // Should be 40, 30, or 20
int currentFrequency;
int whichBandOption;
float countPerHertz[3];
float hertzPerStepperUnitAir[3];

volatile uint8_t result;
//volatile uint8_t resultFrequency;
volatile uint32_t countEncoder;
Rotary menuEncoder = Rotary(18, 17);
Rotary frequencyEncoder = Rotary(22, 21);
int menuEncoderMovement;
int frequencyEncoderMovement;
int digitEncoderMovement;

void encoderCallback(uint gpio, uint32_t events) {
if((gpio == 17) || (gpio == 18)) {
  result = menuEncoder.process();
  if (result != 0) {
    switch (result) {
      case DIR_CW:
        menuEncoderMovement = 1;
        digitEncoderMovement = 1;
     //   digitEncoderMovement = 1;
        break;
      case DIR_CCW:
        menuEncoderMovement = -1;
        digitEncoderMovement = -1;
        break;
    }
  }
}
//else if(gpio == 18) result = menuEncoder.process();
else if((gpio == 21) || (gpio == 22)) {
result = frequencyEncoder.process();
  if (result != 0) {
    switch (result) {
      case DIR_CW:
        frequencyEncoderMovement = 1;
     //   digitEncoderMovement = 1;
        break;
      case DIR_CCW:
        frequencyEncoderMovement = -1;
     //   digitEncoderMovement = -1;
        break;
    }
  }
}
//else if(gpio == 22) result = frequencyEncoder.process();
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
  //  Run the same Splash function as in the Mag Loop Controller project.
  
  // Test a GFX graphics primitive by drawing a border:
  //tft.drawRect(1, 1, 318, 238, ILI9341_WHITE);

  uint32_t eeprom_data;
  EEPROM eeprom = EEPROM();
  //  Use this method one time only and then comment out!
  //eeprom.WriteDefaultEEPROMValues();
  //  Read the position counts and presets into the EEPROM object's buffer.
  eeprom.ReadEEPROMValuesToBuffer();
  //  Overwrite the position counts and preset frequencies:
  eeprom.ReadPositionCounts();

  //eeprom.initialize();  //  Set the buffer to all zeros.
  
  //  EEPROM test code.
  const uint32_t *flash_target_contents = (const uint32_t *) (XIP_BASE + FLASH_TARGET_OFFSET);
  eeprom_data = flash_target_contents[1];
  tft.setTextSize(2);
  tft.setCursor(80, 40);
  //tft.print(eeprom_data);
  tft.print(bandLimitPositionCounts[0][0]);
  eeprom_data = flash_target_contents[2];
  tft.setCursor(80, 60);
  //tft.print(eeprom_data);
  tft.print(bandLimitPositionCounts[0][1]);
  eeprom_data = flash_target_contents[3];
  tft.setCursor(80, 80);
  //tft.print(eeprom_data);
  tft.print(bandLimitPositionCounts[1][0]);
  tft.setCursor(80, 100);
   tft.print(bandLimitPositionCounts[1][1]);
  eeprom_data = flash_target_contents[2];
  tft.setCursor(80, 120);
  //tft.print(eeprom_data);
  tft.print(bandLimitPositionCounts[2][0]);
  eeprom_data = flash_target_contents[3];
  tft.setCursor(80, 140);
  //tft.print(eeprom_data);
  tft.print(bandLimitPositionCounts[2][1]);
  

//  Instantiate the Stepper Manager: 
StepperManagement stepper = StepperManagement(AccelStepper::MotorInterfaceType::DRIVER, STEPPERPUL, STEPPERDIR);

//AccelStepper stepper =AccelStepper(1, STEPPERPUL, STEPPERDIR);

//  Examples of Stepper control functions:
//  stepper.ResetStepperToZero();
//  steppermanage.setCurrentPosition(0);  //  Sets max speed to zero!
//  steppermanage.setMaxSpeed(500);
//  steppermanage.setAcceleration(110);
//  steppermanage.runToNewPosition(2675);
//  steppermanage.disableOutputs();

//  Next test the DDS.
  DDS dds = DDS(DDS_RST, DDS_DATA, DDS_FQ_UD, WLCK);
  dds.DDSWakeUp();
  //dds.SendFrequency(0);
  
// Instantiate SWR object.
  SWR swr = SWR(stepper, tft);
//  Now measure the ADC offsets before the DDS is active.
//  swr.ReadADCoffsets();
//  dds.SendFrequency(8045000);
//  dds.SendFrequency(8045000);
  
AutoTune autotune = AutoTune(swr, tft, stepper);
DisplayManagement display = DisplayManagement(tft, dds, swr, autotune, stepper, eeprom);
//Calibrate calibrate = Calibrate(display, stepper, tft, dds, swr, autotune);
//Presets presets = Presets(tft, stepper, dds, autotune, swr, display);
//Buttons buttons = Buttons(display, presets, dds, calibrate);
display.Splash(version, releaseDate);

//tft.fillScreen(ILI9341_BLACK);
//tft.setTextSize(4);
//tft.setCursor(80, 40);
//tft.print(presetFrequencies[0][0]);
//autotune.AutoTuneSWRQuick();

float VSWR, VSWROld, SWRcurrent;
//VSWR = swr.ReadSWRValue();
//  tft.fillScreen(ILI9341_BLACK);
//  tft.setCursor(50, 10);
//  tft.print("TUNED SWR:");
//  tft.setCursor(110, 50);
//  tft.print(VSWR, 3);
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
//dds.SendFrequency(0);
//gpio_put(POWER_SWITCH, 0);

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
//  Set up the Menu and Frequency encoders:
menuEncoder.begin(true, false);
frequencyEncoder.begin(true, false);
// Encoder interrupts:
uint32_t events = 0x0000000C;  // Rising and falling edges.
countEncoder = 0;
gpio_set_irq_enabled_with_callback(18, events, 1, &encoderCallback);
gpio_set_irq_enabled_with_callback(17, events, 1, &encoderCallback);
gpio_set_irq_enabled_with_callback(21, events, 1, &encoderCallback);
gpio_set_irq_enabled_with_callback(22, events, 1, &encoderCallback);

// Test the rotary encoders:
/*
tft.fillScreen(ILI9341_BLACK);
tft.setTextSize(2);
tft.setCursor(20, 90);
while(1) {
    if(result) {
      tft.fillScreen(ILI9341_BLACK);
      tft.setCursor(20, 90);
      tft.print(countEncoder);
}}
*/

// Setup follows.
// The default band is read from Flash.
currentBand = eeprom.ReadCurrentBand();

  switch (currentBand) {              // Set the frequency default as 1st preset frequency
    case 40:
      currentFrequency = presetFrequencies[0][2];
      //currentFrequency = 7150000L;
      break;
    case 30:
      currentFrequency = presetFrequencies[1][0];
      break;
    case 20:
      currentFrequency = presetFrequencies[2][0];
      break;
    default:
      break;
  }
  dds.SendFrequency(currentFrequency);    // Set the DDS
  SWRcurrent = swr.ReadSWRValue();
  display.UpdateSWR(SWRcurrent, "??");
  VSWROld = SWRcurrent;
  //  Read position counts out of flash and overwrite default values:
  eeprom.ReadPositionCounts();
  display.menuIndex = FREQMENU;
  display.ShowMainDisplay(0, SWRcurrent);       // Draws top menu line
  display.ShowSubmenuData(SWRcurrent, currentFrequency);          // Draws SWR and Freq info
  busy_wait_ms(100);                      // Let DDS stabilize
  whichBandOption = 0;

 // display.EraseBelowMenu();    // Clear work area
      
//  display.DoFirstCalibrate();
  display.EraseBelowMenu();

SWRcurrent = swr.ReadSWRValue();

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
  //swr.ReadSWRValue();  // Does this do anything???
  switch (display.menuIndex) {
    case FREQMENU:
      display.frequencyMenuOption();
      break;

    case PRESETSMENU:                        //Preset frequencies by band - set in .ino file, variable: presetFrequencies[0][2];
      display.whichBandOption = display.SelectBand();            // Select the band to be used
      submenuIndex = 0;
      display.ProcessPresets(display.whichBandOption, submenuIndex);         // Select a preselected frequency
      display.menuIndex = FREQMENU;                         // When done, start over...
      display.ShowMainDisplay(display.menuIndex, SWRcurrent);
      display.ShowSubmenuData(SWRcurrent, display.currentFrequency);
      dds.SendFrequency(display.currentFrequency);
      SWRcurrent = swr.ReadSWRValue();
      break;

    case CALIBRATEMENU:             //Run first time Calibration routine  Takes longer - use to initialize band edge parameters
      display.EraseBelowMenu();                             // Clear work area
      //DoNewCalibrate2();
      display.DoFirstCalibrate();
      display.EraseBelowMenu();
      //MyDelay(200L);
      display.ShowMainDisplay(0, 0.0);
      display.ShowSubmenuData(0.0, display.currentFrequency);
      break;

    default:
      break;
  } //switch (menuIndex)


}


  return 0;
}
