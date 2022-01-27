#pragma once
#include <stdint.h>
#include "pico/stdlib.h"
#include "Adafruit_ILI9341.h"
#include "Arduino.h"
#include <string>
#include "DDS.h"
#include "Data.h"

#include "../Adafruit-GFX-Library/Fonts/FreeSerif9pt7b.h"
//#include "../Adafruit-GFX-Library/Fonts/FreeSerif12pt7b.h"
//#include "../Adafruit-GFX-Library/Fonts/FreeSerif24pt7b.h"

#define YAXISSTART                  55                  // For graphing purposes
#define YAXISEND                    210
#define XAXISSTART                  25
#define XAXISEND                    315
#define PIXELWIDTH                  320                 // Display limits
#define PIXELHEIGHT                 240
#define TEXTLINESPACING             20                  // Pixel spacing per line with text size = 2

//extern long bandLimitPositionCounts[3][2];

//class DisplayManagement;

class GraphPlot {

    public:
    int xIncrement, yIncrement;
    int xOld;
    //int iMax;
    //int tempCurrentPosition[10];
    //float tempSWR[500];
    //long  SWRMinPosition;

    Adafruit_ILI9341 & tft;
    DDS & dds;
    Data & data;

    GraphPlot(Adafruit_ILI9341 & tft, DDS & dds, Data & data);

    void GraphAxis(int whichBandOption);

    void PlotNewStartingFrequency(int whichBandOption);

    void PlotSWRValueNew(int whichBandOption, int iMax, long tempCurrentPosition[], float tempSWR[], long SWRMinPosition);
};