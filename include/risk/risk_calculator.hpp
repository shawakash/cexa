#pragma once
#include "common/interface.hpp"
#include "risk.hpp"
#include <numeric>

class RiskCalculator {
private:
    double initialBalance;
    double currentBalance;
    double dailyHighBalance;
    double dailyLowBalance;
    double dailyStartBalance;
    std::vector<double> trades;

public:
    RiskCalculator(double initialBalance = 100000.0)
        : initialBalance(initialBalance),
          currentBalance(initialBalance),
          dailyHighBalance(initialBalance),
          dailyLowBalance(initialBalance),
          dailyStartBalance(initialBalance) {}

    double calculateDrawdown() {
        if (dailyHighBalance == 0) return 0;
        return ((dailyHighBalance - currentBalance) / dailyHighBalance) * 100.0;
    }

    double calculateDailyVolume() {
        return std::accumulate(trades.begin(), trades.end(), 0.0);
    }

    double calculateExposure() {
        return currentBalance * 0.01;
    }

    double calculateTotalExposure() {
        return trades.empty() ? 0.0 : trades.back();
    }

    double calculatePnL() {
        return ((currentBalance - dailyStartBalance) / dailyStartBalance) * 100.0;
    }

    void updateBalance(double newBalance) {
        currentBalance = newBalance;
        dailyHighBalance = std::max(dailyHighBalance, newBalance);
        dailyLowBalance = std::min(dailyLowBalance, newBalance);
    }

    void addTrade(double amount) {
        trades.push_back(amount);
    }

    void resetDaily() {
        dailyStartBalance = currentBalance;
        dailyHighBalance = currentBalance;
        dailyLowBalance = currentBalance;
        trades.clear();
    }
};
