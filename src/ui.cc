#include "ui.h"

UI::UI(float& current_ti, float& current_tr, float& current_te, std::mutex& mutex_ti, std::mutex& mutex_tr, std::mutex& mutex_te) 
    : do_stop(false),
    current_ti(current_ti), current_tr(current_tr), current_te(current_te),
    mutex_ti(mutex_ti), mutex_tr(mutex_tr), mutex_te(mutex_te)
{}

UI::~UI() {}

void UI::stop() {
    do_stop = true;
    usleep(1e5);
    endwin();
}

void UI::start(bool enable_input) {
    initscr();
    cbreak();

    if (has_colors() == TRUE) {
        use_default_colors();

        start_color();
        init_pair(success_color_id, COLOR_GREEN, -1);
        init_pair(error_color_id, COLOR_RED, -1);
    } else {
        std::cerr << "WARNING: This terminal doesn't support colors!" << std::endl;
    }
    draw_output();

    if (enable_input) {
        draw_input();
        std::thread t_input(&UI::update_input, this);
        t_input.detach();
    }

    update_output();
}

void UI::print_out() {
    cv_output.notify_one(); 
}

void UI::draw_input() {
    std::lock_guard<std::mutex> lock(mutex_ui);

    WINDOW *borders = newwin(input_height, general_width, top_pad, left_pad);
    box(borders, 0, 0);
    mvwprintw(borders, 0, title_pad, "Input");
    wrefresh(borders);

    input_window = newwin(input_height-2, general_width-2, top_pad+1, left_pad+1);

    mvwprintw(input_window, 0, 0, input_string.c_str());
    wrefresh(input_window);
}

void UI::update_input() {
    while(!do_stop) {
        char buff[1024];
        const int str_size = input_string.size();
        mvwgetnstr(input_window, 0, str_size, buff, general_width-str_size-3);  // Blocking

        bool failed = false;
        float temperature = -1;
        try {
            temperature = std::stof(std::string(buff));
        } catch(std::invalid_argument&) {
            failed = true;
        }

        {
            std::lock_guard<std::mutex> lock(mutex_ui);

            if (!failed) {
                float old = -1;
                {
                    std::lock_guard<std::mutex> lock(mutex_tr);

                    old = current_tr;
                    current_tr = temperature;
                }

                wattron(input_window, COLOR_PAIR(success_color_id)); 
                mvwprintw(input_window, 1, 0, "Reference temperature updated: %0.2f -> %0.2f!", old, temperature);
                wattroff(input_window, COLOR_PAIR(success_color_id)); 
            } else {
                wattron(input_window, COLOR_PAIR(error_color_id)); 
                mvwprintw(input_window, 1, 0, "Invalid temperature! Try again.", buff);
                wattroff(input_window, COLOR_PAIR(error_color_id)); 
            }
            wrefresh(input_window);
        }

        sleep(2);

        {
            std::lock_guard<std::mutex> lock(mutex_ui);

            wmove(input_window, 0, 0);
            wclrtoeol(input_window);

            wmove(input_window, 1, 0);
            wclrtoeol(input_window);

            mvwprintw(input_window, 0, 0, input_string.c_str());
            wrefresh(input_window);
        }
    }
}

void UI::draw_output() {
    std::lock_guard<std::mutex> lock(mutex_ui);

    WINDOW *borders = newwin(output_height, general_width, top_pad+windows_pad, left_pad);
    box(borders, 0, 0);
    mvwprintw(borders, 0, title_pad, "Output");
    wrefresh(borders);

    output_window = newwin(output_height-2, general_width-2, top_pad+windows_pad+1, left_pad+1);

    scrollok(output_window, TRUE);
}

void UI::update_output() {
    while(!do_stop) {
        std::unique_lock<std::mutex> lock_ui(mutex_ui);

        {
            std::lock_guard<std::mutex> lock_ti(mutex_ti), lock_tr(mutex_tr), lock_te(mutex_te);

            wprintw(output_window, "TE: %0.2f; TI: %0.2f -> TR: %0.2f\n", current_te, current_ti, current_tr);
            wrefresh(output_window);
        }

        cv_output.wait(lock_ui);
    }
}
