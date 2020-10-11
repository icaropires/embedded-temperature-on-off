#include "uart_sensor.h"

UARTSensor::UARTSensor(const std::string& device_path) : device_path(device_path) {
    serial = open(device_path.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);

    if (serial == -1) {
        auto error_msg = utils::append_strerror("Unable to open temperature input sensor device path " + device_path);
        throw std::invalid_argument(error_msg);
    }

    setup(); 
}

UARTSensor::~UARTSensor() {
    close(serial);
}

void UARTSensor::setup(){
    struct termios options;
    tcgetattr(serial, &options);
    options.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(serial, TCIFLUSH);
    tcsetattr(serial, TCSANOW, &options);
}

ssize_t UARTSensor::write_code(char code){
    char cmd[] = {code, 9, 8, 1, 5};
    tcflush(serial, TCIFLUSH);
    ssize_t count = write(serial, &cmd, sizeof(cmd));

    if (count < 0) {
        auto error_msg = utils::append_strerror("Unable to write to UART bus " + device_path);
        throw std::runtime_error(error_msg);
    }

    return count;
}

ssize_t UARTSensor::read_data(void *buff, int expected_size){
    ssize_t len = read(serial, buff, expected_size);

    if (len < 0) {
        auto error_msg = utils::append_strerror("Unable to read from UART bus " + device_path);
        throw std::runtime_error(error_msg);
    }
    else if (len == 0) {
        fprintf(stderr, "WARNING: No data available when reading UART!\n");
    }

    return len;
}

ssize_t UARTSensor::request_data(char code, void *buff, int expected_size){
    write_code(code);
    usleep(1e5);
    ssize_t len = read_data(buff, expected_size);

    return len;
}

float UARTSensor::request_float(char code){
    uint8_t buff[FLOAT_SIZE];

    ssize_t len = request_data(code, buff, FLOAT_SIZE);

    if (len != FLOAT_SIZE) {
        fprintf(stderr, "WARNING: Missing bytes reading from UART bus!\n");
    }

    float result = *((float*) buff);

    return result;
}
