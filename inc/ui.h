#pragma once

#include <iostream>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <csignal>
#include <stdexcept>

#include <ncurses.h>
#include <unistd.h>

class UI {
    std::mutex mutex_ui;
    std::atomic<bool> do_stop;

    int general_width = 70, input_height = 4, output_height = 20;
    int title_pad = 2, left_pad = 1, top_pad = 1, windows_pad = 5;

    float &current_ti, &current_tr, &current_te;
    std::mutex &mutex_ti, &mutex_te_tr;

    WINDOW *input_window = nullptr;
    WINDOW *output_window = nullptr;

    const std::string input_string = "Reference temperature: ";
    std::condition_variable cv_output;

    const int success_color_id = 1, error_color_id = 2;

public:

    UI(float& current_ti, float& current_tr, float& current_te, std::mutex& mutex_ti, std::mutex& mutex_te_tr);

    ~UI();

    void stop();

    void start(bool enable_input = true);

    void print_out();

private:

    void draw_input();

    void update_input();

    void draw_output();

    void update_output();
};
