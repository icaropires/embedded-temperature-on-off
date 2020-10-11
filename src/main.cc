#include <iostream>
#include "internal_temp_sensor.h"

std::string get_datetime_csv(){
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    char result[64];//TODO: is it this size?
    strftime(result, sizeof(tm), "%F,%T", &tm);

    return std::string(result);
}

int main() {
    InternalTempSensor it_sensor;
    for(int i = 0; i < 10; ++i){
        std::cout << "Heey " << it_sensor.get_next() << std::endl;
        usleep(1e5);
    }

    return 0;
}
