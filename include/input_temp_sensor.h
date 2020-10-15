#pragma once

#include "uart_sensor.h"

#define DATA_SIZE 4

class InputTempSensor : public UARTSensor {

 public:
    InputTempSensor(const std::string& device_path);

    float get_next();
};
