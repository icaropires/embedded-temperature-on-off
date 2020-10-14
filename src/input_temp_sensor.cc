#include "input_temp_sensor.h"

InputTempSensor::InputTempSensor(const std::string& device_path) : UARTSensor(device_path) {
}

float InputTempSensor::get_next() {
    const char code_potentiometer = 0xA2;
    float value = request_float(code_potentiometer);

    return value;
}
