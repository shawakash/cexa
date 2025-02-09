#include "risk.hpp"
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
