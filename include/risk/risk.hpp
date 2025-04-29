#pragma once
#include "common/interface.hpp"

struct RiskMetrics {
    double maxDrawdown;
    double dailyVolume;
    double exposurePerTrade;
    double totalExposure;
    double profitLoss;
};

class IRiskStrategy {
    public:
        virtual bool validateTrade(const Arber& opportunity, const RiskMetrics& metrics) = 0;
        virtual ~IRiskStrategy() = default;
};

class MaxExposureStrategy : public IRiskStrategy {
    private:
        double maxExposureLimit;

    public:
        MaxExposureStrategy(double limit):
            maxExposureLimit(limit) {}

        bool validateTrade(const Arber& opportunity, const RiskMetrics& metrics) override {
            double newExposure = metrics.totalExposure + opportunity.amount;
            return newExposure <= maxExposureLimit;
        }
};

class DrawdownStrategy : public IRiskStrategy {
    private:
        double maxDrawdownLimit;

    public:
        DrawdownStrategy(double limit) : maxDrawdownLimit(limit) {}

        bool validateTrade(const Arber& opportunity, const RiskMetrics& metrics) override {
            return metrics.maxDrawdown <= maxDrawdownLimit;
        }
};

class VolatilityStrategy : public IRiskStrategy {
private:
    double maxVolatility;

public:
    VolatilityStrategy(double limit) : maxVolatility(limit) {}

    bool validateTrade(const Arber& opportunity, const RiskMetrics& metrics) override {
        double spread = opportunity.sellBBO.bid.price - opportunity.buyBBO.ask.price;
        return std::abs(spread) <= maxVolatility;
    }
};
