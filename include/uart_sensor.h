#pragma once

#include <termios.h>

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>

#include "sensor.h"
#include "utils.h"

#define MAX_DATA_SIZE 256
#define FLOAT_SIZE 4


class UARTSensor : public Sensor {
 public:
    UARTSensor(const std::string& device_path);

    ~UARTSensor();

 protected:
    ssize_t request_data(char code, void *buff, int expected_size);
    float request_float(char code);

 private:
    int serial;
    std::string device_path;

    ssize_t write_code(char code);

    ssize_t read_data(void *buf, int expected_size);

    void setup();
};
