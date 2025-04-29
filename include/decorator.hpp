#pragma once

#include "common/Gateway.hpp"
#include "utils/logs.hpp"

#include <fstream>
#include <ctime>
#include <iostream>
#include <chrono>

class GatewayDecorator : public Gateway {
    protected:
        Gateway* gw;

    public:
        GatewayDecorator(Gateway* gw) : gw(gw) {
            this->url = gw->url;
            this->name = gw->name;
        }

        virtual BBO getBBO(Token base, Token quote) override {
            return gw->getBBO(base, quote);
        }

        virtual std::string getTicker(Token& base, Token& quote) override {
            return gw->getTicker(base, quote);
        }

        virtual ~GatewayDecorator() {
            delete gw;
        }
};

class LoggingDecorator : public GatewayDecorator {
    private:
        std::ofstream logFile;

        void checkAndClearLog() {
            if (shouldClearLog()) {
                logFile.close();
                logFile.open("exchange_logs.txt", std::ios::trunc);
                logFile.close();
                logFile.open("exchange_logs.txt", std::ios::app);
            }
        }

    public:
        LoggingDecorator(Gateway* gw) : GatewayDecorator(gw) {
            logFile.open("exchange_logs.txt", std::ios::app);
        }

        BBO getBBO(Token base, Token quote) override {
            checkAndClearLog();

            BBO bbo = gw->getBBO(base, quote);

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

class LatencyDecorator : public GatewayDecorator {
    private:
        std::ofstream logFile;

    public:
        LatencyDecorator(Gateway* gw) : GatewayDecorator(gw) {
            logFile.open("exchange_logs.txt", std::ios::app);
        }

        BBO getBBO(Token base, Token quote) override {
            auto start = std::chrono::high_resolution_clock::now();

            BBO bbo = gw->getBBO(base, quote);

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

        void checkAndClearLog() {
            if (shouldClearLog()) {
                logFile.close();
                logFile.open("arbitrage_logs.txt", std::ios::trunc);
                logFile.close();
                logFile.open("arbitrage_logs.txt", std::ios::app);
            }
        }

    public:
        ArbLogDecorator() {
            logFile.open(
                "arbitrage_logs.txt",
                std::ios::app
            );
        }

        void logOpportunity(const Arber& arb) {
            checkAndClearLog();
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

        void logRiskCheckFailed(const Arber& arb) {
            checkAndClearLog();
            std::string timestamp = std::to_string(std::time(nullptr));

            logFile << "[" << timestamp << "] RISK CHECK FAILED - "
                    << "Buy: " << arb.buyExchange
                    << " Sell: " << arb.sellExchange
                    << " Amount: " << arb.amount
                    << " Profit: " << arb.profit << "%"
                    << std::endl;

            std::cout << "\n=== Risk Check Failed ===" << std::endl;
            std::cout << "Trade rejected due to risk parameters" << std::endl;
            std::cout << "=====================\n" << std::endl;
        }

        ~ArbLogDecorator() {
            logFile.close();
        }
};


class ArbLatencyDecorator {
    private:
        std::ofstream logFile;

        void checkAndClearLog() {
            if (shouldClearLog()) {
                logFile.close();
                logFile.open("arbitrage_latency.txt", std::ios::trunc);
                logFile.close();
                logFile.open("arbitrage_latency.txt", std::ios::app);
            }
        }

    public:
        ArbLatencyDecorator() {
            logFile.open("arbitrage_latency.txt", std::ios::app);
        }

        auto start() {
            return std::chrono::high_resolution_clock::now();
        }

        void end(std::chrono::time_point<std::chrono::high_resolution_clock> start_time) {
            checkAndClearLog();
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
