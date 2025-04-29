#pragma once

#include <string>
#include <sstream>
#include <ctime>
#include <iomanip>

inline bool shouldClearLog() {
    static std::string lastDate;

    time_t now = std::time(nullptr);
    struct tm* timeinfo = std::localtime(&now);

    std::ostringstream oss;
    oss << (1900 + timeinfo->tm_year)
        << std::setfill('0') << std::setw(2) << (timeinfo->tm_mon + 1)
        << std::setfill('0') << std::setw(2) << timeinfo->tm_mday;

    std::string currentDate = oss.str();
    if (lastDate != currentDate) {
        lastDate = currentDate;
        return true;
    }
    return false;
}
