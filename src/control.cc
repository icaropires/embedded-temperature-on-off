#include "control.h"

Control::Control(const std::string &sensor_ti_addr, const std::string &sensor_tr_addr,
    const std::string &sensor_te_addr, uint8_t display_monitor_addr,
    uint8_t cooler_addr, uint8_t resistor_addr, const std::string& csv_file)
: sensor_ti(sensor_ti_addr), sensor_tr(sensor_tr_addr),
  sensor_te(sensor_te_addr), display_monitor(display_monitor_addr),
  cooler(cooler_addr), resistor(resistor_addr),
  ui(current_ti, current_tr, current_te, mutex_ti, mutex_te_tr),
  do_stop(false), has_started(false), csv_file(csv_file)
{
}

void Control::stop() {
    do_stop = true;
    ui.stop();

    usleep(5e5);  // Let threads finish

    display_monitor.clear();
    cooler.turn_off();
    resistor.turn_off();
}

void Control::start() {
    create_csv();

    hysteresis = ask_hysteresis();
    bool use_potentiometer = ask_use_potentiometer();

    has_started = true;

    std::thread t_ti(&Control::update_ti, this);
    std::thread t_te_tr(&Control::update_te_tr, this, use_potentiometer);
    std::thread t_ui (&UI::start, std::ref(ui), !use_potentiometer);
    std::thread t_display(&Control::update_display, this);

    t_ti.join();
    t_te_tr.join();
    t_ui.join();
}

bool Control::ask_use_potentiometer() const {
    std::string str_potentiometer;
    std::cout << "Use potentiometer (yes)? Or give inputs using keyboard? (no)" << std::endl;
    std::getline(std::cin, str_potentiometer);

    if (str_potentiometer == "yes") {
        return  true;
    } 

    return false;
}

float Control::ask_hysteresis() const {
    float result = -1;

    while (result == -1) {
        try {
            std::string str_hysteresis;
            std::cout << "Which value use for hysteresis?" << std::endl;
            std::getline(std::cin, str_hysteresis);

            result = std::stof(str_hysteresis);
        } catch(std::invalid_argument&) {
            std::cout << "Invalid value! Try again." << std::endl << std::endl;
        }
    }

    return result;
}

void Control::schedule() {
    /* Meant to be called every 500ms */
    /* Same period for applying control and updating temperature */
    if (has_started) {
        return;
    }

    unsigned int save_csv_period = 4;  // Save every 2s
    save_csv_counter++;
    if (save_csv_counter == save_csv_period) {
        cv_csv.notify_one();
    } else if(save_csv_counter > save_csv_period) {
        save_csv_counter = 0;
    }

    apply();
    ui.print_out();
    cv_update_temperatures.notify_all();  // Start updating all temperatures at same time
}

void Control::update_display() {
    std::unique_lock<std::mutex> lock_display(mutex_display); 

    while (!do_stop) {
        {
            std::lock_guard<std::mutex> lock_ti(mutex_ti), lock_te_tr(mutex_te_tr);
            display_monitor.print_temps(current_ti, current_tr, current_te);
        }

        cv_update_temperatures.wait(lock_display);
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

std::string Control::get_datetime_csv() {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    char datetime[20];  // Ex: 2020-09-22,19:52:31
    strftime(datetime, sizeof(tm), "%F,%T", &tm);

    return std::string(datetime);
}

void Control::create_csv() {
    std::ofstream s_csv_file;
    s_csv_file.open(csv_file, std::ios::out | std::ios::trunc);
    s_csv_file << "date,time,temperatura interna,temperatura externa,temperatura de referência" << std::endl;
    s_csv_file.close();
}

void Control::update_csv() {
    std::unique_lock<std::mutex> lock(mutex_csv);

    while (!do_stop) {
        std::ofstream s_csv_file;
        s_csv_file.open(csv_file, std::ios::out | std::ios::app);

        std::string datetime = get_datetime_csv();
        s_csv_file << std::fixed << std::setprecision(2) << std::setfill('0') << datetime << ',' << current_ti << ',' << current_te << ',' << current_tr << std::endl;

        s_csv_file.close();

        cv_csv.wait(lock);
    }
}

void Control::apply() {
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
}
