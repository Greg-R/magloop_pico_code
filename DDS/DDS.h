#pragma once
#include <stdint.h>
#include "pico/stdlib.h"


//  Class/library for controlling the DDS signal generator module.

class DDS {

public:

unsigned int    RESET;
unsigned int    DATA;
unsigned int    FQ_UD;
unsigned int    WCLK;

DDS(unsigned int DDS_RST, unsigned int DDS_DATA, unsigned int DDS_FQ_UD, unsigned int DDS_WCLK);

void DDSWakeUp();

void outOne();

void outZero();

void byte_out (unsigned char byte);

void SendFrequency(long frequency);

};