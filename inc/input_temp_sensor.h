#pragma once

#include <termios.h>

#include <iostream>
#include <cstring>
#include <cerrno>
#include <stdexcept>

#define DATA_SIZE 4

class TempInputUART {

 public:
    TempInputUART(const std::string& dev_path) : dev_path(dev_path) {
        serial = open(dev_path.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
        setup_uart(); 
    }

    ~TempInputUART() {
        close(serial);
    }

    float get_next() {
        return read_uart(); 
    }

 private:
    int serial;
    std::string dev_path;

    void setup_uart(){
        if (serial == -1) {
            auto error_msg = std::strerror(errno);
            throw std::invalid_argument(error_msg);
        }

        struct termios options;
        tcgetattr(serial, &options);
        options.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
        options.c_iflag = IGNPAR;
        options.c_oflag = 0;
        options.c_lflag = 0;
        tcflush(serial, TCIFLUSH);
        tcsetattr(serial, TCSANOW, &options);
    }

    ssize_t write_code(char code){
        char cmd[] = {code, 9, 8, 1, 5};
        ssize_t count = write(serial, &cmd, sizeof(cmd));

        if (count < 0) {
            auto error_msg = std::strerror(errno);
            throw std::runtime_error(error_msg);
        }

        return count;
    }

    ssize_t read_data(void *buf, int expected_size){
        ssize_t len = read(serial, buf, expected_size);

        if (len < 0) {
            auto error_msg = std::strerror(errno);
            throw std::runtime_error(error_msg);
        }
        else if (len == 0) {
            fprintf(stderr, "WARNING: No data available for user temperature input!\n");
        }

        return len;
    }

    float read_uart(){
        const char code_potentiometer = 0xA2;
        write_code(code_potentiometer);

        usleep(1e5);

        uint8_t buff[DATA_SIZE];
        read_data(buff, DATA_SIZE);

        float result = *((float*) buff);

        return result;
    }
};
