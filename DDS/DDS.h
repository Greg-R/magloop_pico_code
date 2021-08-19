#include <stdint.h>
#include "pico/stdlib.h"

//  Class for controlling the DDS signal generator module.

class DDS {

    reset 
    data
    fq_ud
    wlck

public:

DDS(DDS_RST, DDS_DATA, DDS_FQ_UD, WLCK) {


}

void DDSWakeUp();

void outOne();

void outZero();

void byte_out (unsigned char byte);

void SendFrequency(long frequency);

};