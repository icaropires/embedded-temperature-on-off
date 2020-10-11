#include <iostream>

#include "external_temp_sensor.h"
#include "internal_temp_sensor.h"
#include "input_temp_sensor.h"

std::string get_datetime_csv(){
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    char result[64];//TODO: is it this size?
    strftime(result, sizeof(tm), "%F,%T", &tm);

    return std::string(result);
}

int main() {
    ExternalTempSensor te_sensor("/dev/i2c-1");
    InternalTempSensor ti_sensor("/dev/serial0");
    TempInputUART temp_input_uart("/dev/serial0");

    for(int i = 0; i < 10; ++i){
        std::cout << "TE Curr " << te_sensor.get_next() << std::endl;
        std::cout << "TI Curr " << ti_sensor.get_next() << std::endl;
        std::cout << "Input " << temp_input_uart.get_next() << std::endl;
        usleep(1e5);
    }

    return 0;
}
