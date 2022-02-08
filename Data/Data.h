#pragma once
#include <stdint.h>



//  This class is intended to manage various frequency and position related constants and variables.
//  The single object will be referenced by most or maybe all of the other class objects.

class Data {

public:
// Define these frequencies for your licensing authority
const uint32_t LOWEND40M =   7000000;
const uint32_t HIGHEND40M =  7300000; 
const uint32_t LOWEND30M =  10100000;
const uint32_t HIGHEND30M = 10150000;
const uint32_t LOWEND20M =  14000000;
const uint32_t HIGHEND20M = 14350000;


// Initial Band edge counts from Calibrate routine.
// These are initial guesses and will be overwritten by the Calibration algorithm.

long bandLimitPositionCounts[3][2] = {
  {  4083L,  5589L},
  {13693L, 13762L},
  {18319L, 18691L}
};

const uint32_t presetFrequencies[3][6] =
{
  { 7030000L,  7040000L,  7100000L,  7150000L,  7250000L,  7285000L},   // 40M
  {10106000L, 10116000L, 10120000L, 10130000L, 10140000L, 10145000L},   // 30M
  {14030000L, 14060000L, 14100000L, 14200000L, 14250000L, 14285000L}    // 20M
};

const uint32_t bandEdges[3][2] = {   // Band edges in Hz
  {LOWEND40M, HIGHEND40M},
  {LOWEND30M, HIGHEND30M},
  {LOWEND20M, HIGHEND20M}
};

//  This should be made variable length arrays.
float countPerHertz[3];
float hertzPerStepperUnitVVC[3];  // Voltage Variable Cap
int currentBand;



Data();

void computeSlopes();

};