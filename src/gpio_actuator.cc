#include "gpio_actuator.h"

GPIOActuator::GPIOActuator(int pin) : pin(pin) {
#ifdef DEBUG
    bcm2835_set_debug(1);
#endif

    if (!bcm2835_init()) {
        throw std::runtime_error("Unable to start bcm2835 for GPIO sensor");    
    }

    bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
}

GPIOActuator::~GPIOActuator() {
    bcm2835_close();
}

void GPIOActuator::turn_on() {
    bcm2835_gpio_write(pin, static_cast<uint8_t>(Cmd::On));
}

void GPIOActuator::turn_off() {
    bcm2835_gpio_write(pin, static_cast<uint8_t>(Cmd::Off));
}
