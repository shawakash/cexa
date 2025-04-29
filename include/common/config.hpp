#pragma once

#include <cstdint>

struct PriceLevel {
    double price;
    double size;
};

struct BBO {
    PriceLevel bid;
    PriceLevel ask;
    uint64_t timestamp;
};
