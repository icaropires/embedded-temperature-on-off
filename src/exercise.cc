/**\
 * Copyright (c) 2020 Bosch Sensortec GmbH. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **/

/**\
 * Modified example due to homework reasons :)
 **/

/**
 * \ingroup bme280
 * \defgroup bme280Examples Examples
 * @brief Reference Examples
 */

/*!
 * @ingroup bme280Examples
 * @defgroup bme280GroupExampleLU linux_userspace
 * @brief Linux userspace test code, simple and mose code directly from the doco.
 * compile like this: gcc linux_userspace.c ../bme280.c -I ../ -o bme280
 * tested: Raspberry Pi.
 * Use like: ./bme280 /dev/i2c-0
 * \include linux_userspace.c
 */

// #ifdef __KERNEL__
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
// #endif

/******************************************************************************/
/*!                         System header files                               */
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>


/******************************************************************************/
/*!                         Own header files                                  */
#include "bme280.h"

/******************************************************************************/
/*!                               Structures                                  */

/* Structure that contains identifier details used in example */
struct identifier
{
    /* Variable to hold device address */
    uint8_t dev_addr;

    /* Variable that contains file descriptor */
    int8_t fd;
};

/******************************************************************************/
/*!                               Constants                                   */
#define MEAN_INTERVAL 10

/****************************************************************************/
/*!                         Functions                                       */

/*!
 *  @brief Function that creates a mandatory delay required in some of the APIs.
 *
 *  @param[in] period              : Delay in microseconds.
 *  @param[in, out] intf_ptr       : Void pointer that can enable the linking of descriptors
 *                                  for interface related call backs
 *  @return void.
 *
 */
void user_delay_us(uint32_t period, void *intf_ptr);

/*!
 *  @brief Function to get CSV formatted string (separator = ",") date and time fields.
 *
 *  @param[out] result       : Pointer to result string
 *
 *  @return void.
 *
 */
void get_datetime_csv(char *result);

/*!
 * @brief Function to assemble a string with temperature, humidity and pressure data separated by ",".
 *
 *  @param[in] data       : the data struct with the attributes values.
 *
 *  @param[out] result    :   The string with the attributes separated by ","
 *
 *  @return void.
 *
 */
void format_data_csv(struct bme280_data *data, char *row);

/*!
 *  @brief Function for reading the sensor's registers through I2C bus.
 *
 *  @param[in] reg_addr       : Register address.
 *  @param[out] data          : Pointer to the data buffer to store the read data.
 *  @param[in] len            : No of bytes to read.
 *  @param[in, out] intf_ptr  : Void pointer that can enable the linking of descriptors
 *                                  for interface related call backs.
 *
 *  @return Status of execution
 *
 *  @retval 0 -> Success
 *  @retval > 0 -> Failure Info
 *
 */
int8_t user_i2c_read(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr);

/*!
 *  @brief Function for writing the sensor's registers through I2C bus.
 *
 *  @param[in] reg_addr       : Register address.
 *  @param[in] data           : Pointer to the data buffer whose value is to be written.
 *  @param[in] len            : No of bytes to write.
 *  @param[in, out] intf_ptr  : Void pointer that can enable the linking of descriptors
 *                                  for interface related call backs
 *
 *  @return Status of execution
 *
 *  @retval BME280_OK -> Success
 *  @retval BME280_E_COMM_FAIL -> Communication failure.
 *
 */
int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr);

/*!
 * @brief Function reads temperature, humidity and pressure data in forced mode.
 *
 * @param[in] dev   :   Structure instance of bme280_dev.
 *
 * @return Result of API execution status
 *
 * @retval BME280_OK - Success.
 * @retval BME280_E_NULL_PTR - Error: Null pointer error
 * @retval BME280_E_COMM_FAIL - Error: Communication fail error
 * @retval BME280_E_NVM_COPY_FAILED - Error: NVM copy failed
 *
 */
int8_t stream_sensor_data_forced_mode(struct bme280_dev *dev, char *csv_path);

/*!
 * @brief This function starts execution of the program.
 */
int main(int argc, char* argv[])
{
    struct bme280_dev dev;

    struct identifier id;

    /* Variable to define the result */
    int8_t rslt = BME280_OK;

    if (argc < 3)
    {
        fprintf(stderr, "Usage syntax: ./bme280 [file bus path] [csv file]\n");
        exit(1);
    }

    if ((id.fd = open(argv[1], O_RDWR)) < 0)
    {
        fprintf(stderr, "Failed to open the i2c bus %s\n", argv[1]);
        exit(1);
    }

// #ifdef __KERNEL__
    /* Make sure to select BME280_I2C_ADDR_PRIM or BME280_I2C_ADDR_SEC as needed */
    id.dev_addr = BME280_I2C_ADDR_PRIM;

    if (ioctl(id.fd, I2C_SLAVE, id.dev_addr) < 0)
    {
        fprintf(stderr, "Failed to acquire bus access and/or talk to slave.\n");
        exit(1);
    }

// #endif

    dev.intf = BME280_I2C_INTF;
    dev.read = user_i2c_read;
    dev.write = user_i2c_write;
    dev.delay_us = user_delay_us;

    /* Update interface pointer with the structure that contains both device address and file descriptor */
    dev.intf_ptr = &id;

    /* Initialize the bme280 */
    rslt = bme280_init(&dev);
    if (rslt != BME280_OK)
    {
        fprintf(stderr, "Failed to initialize the device (code %+d).\n", rslt);
        exit(1);
    }

    rslt = stream_sensor_data_forced_mode(&dev, argv[2]);
    if (rslt != BME280_OK)
    {
        fprintf(stderr, "Failed to stream sensor data (code %+d).\n", rslt);
        exit(1);
    }

    return 0;
}

/*!
 * @brief This function reading the sensor's registers through I2C bus.
 */
int8_t user_i2c_read(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr)
{
    struct identifier id;

    id = *((struct identifier *)intf_ptr);

    write(id.fd, &reg_addr, 1);
    read(id.fd, data, len);

    return 0;
}

/*!
 * @brief This function provides the delay for required time (Microseconds) as per the input provided in some of the
 * APIs
 */
void user_delay_us(uint32_t period, void *intf_ptr)
{
    (void) intf_ptr;  // skip unused warning
    usleep(period);
}

/*!
 *  @brief Function to get CSV formatted string (separator = ",") date and time fields.
 */
void get_datetime_csv(char *result){
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    strftime(result, sizeof(tm), "%F,%T", &tm);
}

/*!
 * @brief This function for writing the sensor's registers through I2C bus.
 */
int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr)
{
    uint8_t *buf;
    struct identifier id;

    id = *((struct identifier *)intf_ptr);

    buf = (unsigned char *) malloc(len + 1);
    buf[0] = reg_addr;
    memcpy(buf + 1, data, len);
    if (write(id.fd, buf, len + 1) < (uint16_t)len)
    {
        return BME280_E_COMM_FAIL;
    }

    free(buf);

    return BME280_OK;
}

/*!
 * @brief Function to assemble a string with temperature, humidity and pressure data separated by ",".
 */
void format_data_csv(struct bme280_data *data, char *row)
{
    float temp = -1.0, press = -1.0, hum = -1.0;

#ifdef BME280_FLOAT_ENABLE
    temp = data->temperature;
    press = 0.01 * data->pressure;
    hum = data->humidity;
#else
#ifdef BME280_64BIT_ENABLE
    temp = 0.01f * data->temperature;
    press = 0.0001f * data->pressure;
    hum = 1.0f / 1024.0f * data->humidity;
#else
    temp = 0.01f * data->temperature;
    press = 0.01f * data->pressure;
    hum = 1.0f / 1024.0f * data->humidity;
#endif
#endif
    char datetime[20];  // Ex: 2020-09-22,19:52:31
    get_datetime_csv(datetime);

    sprintf(row, "%s,%0.2lf,%0.2lf,%0.2lf", datetime, temp, hum, press);
}

/*!
 * @brief This API reads the sensor temperature, pressure and humidity data in forced mode.
 */
int8_t stream_sensor_data_forced_mode(struct bme280_dev *dev, char *csv_path)
{
    /* Recommended mode of operation: Indoor navigation */
    dev->settings.osr_h = BME280_OVERSAMPLING_1X;
    dev->settings.osr_p = BME280_OVERSAMPLING_16X;
    dev->settings.osr_t = BME280_OVERSAMPLING_2X;
    dev->settings.filter = BME280_FILTER_COEFF_16;

    uint8_t settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;

    // Set the sensor settings
    int8_t result = bme280_set_sensor_settings(settings_sel, dev);
    if (result != BME280_OK)
    {
        fprintf(stderr, "Failed to set sensor settings (code %+d).", result);

        return result;
    }

    FILE *csv = fopen(csv_path, "w");
    if (csv == NULL)
    {
        perror("Unable to open the file");
        exit(1);
    }

    printf("Started collecting. Saving to %s every %d seconds...\n", csv_path, MEAN_INTERVAL);

    fprintf(csv, "date,time,temperature,humidity,pressure\n");

    unsigned long long saved_counter = 0;
    while (1)
    {
        printf("\r\tSaved %llu samples so far!", saved_counter++);
        fflush(stdout);

        result = bme280_set_sensor_mode(BME280_FORCED_MODE, dev);
        if (result != BME280_OK)
        {
            fprintf(stderr, "Failed to set sensor mode (code %+d).", result);
            break;
        }

        struct bme280_data data_mean = {0, 0, 0};

        for (int i = 0; i < MEAN_INTERVAL; ++i)
        {
            uint32_t measurement_period = (uint32_t) 1e6;
            dev->delay_us(measurement_period, dev->intf_ptr);

            struct bme280_data data;
            result = bme280_get_sensor_data(BME280_ALL, &data, dev);

            if (result != BME280_OK)
            {
                fprintf(stderr, "Failed to get sensor data (code %+d).", result);
                break;
            }

            data_mean.temperature += data.temperature / MEAN_INTERVAL;
            data_mean.pressure += data.pressure / MEAN_INTERVAL;
            data_mean.humidity += data.humidity / MEAN_INTERVAL;
        }

        char row[50];  // Ex: 2020-09-22,20:53:36,29.58,898.82,35.83
        format_data_csv(&data_mean, row);

        fprintf(csv, "%s\n", row);
        fflush(csv);
    }

    putchar('\n');
    fclose(csv);
    return result;
}
