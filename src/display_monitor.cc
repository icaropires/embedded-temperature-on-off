#include "display_monitor.h"

DisplayMonitor::DisplayMonitor(int i2c_addr): i2c_addr(i2c_addr) {
    wiringPiSetup();  // It doesn't return error code anymore: http://wiringpi.com/reference/setup/

    fd = wiringPiI2CSetup(i2c_addr);

    if (fd < 0) {
        auto error_msg = utils::append_strerror("Unable to connect to given i2c address " + std::to_string(i2c_addr));
        throw std::invalid_argument(error_msg);
    }

    lcd_init(fd);
}

DisplayMonitor::~DisplayMonitor() {
    clear();
    close(fd);
}

void DisplayMonitor::clear() const {
    clrLcd(fd);
}

void DisplayMonitor::print(const std::string& first_line, const std::string& second_line) const {
    lcdLoc(fd, LINE1);
    typeln(fd, first_line.c_str());

    lcdLoc(fd, LINE2);
    typeln(fd, second_line.c_str()); 
}

void DisplayMonitor::print_temps(float internal_temp, float reference_temp, float external_temp) const {
    char line[LINE_SIZE];

    sprintf(line, "TI:%0.1f TR:%0.1f", internal_temp, reference_temp);
    std::string first_line(line);

    sprintf(line, "TE: %0.2f", external_temp);
    std::string second_line(line);

    print(first_line, second_line);
}
