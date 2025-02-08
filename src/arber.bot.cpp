#include "interface.hpp"
#include <vector>
#include "decorator.cpp"
#include <thread>
#include <chrono>
#include <csignal>

class ArbitrageBot {
    private:
        std::vector<IExchange*> exchanges;
        double maxTradeAmount;
        double minProfit;

        bool running;

        ArbLogDecorator logger;
        ArbLatencyDecorator latencyMonitor;

        Arber findArbitrage(Token base, Token quote) {
            Arber bestArb(Exchange::BINANCE, Exchange::BINANCE, 0, 0, BBO(), BBO(), false);

            for (IExchange* buyEx : exchanges) {
                BBO buyBBO = buyEx->getBBO(base, quote);

                for (IExchange* sellEx : exchanges) {
                    if (buyEx == sellEx) continue;

                    BBO sellBBO = sellEx->getBBO(base, quote);

                    double profit = (sellBBO.bid.price - buyBBO.ask.price) / buyBBO.ask.price * 100;

                    // if (profit > minProfit && profit > bestArb.profit) {
                    if (sellBBO.bid.price > buyBBO.ask.price) {
                        double amount = std::min({
                            buyBBO.ask.size * buyBBO.ask.price,
                            sellBBO.bid.size * sellBBO.bid.price
                        });

                        bestArb = Arber(
                            buyEx->name,
                            sellEx->name,
                            profit,
                            amount,
                            buyBBO,
                            sellBBO,
                            true
                        );
                    }
                }
            }

            return bestArb;
        }

    public:
        ArbitrageBot(double minProfit, double maxTradeAmount)
            : minProfit(minProfit), maxTradeAmount(maxTradeAmount), running(true) {}

        void addExchange(IExchange* exchange) {
            exchanges.push_back(exchange);
        }

        void stop() {
            running = false;
        }

        void run(Token base, Token quote, int scanInterval = 1000) {
            std::cout << "Starting arbitrage scanner..." << std::endl;

            while (running) {
                auto start_time = latencyMonitor.start();

                Arber opportunity = findArbitrage(base, quote);

                if (opportunity.getExecute()) {
                    logger.logOpportunity(opportunity);
                }

                latencyMonitor.end(start_time);

                std::this_thread::sleep_for(std::chrono::milliseconds(scanInterval));
            }
        }

        Arber scan(Token base, Token quote) {
            return findArbitrage(base, quote);
        }

        ~ArbitrageBot() {
            for (auto* exchange : exchanges) {
                delete exchange;
            }
        }
};
