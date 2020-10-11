#include "internal_temp_sensor.h"

#include <iostream>

InternalTempSensor::InternalTempSensor() : dev_path("/dev/i2c-1") {

    if ((id.fd = open(dev_path.c_str(), O_RDWR)) < 0) {
        fprintf(stderr, "Failed to open the i2c bus %s\n", dev_path.c_str());
        exit(1);
    }

    id.dev_addr = BME280_I2C_ADDR_PRIM;

    if (ioctl(id.fd, I2C_SLAVE, id.dev_addr) < 0) {
        fprintf(stderr, "Failed to acquire bus access and/or talk to slave.\n");
        exit(1);
    }

    dev.intf = BME280_I2C_INTF;
    dev.read = user_i2c_read;
    dev.write = user_i2c_write;
    dev.delay_us = user_delay_us;

    dev.intf_ptr = &id;

    int8_t result = bme280_init(&dev);
    if (result != BME280_OK) {
        fprintf(stderr, "Failed to initialize the device (code %+d).\n", result);
        exit(1);
    }

    setup();
}

InternalTempSensor::~InternalTempSensor() {
    close(id.fd);
}

float InternalTempSensor::get_next() {
    int8_t result = bme280_set_sensor_mode(BME280_FORCED_MODE, &dev);
    if (result != BME280_OK) {
        fprintf(stderr, "Failed to set sensor mode (code %+d).", result);
        exit(1);
    }

    struct bme280_data data;

    result = bme280_get_sensor_data(BME280_ALL, &data, &dev);
    if (result != BME280_OK){
        fprintf(stderr, "Failed to get sensor data (code %+d).", result);
    }

    float temp = -1.0;

    #ifdef BME280_FLOAT_ENABLE
        temp = data.temperature;
    #else
    #ifdef BME280_64BIT_ENABLE
        temp = 0.01f * data.temperature;
    #else
        temp = 0.01f * data.temperature;
    #endif
    #endif

    return temp;
}

void InternalTempSensor::setup() {
    /* Recommended mode of operation: Indoor navigation */
    dev.settings.osr_h = BME280_OVERSAMPLING_1X;
    dev.settings.osr_p = BME280_OVERSAMPLING_16X;
    dev.settings.osr_t = BME280_OVERSAMPLING_2X;
    dev.settings.filter = BME280_FILTER_COEFF_16;

    uint8_t settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;

    // Set the sensor settings
    int8_t result = bme280_set_sensor_settings(settings_sel, &dev);
    if (result != BME280_OK) {
        fprintf(stderr, "Failed to set sensor settings (code %+d).", result);
        exit(1);
    }

    result = bme280_set_sensor_mode(BME280_FORCED_MODE, &dev);
    if (result != BME280_OK) {
        fprintf(stderr, "Failed to set sensor mode (code %+d).", result);
        exit(1);
    }

    uint32_t delay = 4e4;  // If less, it gives always 22.1504
    dev.delay_us(delay, dev.intf_ptr);
}

void InternalTempSensor::user_delay_us(uint32_t period, void *intf_ptr) {
    (void) intf_ptr;  // skip unused warning
    usleep(period);
}

int8_t InternalTempSensor::user_i2c_read(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr){
    struct identifier id;

    id = *((struct identifier *)intf_ptr);

    write(id.fd, &reg_addr, 1);
    read(id.fd, data, len);

    return 0;
}

int8_t InternalTempSensor::user_i2c_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr){
    struct identifier id = *((struct identifier *)intf_ptr);

    uint8_t *buf = (uint8_t *) malloc(len + 1);
    buf[0] = reg_addr;
    memcpy(buf + 1, data, len);

    if (write(id.fd, buf, len + 1) < (uint16_t)len){
        return BME280_E_COMM_FAIL;
    }

    free(buf);
    return BME280_OK;
}
