#include <stdint.h>
#include "pico/stdlib.h"


//  Class/library for controlling the DDS signal generator module.

class DDS {

public:

uint    RESET;
uint    DATA;
uint    FQ_UD;
uint    WCLK;

DDS(uint DDS_RST, uint DDS_DATA, uint DDS_FQ_UD, uint DDS_WCLK);

void DDSWakeUp();

void outOne();

void outZero();

void byte_out (unsigned char byte);

void SendFrequency(long frequency);

};