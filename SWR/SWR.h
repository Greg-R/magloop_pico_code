#pragma once
#include "pico/stdlib.h"
#include "hardware/adc.h"


class SWR {

public:

const int MAXPOINTSPERSAMPLE = 2;
int forward_offset, reverse_offset;
float forward_voltage, reverse_voltage;

SWR();

void ReadADCoffsets();

float ReadSWRValue();

float ReadNewSWRValue();

};