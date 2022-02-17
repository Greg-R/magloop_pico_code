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
#include "GraphPlot.h"
#include "Data.h"

#include "../Adafruit-GFX-Library/Fonts/FreeSerif9pt7b.h"
#include "../Adafruit-GFX-Library/Fonts/FreeSerif12pt7b.h"
#include "../Adafruit-GFX-Library/Fonts/FreeSerif24pt7b.h"

#define PRESETSPERBAND   6
#define PIXELWIDTH 320
//#define AUTOTUNE   7    // Auto-tune Button
extern const int autotunebutton;
#define INCREMENTPAD  22  // Used to display increment cursor
#define MENUBUTTON3  4 //  Full Calibrate using band edges for faster calibration.
#define MENUBUTTON1  8 // Band cal
#define MAXMENUES  3  // The menu selections are: Freq, Presets, 1st Cal

#define MAXBANDS   3 
#define MAXSWITCH 10
#define TARGETMAXSWR 5.5  // Originally set to 2.5, increased for debugging.
#define TEXTLINESPACING 20
//#define MENUENCODERSWITCH 19
extern const int enterbutton;
//#define FREQUENCYENCODERSWITCH 20
extern const int exitbutton;
#define FREQMENU  0  // Menuing indexes

//#define FASTMOVESPEED               1000
//#define NORMALMOVESPEED             500  // Was 100, changed to speed up AutoTune.
#define MAXNUMREADINGS              500
#define PIXELHEIGHT                 240
#define ACCURACYBUTTON         6


//extern const uint32_t presetFrequencies[3][6];
//extern long bandLimitPositionCounts[3][2];  // This array is written to during calibration; it can't be const.
//extern const uint32_t bandEdges[3][2];

//class Calibrate;

//class Buttons;

//class GraphPlot;

class DisplayManagement : public GraphPlot {

public:

Adafruit_ILI9341 & tft;
//Calibrate & calibrate;
DDS & dds;
SWR & swr;
//AutoTune & autotune;
StepperManagement & stepper;
EEPROM & eeprom;
//GraphPlot & graphplot;
Data & data;
//Buttons & buttons;
int whichBandOption;
//int quickCalFlag;
//long frequency;
//long presetFrequencies[10][PRESETSPERBAND];
float SWRValue;
float SWRcurrent;
float readSWRValue;
int position;
int positionTemp;
//long bandLimitPositionCounts[10][2];
//long bandEdges[10][2];
//float hertzPerStepperUnitVVC[10];
//volatile int menuEncoderMovement;
//volatile int digitEncoderMovement;
int currentBand;  // Is this used???
//char *menuOptions[100];
//volatile int frequencyEncoderMovement;
int menuIndex;
int submenuIndex;

int SWRFinalPosition;
volatile int menuEncoderState;
//volatile int menuEncoderMovement;
//volatile int frequencyEncoderMovement;
const std::string menuOptions[3] = {" Freq ", " Presets ", " Calibrate"};
std::string band[3] = {"40M", "30M", "20M"};  // Make this a global???

int stepperDirectionOld;
uint32_t stepperDistanceOld;
int iMax;
float tempSWR[500];
long tempCurrentPosition[500];
long  SWRMinPosition;
float minSWRAuto;
float minSWR;
//int whichBandOption;
//long  SWRFinalPosition;
enum class State {state0, state1, state2, state3};  // Used to move between states in state machines.

DisplayManagement(Adafruit_ILI9341 & tft, DDS & dds, SWR & swr, StepperManagement & stepper, EEPROM & eeprom, Data & data);

void Splash(std::string version, std::string releaseDate);

void frequencyMenuOption();

int manualTune();

long ChangeFrequency(int bandIndex, long frequency);

int MakeMenuSelection(int index);

int SelectBand(const std::string bands[3]);

void EraseBelowMenu();

void ErasePage();

void ShowMainDisplay(int whichMenuPage);

void ShowSubmenuData(float SWR, int currentFrequency);

void UpdateFrequency(int frequency);

//void UpdateSWR(float SWR, char msg[]);
void UpdateSWR(float SWR, std::string msg);

void updateMessageTop(std::string messageToPrint);

void updateMessageBottom(std::string messageToPrint);

// The following 4 methods were consolidated from "Calibrate".
void DoNewCalibrate2();

void DoFirstCalibrate();

void CalSWR();

void DoSingleBandCalibrate(int whichBandOption);

// The following 3 methods were consolidated from "Buttons":
//void executeButton1();

//void executeButton3();

//void quickCalISR();

// The following 3 methods were consolidated from "Presets":
void ProcessPresets();

int SelectPreset();

void RestorePreviousPresetChoice(int submenuIndex, int whichBandOption);

void HighlightNewPresetChoice(int submenuIndex, int whichBandOption);

float AutoTuneSWR();

float AutoTuneSWRQuick();

void ManualFrequencyControl(int whichBandOption);

void ManualStepperControl();

int DetectMaxSwitch();

};