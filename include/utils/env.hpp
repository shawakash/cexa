#pragma once

#include <string>
#include <cstdlib>
#include <stdexcept>

class Environment {
public:
    static std::string getVar(const std::string& key, const std::string& defaultValue = "") {
        const char* value = std::getenv(key.c_str());
        if (value == nullptr) {
            if (defaultValue.empty()) {
                throw std::runtime_error("Environment variable " + key + " not set");
            }
            return defaultValue;
        }
        return std::string(value);
    }

    static bool hasVar(const std::string& key) {
        return std::getenv(key.c_str()) != nullptr;
    }
};
