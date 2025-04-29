#pragma once

#include "common/Arber.hpp"

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

#include "risk/risk.hpp"
#include <vector>

class RiskManager {
private:
    std::vector<IRiskStrategy*> strategies;
    RiskMetrics metrics;

public:
    RiskManager() {
        // Initialize with default metrics
        metrics = {
            .maxDrawdown = 0.0,
            .dailyVolume = 0.0,
            .exposurePerTrade = 0.0,
            .totalExposure = 0.0,
            .profitLoss = 0.0
        };
    }

    void addStrategy(IRiskStrategy* strategy) {
        strategies.push_back(strategy);
    }

    void updateMetrics(const RiskMetrics& newMetrics) {
        metrics = newMetrics;
    }

    bool validateArbitrage(const Arber& opportunity) {
        for (auto* strategy : strategies) {
            if (!strategy->validateTrade(opportunity, metrics)) {
                return false;
            }
        }
        return true;
    }

    ~RiskManager() {
        for (auto* strategy : strategies) {
            delete strategy;
        }
    }
};
