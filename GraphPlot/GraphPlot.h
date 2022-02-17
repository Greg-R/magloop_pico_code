#pragma once
#include <stdint.h>
#include "pico/stdlib.h"
#include "Adafruit_ILI9341.h"
#include "Arduino.h"
#include <string>
#include "DDS.h"
#include "Data.h"

#include "../Adafruit-GFX-Library/Fonts/FreeSerif9pt7b.h"


class GraphPlot {

public:
const int YAXISSTART       =           55;             // For graphing purposes
const int YAXISEND           =         170;
const int XAXISSTART         =         25;
const int XAXISEND           =         315;
const int PIXELWIDTH         =         320;                 // Display limits
const int PIXELHEIGHT        =         240;
const int TEXTLINESPACING    =         20;                  // Pixel spacing per line with text size = 2
   
    int xIncrement, yIncrement;
    int xOld;

    Adafruit_ILI9341 & tft;
    DDS & dds;
    Data & data;

    GraphPlot(Adafruit_ILI9341 & tft, DDS & dds, Data & data);

    void GraphAxis(int whichBandOption);

    void PlotNewStartingFrequency(int whichBandOption);

    void PlotSWRValueNew(int whichBandOption, int iMax, long tempCurrentPosition[], float tempSWR[], long SWRMinPosition);
};