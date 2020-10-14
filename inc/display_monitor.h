#pragma once

#include <string>
#include <stdexcept>
#include <cstdio>

#include <unistd.h>

#include "display.h"
#include "utils.h"

#define LINE_SIZE 16

class DisplayMonitor {

 private:
    int fd;
    int i2c_addr;

 public:
    DisplayMonitor(int i2c_addr);

    ~DisplayMonitor();

    void print(const std::string& first_line, const std::string& second_line) const;

    void print_temps(float internal_temp, float external_temp, float reference_temp) const;
};
