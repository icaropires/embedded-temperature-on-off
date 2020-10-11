#pragma once

#include "uart_sensor.h"

#define DATA_SIZE 4

class TempInputUART : public UARTSensor {

 public:
    TempInputUART(const std::string& device_path);

    float get_next();
};
