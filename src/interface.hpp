#pragma once
#include <iostream>
#include <cstdint>
#include <string>
#include <ctime>
#include <format>
#include <vector>
#include "utils/http.hpp"

struct PriceLevel {
    double price;
    double size;
};

struct BBO {
    PriceLevel bid;
    PriceLevel ask;
    uint64_t timestamp;
};

enum class Exchange {
    BINANCE,
    BYBIT,
    AKIRA,
    DYDX,
    COINBASE,
    OKX
};

enum class Token {
    BTC,
    ETH,
    USDC,
    USDT
};

inline std::ostream& operator<<(std::ostream& os, Exchange ex) {
    switch (ex) {
        case Exchange::BINANCE: return os << "BINANCE";
        case Exchange::BYBIT: return os << "BYBIT";
        case Exchange::AKIRA: return os << "AKIRA";
        case Exchange::DYDX: return os << "DYDX";
        case Exchange::COINBASE: return os << "COINBASE";
        case Exchange::OKX: return os << "OKX";
    }
    return os;
}

inline std::ostream& operator<<(std::ostream& os, Token token) {
    switch (token) {
        case Token::BTC: return os << "BTC";
        case Token::ETH: return os << "ETH";
        case Token::USDC: return os << "USDC";
        case Token::USDT: return os << "USDT";
    }
    return os;
}

class Arber {
    private:
        bool execute;

    public:
        Exchange buyExchange;
        Exchange sellExchange;
        double profit;
        double amount;
        BBO buyBBO;
        BBO sellBBO;

        Arber(
            Exchange buyExchange,
            Exchange sellExchange,
            double profit,
            double amount,
            BBO buyBBO,
            BBO sellBBO,
            bool execute = true
        ) : buyExchange(buyExchange), sellExchange(sellExchange),
            profit(profit), amount(amount),
            sellBBO(sellBBO), buyBBO(buyBBO), execute(execute) {}

        bool getExecute() {return execute;}
};

class IExchange {
    private:
        HttpClient http;

    protected:
        HttpClient& getHttp() {return http;}

    public:
        std::string url;
        Exchange name;

        virtual BBO getBBO(Token base, Token quote) = 0;
        virtual std::string getTicker(Token& base, Token& quote) = 0;

        virtual ~IExchange() = default;
};
