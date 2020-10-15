#pragma once

#include <iostream>
#include <fstream>
#include <iomanip>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <csignal>
#include <stdexcept>


#include <unistd.h>

#include "sensor.h"
#include "ui.h"
#include "internal_temp_sensor.h"
#include "external_temp_sensor.h"
#include "input_temp_sensor.h"
#include "display_monitor.h"
#include "gpio_actuator.h"

class Control {
    /*
     * Class to control the specified system: https://gitlab.com/fse_fga/projetos/projeto-1
     *
     * Assuming that tr and ti sensors are connected through UART at the same bus. They
     *   will be synchronized together
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

    UI ui;

    float current_ti = 23, current_te = 23, current_tr = 23;
    float hysteresis = 4;

    std::atomic<bool> do_stop, has_started;

    // cv_general for loops which follow same period
    std::condition_variable cv_general, cv_csv;
    std::mutex mutex_ti, mutex_te, mutex_tr, mutex_display, mutex_csv, mutex_apply;

    std::string csv_file;
    unsigned int save_csv_counter = 0;

 public:
    Control(const std::string &sensor_ti_addr, const std::string &sensor_tr_addr,
        const std::string &sensor_te_addr, uint8_t display_monitor_addr,
        uint8_t cooler_addr, uint8_t resistor_addr, const std::string& csv_file);

    void stop();

    void start();

    void schedule();

 private:
    bool ask_use_potentiometer() const;
    float ask_hysteresis() const;

    void create_csv();
    std::string get_datetime_csv();

    void update_csv_loop();
    void update_display_loop();
    void update_te_loop();
    void update_ti_tr_loop(bool update_tr);
    void apply_loop();
};
