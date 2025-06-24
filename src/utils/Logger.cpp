#include "../../include/utils/Logger.hpp"
#include <iostream>

void Logger::log(const std::string& message) {
    std::cout << message << std::endl;
}