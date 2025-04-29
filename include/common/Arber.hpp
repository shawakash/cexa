#pragma once

#include "AsyncHttp.hpp"
#include "Instrument.hpp"
#include "config.hpp"

#include <string>

class Arber {
    private:
        bool execute;

    public:
        Token buyToken;
        Token sellToken;
        Exchange buyExchange;
        Exchange sellExchange;
        double profit;
        double amount;
        BBO buyBBO;
        BBO sellBBO;

        Arber(
            Token buyToken,
            Token sellToken,
            Exchange buyExchange,
            Exchange sellExchange,
            double profit,
            double amount,
            BBO buyBBO,
            BBO sellBBO,
            bool execute = true
        ) : buyToken(buyToken), sellToken(sellToken),
            buyExchange(buyExchange), sellExchange(sellExchange),
            profit(profit), amount(amount),
            sellBBO(sellBBO), buyBBO(buyBBO), execute(execute) {}

        bool getExecute() const { return execute; }
};

class IExchange {
    private:
        AsyncHttp http;

    protected:
        AsyncHttp& getHttp() {return http;}

    public:
        std::string url;
        Exchange name;

        virtual BBO getBBO(Token buyToken, Token sellToken) = 0;
        virtual std::string getTicker(Token& base, Token& quote) = 0;

        virtual ~IExchange() = default;
};
