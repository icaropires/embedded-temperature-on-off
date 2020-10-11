#include "input_temp_sensor.h"

TempInputUART::TempInputUART(const std::string& device_path) : UARTSensor(device_path) {
}

float TempInputUART::get_next() {
    const char code_potentiometer = 0xA2;
    float value = request_float(code_potentiometer);

    return value;
}
