#pragma once
#include "common/interface.hpp"

class IObserver {
public:
    virtual void onArbitrageOpportunity(const Arber& opportunity) = 0;
    virtual ~IObserver() = default;
};
