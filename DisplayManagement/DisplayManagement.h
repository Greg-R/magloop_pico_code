#pragma once
#include <stdint.h>
#include "pico/stdlib.h"
#include "../Adafruit_ILI9341/Adafruit_ILI9341.h"
#include "Arduino.h"
#include "Calibrate.h"

#define PRESETSPERBAND   6

class DisplayManagement {

public:

Adafruit_ILI9341 & tft;
Calibrate & calibrate;
int whichBandOption;
int quickCalFlag;
long currentFrequency;
long presetFrequencies[10][PRESETSPERBAND];


DisplayManagement(Adafruit_ILI9341 & tft, Calibrate & calibrate);

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