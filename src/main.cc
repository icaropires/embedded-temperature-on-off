#include <iostream>

#include <unistd.h>

#include "control.h"


// std::string get_datetime_csv(){
//     time_t t = time(NULL);
//     struct tm tm = *localtime(&t);
// 
//     char result[64];//TODO: is it this size?
//     strftime(result, sizeof(tm), "%F,%T", &tm);
// 
//     return std::string(result);
// }

// params: sensor_ti, sensor_tr, sensor_te, display_monitor, cooler, resistor
Control control("/dev/serial0", "/dev/serial0", "/dev/i2c-1", 0x27, 24, 23);

void finish_handler(int) {
    control.stop();
}

void scheduler(int) {
    control.schedule();
}

int main() {
    signal(SIGINT, finish_handler);
    signal(SIGTERM, finish_handler);
    signal(SIGALRM, scheduler);
    ualarm(1, 5e5);

    control.start();

    std::cout << "Finished controlling the system" << std::endl;

    return 0;
}
