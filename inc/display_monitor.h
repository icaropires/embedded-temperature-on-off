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
    DisplayMonitor(const int& i2c_addr);

    ~DisplayMonitor();

    void print(const std::string& first_line, const std::string& second_line);

    void print_temps(const float& internal_temp, const float& external_temp, const float& reference_temp);
};
