#pragma once

#include <iostream>

#include <cstring>
#include <cerrno>

namespace utils {
    std::string append_strerror(const std::string& error_msg);
}
