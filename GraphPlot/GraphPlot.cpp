

#include "GraphPlot.h"

extern long bandLimitPositionCounts[3][2];

GraphPlot::GraphPlot(Adafruit_ILI9341 & tft, DDS & dds, Data & data): tft(tft), dds(dds), data(data) {}

/*****
  Purpose: To display the axes for a graph
  Paramter list:
    int whichBandOption     // The band being used
  Return value:
    void
*****/
void GraphPlot::GraphAxis(int whichBandOption) //al modified 9-8-19
{
  tft.setTextSize(1);
  tft.setFont(&FreeSerif9pt7b);
  char buff[10];
  int chunks, tickCount;
  int yTick, xTick, yDotIncrement, xDotIncrement;
  int tcolor, bcolor;
  int i, k;
  float freqCount, freqEnd, pip;
  switch (whichBandOption) {
    case 0:
      freqCount = 7.0;
      freqEnd = 7.3;
      pip = 0.1;
      chunks = 3;
      xDotIncrement = 12;
      xIncrement = (XAXISEND - XAXISSTART) / chunks;
      break;

    case 1:
      freqCount = 10.1;
      freqEnd = 10.15;
      pip = 0.02;
      chunks = 3;
      xDotIncrement = 20;
      break;

    case 2:
      freqCount = 14.0;
      freqEnd = 14.35;
      pip = 0.10;
      chunks = 3;
      xDotIncrement = 12;
      xIncrement = (XAXISEND * (.3 / .35) - XAXISSTART) / chunks;
      break;
  }
  tcolor = ILI9341_YELLOW;
  bcolor = ILI9341_BLACK;
  tickCount = 3;
  tft.fillRect(0, 47, PIXELWIDTH, PIXELHEIGHT, ILI9341_BLACK);
  tft.drawLine(XAXISSTART, YAXISSTART, XAXISSTART, YAXISEND, ILI9341_DARKGREY);    // Solid Y axis Left side  OK 9-8-19
  tft.drawLine(XAXISSTART, YAXISEND + 3,   XAXISEND,   YAXISEND + 3, ILI9341_DARKGREY); // X axis  OK 9-8-19
  tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);              // Lable X axis   OK 9-8-19
  tft.setCursor((XAXISEND - XAXISSTART) / 2 - 12, YAXISEND + 20 );
  tft.print(" Mhz");
  yIncrement = (YAXISEND - YAXISSTART) / 3;                     // Spacing for graph tick marks
  yTick = YAXISSTART + 5;                                       // on Y axis
  tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
  for (i = YAXISSTART; i < YAXISEND; i += yIncrement, yTick += yIncrement) {
    for (k = XAXISSTART + 10; k < XAXISEND; k += xDotIncrement) {
      tft.drawPixel(k, yTick, ILI9341_DARKGREY);    // Horizontal dotted axis OK 9-8-19
    }
    tft.setCursor(0, yTick - 1); //Print y axis SWR labels
    tft.print(tickCount--);
    tft.print(".0");
  }

  xIncrement = (XAXISEND - XAXISSTART) / chunks;   // Spacing for graph tick marks
  xTick = XAXISSTART - 10;                                       // on X axis

  for (i = 0; i < chunks + 1; i++, xTick += xIncrement) {
    tft.setCursor(XAXISSTART - 25 + i * xIncrement, YAXISEND + 20);
    if (freqCount > freqEnd) {
      freqCount = freqEnd;
    }
    if (whichBandOption == 2) {    // 20M
      if (freqCount < 14.15 || freqCount < 14.25 || freqCount < 14.31 || freqCount < 14.4) {
        if (freqCount > 14.35) {
          freqCount = 14.35;
        //  dtostrf(freqCount, 5, 2, buff);
        std::sprintf(buff, "%3.1f", freqCount);
        } else {
        //  dtostrf(freqCount, 4, 1, buff);
        std::sprintf(buff, "%3.1f", freqCount);
        }
        tft.print(buff);  //Print 20M Frequency Labels OK 9-8-19
      }
    } else {
      tft.print(freqCount); //Print 20M Frequency Labels OK 9-8-19
    }
    freqCount += pip;
  }
  for (k = XAXISSTART; k <= XAXISEND; k += xIncrement / 2) {      // Draw horizontal dotted grid lines
    for (i = YAXISSTART; i < YAXISEND; i += 9) {
      tft.drawPixel(k, i, ILI9341_DARKGREY);    // Print Vertical Y dotted lines OK 9-8-19
    }
  }
}

/*****
  Purpose: Plot frequency given by user

  Argument list:
    int whichBandOption       the selected band

  Return value:
    int                       the menu selected
*****/
void GraphPlot::PlotNewStartingFrequency(int whichBandOption)
{
  int delta, tickCount;
  int x, y;
  long highEnd, lowEnd, midPoint;
  float freqCount;
  float HzPerPix;
  switch (whichBandOption) {
    case 0:
      freqCount = 7.0;
      highEnd = 7300000L;
      lowEnd  = 7000000L;
      HzPerPix = float(highEnd - lowEnd) / float(XAXISEND - XAXISSTART);
      x = 25 + float(dds.currentFrequency - lowEnd) / HzPerPix;
      
      //Serial.print("x=  "); Serial.println(x);
      break;
    case 1:
      freqCount = 10.1;
      highEnd = 10150000L;
      lowEnd  = 10100000L;
      HzPerPix = float(highEnd - lowEnd) / float(XAXISEND - XAXISSTART);
      x = 25 + float(dds.currentFrequency - lowEnd) / HzPerPix;
      break;
    case 2:
      freqCount = 14.0;
      highEnd = 14350000L;
      lowEnd  = 14000000L;
      HzPerPix = float(highEnd - lowEnd) / float(XAXISEND - XAXISSTART);
      x = 25 + float(dds.currentFrequency - lowEnd) / HzPerPix;
      break;
  }

  tft.drawLine(xOld, YAXISSTART, xOld, YAXISEND, ILI9341_BLACK);
  tft.drawLine(x, YAXISSTART, x, YAXISEND, ILI9341_YELLOW);   // Y axis
//  display.UpdateFrequency(dds.currentFrequency);  Move to main???  Or is this redundant???
  xOld = x;
}


/*****
  Purpose: Displays the Point for measured SWR
  This method should be moved to the GraphPlot class.
  Parameter list:
    int whichBandOption     // The band being used
    float swr

  Return value:
    void
*****/
void GraphPlot::PlotSWRValueNew(int whichBandOption, int iMax, long tempCurrentPosition[], float tempSWR[], long SWRMinPosition)
{
  float stepsPerPix;
  int pixPerSWRUnit;
  float HzPerStep;
  float HzPerPix;
  float currentFrequencyDiff;
  float plotFreq;
  long freqStart;
  long freqEnd;
  // ShowSubmenuData(minSWRAuto);  TEMPORARILY COMMENTED.  This is a DisplayManagement method???
  switch (whichBandOption) {
    case 0:
      freqStart = 7000000;
      freqEnd = 7300000;
      break;

    case 1:
      freqStart = 10100000;
      freqEnd = 10150000;

    case 2:
      freqStart = 14000000;
      freqEnd = 14350000;
      break;

  }
  // This for loop plots the data to the axes.  The data is in the array tempSWR[i].
  for (int i = 0; i < iMax; i++) {
    if (tempCurrentPosition[i] > 0  and tempSWR[i] < 3) {
      HzPerStep = (freqEnd - freqStart) / (float(data.bandLimitPositionCounts[whichBandOption][1] - data.bandLimitPositionCounts[whichBandOption][0]));
      currentFrequencyDiff = float(tempCurrentPosition[i] - SWRMinPosition) * HzPerStep;
      plotFreq = (dds.currentFrequency + currentFrequencyDiff);
      HzPerPix = float(freqEnd - freqStart) / float(XAXISEND - XAXISSTART);
      stepsPerPix = float(data.bandLimitPositionCounts[whichBandOption][1] - data.bandLimitPositionCounts[whichBandOption][0]) / (XAXISEND - XAXISSTART);
      pixPerSWRUnit = float(YAXISEND - YAXISSTART) / 3;
      int xposition = 27 + float(plotFreq - freqStart) / HzPerPix;
      int yposition = YAXISSTART + (3 - tempSWR[i]) * pixPerSWRUnit;
      tft.fillCircle(xposition, yposition, 1, ILI9341_YELLOW);
#ifdef DEBUG
      Serial.print("bandLimitPositionCounts[whichBandOption][1]=  "); Serial.println(bandLimitPositionCounts[whichBandOption][1]);
      Serial.print("bandLimitPositionCounts[whichBandOption][0]=  "); Serial.println(bandLimitPositionCounts[whichBandOption][0]);
      Serial.print("currentFrequencyDiff=  "); Serial.println(currentFrequencyDiff);
      Serial.print("HzPerStep=  "); Serial.println(HzPerStep);
      Serial.print("stepsPerPix=  "); Serial.println(stepsPerPix);
      Serial.print("plotFreq=  "); Serial.println(plotFreq);
      Serial.print("pixPerHz=  "); Serial.println(HzPerPix);
      Serial.print("tempCurrentPosition[i]=  "); Serial.println(tempCurrentPosition[i]);
      Serial.print("xposition=  "); Serial.println(xposition);
      Serial.print("yposition=  "); Serial.println(yposition);
#endif
    }
  }
//  PlotNewStartingFrequency(whichBandOption);  TEMPORARILY COMMENTED
}