#pragma once
#include <stdint.h>
#include "pico/stdlib.h"


//  The Buttons class is used to debounce the push buttons.

class Button {

    public:

    uint gpio;
    int buttonFlag;
    enum class State {state0, state1, state2, state3};
    State state;
    bool pushed, lastPushed;  // This member indicates if the (de-bounced) button was pushed.
    uint64_t startTime;        // Record the start time of the de-bounce execute (appromiate).
    uint64_t debounceInterval = 50;

    Button(uint gpio);

    void initialize();

    void buttonPushed();
};