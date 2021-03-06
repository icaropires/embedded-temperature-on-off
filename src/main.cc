#include <iostream>

#include <unistd.h>

#include "control.h"

// params: sensor_ti, sensor_tr, sensor_te, display_monitor, cooler, resistor
Control control("/dev/serial0", "/dev/serial0", "/dev/i2c-1", 0x27, 24, 23, "log.csv");

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

    return 0;
}
