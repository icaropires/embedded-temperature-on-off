#pragma once

#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

#include <string>
#include <stdexcept>

#include "bme280.h"
#include "sensor.h"

class ExternalTempSensor : public Sensor {
    struct identifier {
        uint8_t device_addr;
        int8_t fd;
    };

    struct identifier id;
    struct bme280_dev dev;

    std::string device_path;

 public:
    ExternalTempSensor(const std::string& device_path);
    ~ExternalTempSensor();

    float get_next();

 private:
    void setup();

    // To use with bme280_dev type struct
    static void user_delay_us(uint32_t period, void *intf_ptr);
    static int8_t user_i2c_read(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr);
    static int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr);
};
