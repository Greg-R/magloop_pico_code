#pragma once
#include <stdint.h>
#include <string>


//  This class is intended to manage various frequency and position related constants and variables.
//  The single object will be referenced by most or maybe all of the other class objects.

class Data {

public:
// Set these frequencies for your band limits and desired presets.
/*
std::string bands[3] = {"30M", "20M", "15M"};
const uint32_t LOWEND40M =  10100000;
const uint32_t HIGHEND40M = 10150000; 
const uint32_t LOWEND30M =  14000000;
const uint32_t HIGHEND30M = 14350000;
const uint32_t LOWEND20M =  21000000;
const uint32_t HIGHEND20M = 21450000;
const uint32_t presetFrequencies[3][6] =
{
  {10106000L, 10116000L, 10120000L, 10130000L, 10140000L, 10145000L},   // 30M
  {14030000L, 14060000L, 14100000L, 14200000L, 14250000L, 14285000L},   // 20M
  {21030000L, 21040000L, 21100000L, 21150000L, 21250000L, 21285000L},   // 15M
};
*/
// Bands used:

std::string bands[3] = {"40M", "30M", "20M"};
const uint32_t LOWEND40M =   7000000;
const uint32_t HIGHEND40M =  7300000; 
const uint32_t LOWEND30M =  10100000;
const uint32_t HIGHEND30M = 10150000;
const uint32_t LOWEND20M =  14000000;
const uint32_t HIGHEND20M = 14350000;
const uint32_t presetFrequencies[3][6] =
{
  { 7030000L,  7040000L,  7100000L,  7150000L,  7250000L,  7285000L},   // 40M
  {10106000L, 10116000L, 10120000L, 10130000L, 10140000L, 10145000L},   // 30M
  {14030000L, 14060000L, 14100000L, 14200000L, 14250000L, 14285000L}    // 20M
};


long bandLimitPositionCounts[3][2];

const uint32_t bandEdges[3][2] = {   // Band edges in Hz
  {LOWEND40M, HIGHEND40M},
  {LOWEND30M, HIGHEND30M},
  {LOWEND20M, HIGHEND20M}
};

//  This should be made variable length arrays.
float countPerHertz[3];
float hertzPerStepperUnitVVC[3];  // Voltage Variable Cap
int currentBand;

const int STEPPERSLEEPNOT = 9;
const int OPAMPPOWER = 3;
const int RFAMPPOWER = 2;
const int RFRELAYPOWER = 19;

Data();

void computeSlopes();

};