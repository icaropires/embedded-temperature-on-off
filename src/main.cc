#include <iostream>

#include "internal_temp_sensor.h"
#include "external_temp_sensor.h"
#include "input_temp_sensor.h"
#include "display_monitor.h"

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
    DisplayMonitor display_monitor(0x27);

    for(int i = 0; i < 10; ++i){
        float ti_temp = ti_sensor.get_next();
        float te_temp = te_sensor.get_next();
        float tr_temp = temp_input_uart.get_next();

        std::cout << "TE " << te_temp << std::endl;
        std::cout << "TI " << ti_temp << std::endl;
        std::cout << "TR " << tr_temp << std::endl;

        display_monitor.print_temps(ti_temp, te_temp, tr_temp);
        usleep(1e5);
    }

    return 0;
}
