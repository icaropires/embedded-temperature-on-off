#include "internal_temp_sensor.h"

#include <iostream>

InternalTempSensor::InternalTempSensor(const std::string& device_path) : UARTSensor(device_path) {
}

float InternalTempSensor::get_next() {
    const char lm35_sensor = 0xA1;
    float value = request_float(lm35_sensor);

    return value;
}
