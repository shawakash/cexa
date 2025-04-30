#pragma once

#include <string>

template<typename EnumType>
struct EnumTraits {
    static EnumType fromString(const std::string& str);
    static std::string toString(const EnumType value);
};
