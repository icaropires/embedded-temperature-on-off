#pragma once

#include <iostream>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <csignal>

#include <unistd.h>

#include "sensor.h"
#include "internal_temp_sensor.h"
#include "external_temp_sensor.h"
#include "input_temp_sensor.h"
#include "display_monitor.h"
#include "gpio_actuator.h"

class Control {
    /*
     * Class to control the specified system: https://gitlab.com/fse_fga/projetos/projeto-1
     *
     * Abbreviations:
     * ti: internal temperature
     * te: external temperature
     * tr: reference temperature
     * */
    InternalTempSensor sensor_ti;
    InputTempSensor sensor_tr;
    ExternalTempSensor sensor_te;

    DisplayMonitor display_monitor;
    GPIOActuator cooler;
    GPIOActuator resistor;

    float current_ti, current_te, current_tr;

    std::atomic<bool> do_stop, has_started;
    std::condition_variable cv_update_temperatures;

 public:
    Control(const std::string &sensor_ti_addr, const std::string &sensor_tr_addr,
        const std::string &sensor_te_addr, uint8_t display_monitor_addr,
        uint8_t cooler_addr, uint8_t resistor_addr);

    void stop();

    void start();

    void schedule();

 private:
    bool ask_use_potentiometer();

    void update_temperature(float& current_temperature, Sensor &sensor, std::mutex& mutex);

    void stdin_tr(std::mutex& mutex);

    void control();
};
