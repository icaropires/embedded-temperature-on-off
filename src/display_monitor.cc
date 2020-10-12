#include "display_monitor.h"

DisplayMonitor::DisplayMonitor(const int& i2c_addr): i2c_addr(i2c_addr) {
    wiringPiSetup();  // It doesn't return error code anymore: http://wiringpi.com/reference/setup/

    fd = wiringPiI2CSetup(i2c_addr);

    if (fd < 0) {
        auto error_msg = utils::append_strerror("Unable to connect to given i2c address " + std::to_string(i2c_addr));
        throw std::invalid_argument(error_msg);
    }

    lcd_init(fd);
}

DisplayMonitor::~DisplayMonitor() {
    clrLcd(fd);
    close(fd);
}

void DisplayMonitor::print(const std::string& first_line, const std::string& second_line){
    lcdLoc(fd, LINE1);
    typeln(fd, first_line.c_str());

    lcdLoc(fd, LINE2);
    typeln(fd, second_line.c_str()); 
}

void DisplayMonitor::print_temps(const float& internal_temp, const float& external_temp, const float& reference_temp){
    char line[LINE_SIZE];

    sprintf(line, "TI:%0.1f TR:%0.1f", internal_temp, reference_temp);
    std::string first_line(line);

    sprintf(line, "TR: %0.2f", external_temp);
    std::string second_line(line);

    print(first_line, second_line);
}
