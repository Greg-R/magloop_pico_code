#pragma once
#include <stdint.h>
#include <string>
#include "pico/stdlib.h"
#include "Adafruit_ILI9341.h"
#include "Arduino.h"
//#include "Calibrate.h"
#include "DDS.h"
#include "SWR.h"
#include "AutoTune.h"
#include "StepperManagement.h"
//#include "Buttons.h"
#include "EEPROM.h"

#include "../Adafruit-GFX-Library/Fonts/FreeSerif9pt7b.h"
#include "../Adafruit-GFX-Library/Fonts/FreeSerif12pt7b.h"
#include "../Adafruit-GFX-Library/Fonts/FreeSerif24pt7b.h"

#define PRESETSPERBAND   6
#define PIXELWIDTH 320
#define AUTOTUNE   7    // Auto-tune Button
#define INCREMENTPAD  22  // Used to display increment cursor
#define MENUBUTTON3  8 // This is probably not correct!
#define MENUBUTTON1  4 // Also probably not correct!
#define MAXMENUES  3  // The menu selections are: Freq, Presets, 1st Cal

#define MAXBANDS   3 
#define MAXSWITCH 10
#define TARGETMAXSWR 5.5  // Originally set to 2.5, increased for debugging.
#define TEXTLINESPACING 20
#define MENUENCODERSWITCH 19
#define FREQUENCYENCODERSWITCH 20
#define FREQMENU  0  // Menuing indexes

extern int menuEncoderMovement;
extern int frequencyEncoderMovement;
extern int digitEncoderMovement;

extern const uint32_t presetFrequencies[3][6];
extern long bandLimitPositionCounts[3][2];  // This array is written to during calibration; it can't be const.
extern const uint32_t bandEdges[3][2];

//class Calibrate;

//class Buttons;

class AutoTune;

class DisplayManagement {

public:

Adafruit_ILI9341 & tft;
//Calibrate & calibrate;
DDS & dds;
SWR & swr;
AutoTune & autotune;
StepperManagement & stepper;
EEPROM & eeprom;
//Buttons & buttons;
int whichBandOption;
int quickCalFlag;
long currentFrequency;
//long presetFrequencies[10][PRESETSPERBAND];
float SWRValue;
float SWRcurrent;
float readSWRValue;
int currPosition;
//long bandLimitPositionCounts[10][2];
//long bandEdges[10][2];
float hertzPerStepperUnitVVC[10];
//volatile int menuEncoderMovement;
//volatile int digitEncoderMovement;
int currentBand;
//char *menuOptions[100];
//volatile int frequencyEncoderMovement;
int menuIndex;
int submenuIndex;
float minSWRAuto;
int SWRFinalPosition;
long SWRMinPosition;
volatile int menuEncoderState;
//volatile int menuEncoderMovement;
//volatile int frequencyEncoderMovement;
const std::string menuOptions[3] = {" Freq ", " Presets ", " 1st Cal"};

DisplayManagement(Adafruit_ILI9341 & tft, DDS & dds, SWR & swr, AutoTune & autotune, StepperManagement & stepper, EEPROM & eeprom);

void Splash(std::string version, std::string releaseDate);

void frequencyMenuOption();

void ChangeFrequency(int bandIndex);

int MakeMenuSelection();

int SelectBand();

void EraseBelowMenu();

void ErasePage();

int ShowMainDisplay(int whichMenuPage, float SWR);

void ShowSubmenuData(float SWR, int currentFrequency);

void UpdateFrequency(int frequency);

//void UpdateSWR(float SWR, char msg[]);
void UpdateSWR(float SWR, std::string msg);

void updateMessage(std::string messageToPrint);

// The following 4 methods were consolidated from "Calibrate".
void DoNewCalibrate2();

void DoFirstCalibrate();

void CalSWR();

void DoSingleBandCalibrate(int whichBandOption);

// The following 3 methods were consolidated from "Buttons":
void executeButton1();

void executeButton3();

void quickCalISR();

// The following 3 methods were consolidated from "Presets":
void ProcessPresets(int whichBandOption, int submenuIndex);

void RestorePreviousPresetChoice(int submenuIndex, int whichBandOption);

void HighlightNewPresetChoice(int submenuIndex, int whichBandOption);

};