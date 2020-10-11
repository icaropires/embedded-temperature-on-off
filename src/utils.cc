#include "utils.h"

std::string utils::append_strerror(const std::string& error_msg) {
    auto strerror = std::strerror(errno);

    return error_msg + ": " + std::string(strerror);
}
