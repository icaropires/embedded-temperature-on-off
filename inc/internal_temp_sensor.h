#pragma once

#include "uart_sensor.h"

class InternalTempSensor : public UARTSensor {

 public:
    InternalTempSensor(const std::string& device_path);

    float get_next();
};
