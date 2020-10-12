#pragma once

// Conflicting with wiringPi definitions
#ifdef HIGH
#undef HIGH
#endif
#ifdef LOW
#undef LOW
#endif

#include <bcm2835.h>

#include <stdexcept>

class GPIOActuator {
    int pin;

    enum class Cmd {On = LOW, Off = HIGH};

 public:
    GPIOActuator(int pin);

    ~GPIOActuator();

    void turn_on();

    void turn_off();
};
