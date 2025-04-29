#include "common/Arber.hpp"
#include "common/Gateway.hpp"
#include "common/Instrument.hpp"
#include "observer.hpp"
#include "risk/risk.hpp"
#include "decorator.hpp"

#include <memory>
#include <vector>
#include <thread>
#include <chrono>
#include <csignal>

class ArbitrageBot {
    private:
        std::vector<Gateway*> gws;
        double maxTradeAmount;
        double minProfit;

        bool running;

        ArbLogDecorator logger;
        ArbLatencyDecorator latencyMonitor;

        std::vector<std::unique_ptr<IObserver>> observers;

        RiskManager riskManager;
        RiskMetrics currentMetrics;

        Arber findArbitrage(Token buyToken, Token sellToken) {
            Arber bestArb(buyToken, sellToken, Exchange::BINANCE, Exchange::BINANCE, 0, 0, BBO(), BBO(), false);

            for (Gateway* buyExGw : gws) {
                BBO buyBBO = buyExGw->getBBO(buyToken, sellToken);

                for (Gateway* sellExGw : gws) {
                    if (buyExGw == sellExGw) continue;

                    BBO sellBBO = sellExGw->getBBO(buyToken, sellToken);

                    double profit = (sellBBO.bid.price - buyBBO.ask.price) / buyBBO.ask.price * 100;

                    // if (profit > minProfit && profit > bestArb.profit) {
                    if (sellBBO.bid.price > buyBBO.ask.price) {
                        double amount = std::min({
                            buyBBO.ask.size * buyBBO.ask.price,
                            sellBBO.bid.size * sellBBO.bid.price
                        });

                        bestArb = Arber(
                            buyToken,
                            sellToken,
                            buyExGw->name,
                            sellExGw->name,
                            profit,
                            amount,
                            buyBBO,
                            sellBBO,
                            true
                        );

                        // set the risk calculator accordingly to use this
                        // if (bestArb.getExecute()) {
                            // if (!riskManager.validateArbitrage(bestArb)) {
                                // bestArb = Arber(
                                //     bestArb.buyExchange,
                                //     bestArb.sellExchange,
                                //     bestArb.profit,
                                //     bestArb.amount,
                                //     bestArb.buyBBO,
                                //     bestArb.sellBBO,
                                //     true  // Set execute to false if risk check fails
                                // );
                                // logger.logRiskCheckFailed(bestArb);
                            // }
                        // }
                    }
                }
            }

            return bestArb;
        }

        void notifyObservers(const Arber& opportunity) {
            for (const auto& observer : observers) {
                observer->onArbitrageOpportunity(opportunity);
            }
        }

    public:
        ArbitrageBot(double minProfit, double maxTradeAmount)
            : minProfit(minProfit), maxTradeAmount(maxTradeAmount), running(true) {
            // Initialize risk strategies
            riskManager.addStrategy(new MaxExposureStrategy(100000)); // $100k max exposure
            riskManager.addStrategy(new DrawdownStrategy(0.05));      // 5% max drawdown
            riskManager.addStrategy(new VolatilityStrategy(0.01));    // 1% max volatility

            // Initialize metrics
            currentMetrics = {
                .maxDrawdown = 0.0,
                .dailyVolume = 0.0,
                .exposurePerTrade = 0.0,
                .totalExposure = 0.0,
                .profitLoss = 0.0
            };
        }

        void addObserver(std::unique_ptr<IObserver> observer) {
            observers.push_back(std::move(observer));
        }

        void updateRiskMetrics(const RiskMetrics& metrics) {
            currentMetrics = metrics;
            riskManager.updateMetrics(metrics);
        }

        void addExchange(Gateway* gw) {
            gws.push_back(gw);
        }

        void stop() {
            running = false;
            for (auto* gw : gws) {
                gw->destroy();
            }
        }

        void run(Token buyToken, Token sellToken, int scanInterval = 1000) {
            std::cout << "Starting arbitrage scanner..." << std::endl;

            while (running) {
                auto start_time = latencyMonitor.start();

                Arber opportunity = findArbitrage(buyToken, sellToken);

                if (opportunity.getExecute()) {
                    logger.logOpportunity(opportunity);
                    notifyObservers(opportunity);
                }

                latencyMonitor.end(start_time);

                std::this_thread::sleep_for(std::chrono::milliseconds(scanInterval));
            }
        }

        Arber scan(Token buyToken, Token sellToken) {
            return findArbitrage(buyToken, sellToken);
        }

        ~ArbitrageBot() {
            // TODO: Bad Each gw should be handles indiviual
        }
};
