#include "interface.hpp"
#include <fstream>

class ExchangeDecorator : public IExchange {
    protected:
        IExchange* exchange;

    public:
        ExchangeDecorator(IExchange* exchange) : exchange(exchange) {
            this->url = exchange->url;
            this->name = exchange->name;
        }

        virtual BBO getBBO(Token base, Token quote) override {
            return exchange->getBBO(base, quote);
        }

        virtual std::string getTicker(Token& base, Token& quote) override {
            return exchange->getTicker(base, quote);
        }

        virtual ~ExchangeDecorator() {
            delete exchange;
        }
};

class LoggingDecorator : public ExchangeDecorator {
    private:
        std::ofstream logFile;

    public:
        LoggingDecorator(IExchange* exchange) : ExchangeDecorator(exchange) {
            logFile.open("exchange_logs.txt", std::ios::app);
        }

        BBO getBBO(Token base, Token quote) override {
            BBO bbo = exchange->getBBO(base, quote);

            logFile << "[" << std::time(nullptr) << "] "
                    << name << " " << base << quote
                    << " Bid: " << bbo.bid.price << "@" << bbo.bid.size
                    << " Ask: " << bbo.ask.price << "@" << bbo.ask.size
                    << std::endl;

            return bbo;
        }

        ~LoggingDecorator() {
            logFile.close();
        }
};

class LatencyDecorator : public ExchangeDecorator {
    private:
        std::ofstream logFile;

    public:
        LatencyDecorator(IExchange* exchange) : ExchangeDecorator(exchange) {
            logFile.open("exchange_logs.txt", std::ios::app);
        }

        BBO getBBO(Token base, Token quote) override {
            auto start = std::chrono::high_resolution_clock::now();

            BBO bbo = exchange->getBBO(base, quote);

            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

            logFile << "[LATENCY] " << name << " request took "
                    << duration.count() << "ms" << std::endl;

            return bbo;
        }

        ~LatencyDecorator() {
            logFile.close();
        }
};

class ArbLogDecorator {
    private:
        std::ofstream logFile;

    public:
        ArbLogDecorator() {
            logFile.open(
                "arbitrage_logs.txt",
                std::ios::app
            );
        }

        void logOpportunity(const Arber& arb) {
            std::string timestamp = std::to_string(std::time(nullptr));

            logFile << "[" << timestamp << "] "
                    << "Buy: " << arb.buyExchange
                    << " @ " << arb.buyBBO.ask.price
                    << " Sell: " << arb.sellExchange
                    << " @ " << arb.sellBBO.bid.price
                    << " Amount: " << arb.amount
                    << " Spread: " << (arb.sellBBO.bid.price - arb.buyBBO.ask.price)
                    << " Profit: " << arb.profit << " %"
                    << std::endl;

            // Also print to console
            std::cout << "\n=== Arbitrage Opportunity Found! ===" << std::endl;
            std::cout << "Buy from: " << arb.buyExchange
                        << " at " << arb.buyBBO.ask.price << std::endl;
            std::cout << "Sell to: " << arb.sellExchange
                        << " at " << arb.sellBBO.bid.price << std::endl;
            std::cout << "Amount: " << arb.amount << std::endl;
            std::cout << "Spread: " << (arb.sellBBO.bid.price - arb.buyBBO.ask.price) << std::endl
            << "Profit: " << arb.profit << " %" << std::endl;
            std::cout << "==============================\n" << std::endl;
        }

        ~ArbLogDecorator() {
            logFile.close();
        }
};


class ArbLatencyDecorator {
    private:
        std::ofstream logFile;

    public:
        ArbLatencyDecorator() {
            logFile.open("arbitrage_latency.txt", std::ios::app);
        }

        auto start() {
            return std::chrono::high_resolution_clock::now();
        }

        void end(std::chrono::time_point<std::chrono::high_resolution_clock> start_time) {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_time);

            std::string timestamp = std::to_string(std::time(nullptr));

            logFile << "[" << timestamp << "] "
                    << "Scan duration: " << duration.count() << "ms" << std::endl;

            std::cout << "Scan completed in " << duration.count() << "ms" << std::endl;
        }

        ~ArbLatencyDecorator() {
            logFile.close();
        }
};
