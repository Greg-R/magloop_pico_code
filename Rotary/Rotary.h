/*
 * Rotary encoder library for Arduino.
 */

#ifndef Rotary_h
#define Rotary_h

#include "Arduino.h"
#include "pico/stdlib.h"

// Enable this to emit codes twice per step.
// #define HALF_STEP

// Values returned by 'process'
// No complete step yet.
#define DIR_NONE 0x0
// Clockwise step.
#define DIR_CW 0x10
// Counter-clockwise step.
#define DIR_CCW 0x20

//gpio_irq_callback_t encoderCallback1(uint gpio, uint32_t events);

class Rotary
{
  public:
    Rotary(char, char);
    unsigned char process();
    void begin(bool internalPullup=true, bool flipLogicForPulldown=false);
    //volatile static uint8_t result;  // Set by encoder callback function.
    gpio_irq_callback_t encoderCallback1(uint gpio, uint32_t events);
 //   gpio_irq_callback_t encoderCallback2(uint gpio, uint32_t events);
    uint8_t result;
  private:
    unsigned char state;
    unsigned char pin1;
    unsigned char pin2;
    unsigned char inverter;
};

#endif
 
