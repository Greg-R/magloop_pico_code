#pragma once
#include <stdint.h>
//#include <string>
#include "pico/stdlib.h"
#include "Adafruit_ILI9341.h"
#include "Arduino.h"
#include "Calibrate.h"
#include "DDS.h"
#include "SWR.h"
#include "AutoTune.h"
#include "StepperManagement.h"
#include "Buttons.h"

#define PRESETSPERBAND   6
#define PIXELWIDTH 320
#define AUTOTUNE   7    // Auto-tune Button
#define INCREMENTPAD  22  // Used to display increment cursor
#define MENUBUTTON3  8 // This is probably not correct!
#define MENUBUTTON1  4 // Also probably not correct!
#define MAXMENUES  10  // Couldn't find in Magloop.h ?

class Calibrate;

class Buttons;

class AutoTune;

class DisplayManagement {

public:

Adafruit_ILI9341 & tft;
Calibrate & calibrate;
DDS & dds;
SWR & swr;
AutoTune & autotune;
AccelStepper & stepper;
StepperManagement & steppermanage;
Buttons & buttons;
int whichBandOption;
int quickCalFlag;
long currentFrequency;
long presetFrequencies[10][PRESETSPERBAND];
float SWRValue;
float readSWRValue;
int currPosition;
long bandLimitPositionCounts[10][2];
long bandEdges[10][2];
float hertzPerStepperUnitVVC[10];
volatile int menuEncoderMovement;
volatile int digitEncoderMovement;
int currentBand;
char *menuOptions[100];
volatile long frequencyEncoderMovement;
int menuIndex;

DisplayManagement(Adafruit_ILI9341 & tft, Calibrate & calibrate, DDS & dds, SWR & swr, AccelStepper & stepper, AutoTune & autotune, StepperManagement & steppermanage, Buttons & buttons);

void frequencyMenuOption();

void ChangeFrequency(int bandIndex);

int MakeMenuSelection();

int SelectBand();

void EraseBelowMenu();

void ErasePage();

int ShowMainDisplay(int whichMenuPage, float SWR);

void ShowSubmenuData(float SWR);

void UpdateFrequency();

void UpdateSWR(float SWR, char msg[]);

void updateMessage(char messageToPrint[]);

};