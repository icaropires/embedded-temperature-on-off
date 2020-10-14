#include "control.h"

Control::Control(const std::string &sensor_ti_addr, const std::string &sensor_tr_addr,
    const std::string &sensor_te_addr, uint8_t display_monitor_addr,
    uint8_t cooler_addr, uint8_t resistor_addr)
: sensor_ti(sensor_ti_addr), sensor_tr(sensor_tr_addr),
  sensor_te(sensor_te_addr), display_monitor(display_monitor_addr),
  cooler(cooler_addr), resistor(resistor_addr), current_ti(23),
  current_te(23), current_tr(23), do_stop(false), has_started(false) {}

void Control::stop() {
    do_stop = true;

    usleep(5e5);  // Let threads finish

    cooler.turn_off();
    resistor.turn_off();
}

void Control::start() {
    bool use_potentiometer = ask_use_potentiometer();

    has_started = true;

    std::thread t_stdin_tr(&Control::stdin_tr, this, !use_potentiometer, std::ref(mutex_te_tr));
    t_stdin_tr.detach();

    std::thread t_ti(&Control::update_ti, this);
    std::thread t_te_tr(&Control::update_te_tr, this, use_potentiometer);

    t_ti.join();
    t_te_tr.join();
}

bool Control::ask_use_potentiometer() {
    std::string str_potentiometer;
    std::cout << "Activate Potentiometer (yes)? Or give inputs using keyboard? (no)" << std::endl;
    std::getline(std::cin, str_potentiometer);

    if (str_potentiometer == "yes") {
        return  true;
    } 

    return false;
}

void Control::schedule() {
    /* Same period for applying control and updating temperature */

    if (has_started) {
        control();
        cv_update_temperatures.notify_all();  // Start updating all temperatures at same time
        display_monitor.print_temps(current_ti, current_tr, current_te);
    }
}

void Control::update_ti() {
    std::unique_lock<std::mutex> lock(mutex_ti); 

    while (!do_stop) {
        try {
            current_ti = sensor_ti.get_next();
        } catch (std::runtime_error& e) {
            std::cerr << "Error when getting data: " << e.what() << std::endl;
        }
        cv_update_temperatures.wait(lock);
    }
}

void Control::update_te_tr(bool update_tr) {
    std::unique_lock<std::mutex> lock(mutex_te_tr); 

    while (!do_stop) {
        try {
            current_te = sensor_te.get_next();

            if (update_tr) {
                current_tr = sensor_tr.get_next();
            }
        } catch (std::runtime_error& e) {
            std::cerr << "Error when getting data: " << e.what() << std::endl;
        }
        cv_update_temperatures.wait(lock);
    }
}

void Control::stdin_tr(bool activate, std::mutex& mutex) {
    if (!activate) {
        return;
    }

    while (!do_stop) {
        std::string tr_str;
        std::getline(std::cin, tr_str);
        float new_tr = stof(tr_str);

        {
            std::lock_guard lock(mutex);
            current_tr = new_tr;
            std::cout << "Reference temperature updated to: "  << current_tr << std::endl;
        }
    }
}

void Control::control() {
    const float hysteresis = 4; // tem que ser input do usuário

    float tr_aux = -1;
    {
        std::lock_guard<std::mutex> lock(mutex_te_tr);
        tr_aux = current_tr;
    }

    float upper = tr_aux + hysteresis / 2;
    float lower = tr_aux - hysteresis / 2;

    {
        std::lock_guard<std::mutex> lock(mutex_ti);

        if (current_ti < lower) {
            cooler.turn_off();
            resistor.turn_on();

        } else if(current_ti > upper) {
            cooler.turn_on();
            resistor.turn_off();
        }
    }

    std::cout << "TI: " << current_ti << " -> TR: " << current_tr << std::endl;
    std::cout << "TE: " << current_te << std::endl;
    std::cout << std::endl;
}
