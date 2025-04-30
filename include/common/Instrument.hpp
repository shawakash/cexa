#pragma once

#include <string>
#include <iostream>
#include <unordered_map>
#include <stdexcept>
#include <vector>

#include "enum.hpp"

enum class Exchange {
    BINANCE,
    BYBIT,
    DYDX,
    COINBASE,
    OKX,
};

enum class Token {
    BTC,
    ETH,
    USDC,
    USDT
};

enum class FeedType {
    SWAP,
    OPTIONS,
    FUTURES,
    SPOT,
    UNKNOWN
};

enum class FutureExpiration {
    NOT_A_FUTURE,
    WEEKLY,
    BIWEEKLY,
    QUATERLY,
    BIQUATERLY,
    PERP
};

template<>
struct EnumTraits<Exchange> {
    static Exchange fromString(const std::string& str) {
        static const std::unordered_map<std::string, Exchange> mapping = {
            {"BINANCE", Exchange::BINANCE},
            {"BYBIT", Exchange::BYBIT},
            {"DYDX", Exchange::DYDX},
            {"COINBASE", Exchange::COINBASE},
            {"OKX", Exchange::OKX},
        };

        auto it = mapping.find(str);
        if (it != mapping.end()) {
            return it->second;
        }
        throw std::invalid_argument("Invalid exchange string: " + str);
    }

    static std::string toString(const Exchange value) {
        switch (value) {
            case Exchange::BINANCE: return "BINANCE";
            case Exchange::BYBIT: return "BYBIT";
            case Exchange::DYDX: return "DYDX";
            case Exchange::COINBASE: return "COINBASE";
            case Exchange::OKX: return "OKX";
            default: return "UNKNOWN";
        }
    }
};

template<>
struct EnumTraits<Token> {
    static Token fromString(const std::string& str) {
        static const std::unordered_map<std::string, Token> mapping = {
            {"BTC", Token::BTC},
            {"ETH", Token::ETH},
            {"USDC", Token::USDC},
            {"USDT", Token::USDT},
        };

        auto it = mapping.find(str);
        if (it != mapping.end()) {
            return it->second;
        }
        throw std::invalid_argument("Invalid token string: " + str);
    }

    static std::string toString(const Token value) {
        switch (value) {
            case Token::BTC: return "BTC";
            case Token::ETH: return "ETH";
            case Token::USDC: return "USDC";
            case Token::USDT: return "USDT";
            default: return "UNKNOWN";
        }
    }
};

template<>
struct EnumTraits<FeedType> {
    static FeedType fromString(const std::string& str) {
        static const std::unordered_map<std::string, FeedType> mapping = {
            {"SWAP", FeedType::SWAP},
            {"OPTIONS", FeedType::OPTIONS},
            {"FUTURES", FeedType::FUTURES},
            {"SPOT", FeedType::SPOT},
            {"UNKNOWN", FeedType::UNKNOWN}
        };

        auto it = mapping.find(str);
        if (it != mapping.end()) {
            return it->second;
        }
        throw std::invalid_argument("Invalid feed type string: " + str);
    }

    static std::string toString(const FeedType value) {
        switch (value) {
            case FeedType::SWAP: return "SWAP";
            case FeedType::OPTIONS: return "OPTIONS";
            case FeedType::FUTURES: return "FUTURES";
            case FeedType::SPOT: return "SPOT";
            case FeedType::UNKNOWN: return "UNKNOWN";
            default: return "UNKNOWN";
        }
    }
};

template<>
struct EnumTraits<FutureExpiration> {
    static FutureExpiration fromString(const std::string& str) {
        static const std::unordered_map<std::string, FutureExpiration> mapping = {
            {"NOT_A_FUTURE", FutureExpiration::NOT_A_FUTURE},
            {"WEEKLY", FutureExpiration::WEEKLY},
            {"BIWEEKLY", FutureExpiration::BIWEEKLY},
            {"QUATERLY", FutureExpiration::QUATERLY},
            {"BIQUATERLY", FutureExpiration::BIQUATERLY},
            {"PERP", FutureExpiration::PERP}
        };

        auto it = mapping.find(str);
        if (it != mapping.end()) {
            return it->second;
        }
        throw std::invalid_argument("Invalid future expiration string: " + str);
    }

    static std::string toString(const FutureExpiration value) {
        switch (value) {
            case FutureExpiration::NOT_A_FUTURE: return "NOT_A_FUTURE";
            case FutureExpiration::WEEKLY: return "WEEKLY";
            case FutureExpiration::BIWEEKLY: return "BIWEEKLY";
            case FutureExpiration::QUATERLY: return "QUATERLY";
            case FutureExpiration::BIQUATERLY: return "BIQUATERLY";
            case FutureExpiration::PERP: return "PERP";
            default: return "UNKNOWN";
        }
    }
};

inline std::ostream& operator<<(std::ostream& os, Exchange ex) {
    return os << EnumTraits<Exchange>::toString(ex);
}

inline std::ostream& operator<<(std::ostream& os, Token token) {
    return os << EnumTraits<Token>::toString(token);
}

inline std::ostream& operator<<(std::ostream& os, FeedType ft) {
    return os << EnumTraits<FeedType>::toString(ft);
}

inline std::ostream& operator<<(std::ostream& os, FutureExpiration fe) {
    return os << EnumTraits<FutureExpiration>::toString(fe);
}

inline Exchange exchangeFromStr(const std::string& ex) {
    return EnumTraits<Exchange>::fromString(ex);
}

inline Token tokenFromStr(const std::string& tk) {
    return EnumTraits<Token>::fromString(tk);
}

inline FeedType feedTypeFromStr(const std::string& ft) {
    return EnumTraits<FeedType>::fromString(ft);
}

inline FutureExpiration futureExpirationFromStr(const std::string& fe) {
    return EnumTraits<FutureExpiration>::fromString(fe);
}

class Instrument {
public:
    Token baseSymbol;
    Token quoteSymbol;
    Exchange exchange;
    FeedType feedType;
    FutureExpiration futureExpiration;

    Instrument(Token base, Token quote, Exchange ex,
               FeedType ft, FutureExpiration fe = FutureExpiration::NOT_A_FUTURE) :
        baseSymbol(base),
        quoteSymbol(quote),
        exchange(ex),
        feedType(ft),
        futureExpiration(fe) {}

    static Instrument fromString(const std::string& str) {
        std::vector<std::string> parts;

        size_t firstColon = str.find(':');
        if (firstColon == std::string::npos) {
            throw std::invalid_argument("Invalid instrument string: missing first colon");
        }

        std::string exchangeStr = str.substr(0, firstColon);
        Exchange ex = EnumTraits<Exchange>::fromString(exchangeStr);

        size_t slash = str.find('/', firstColon + 1);
        if (slash == std::string::npos) {
            throw std::invalid_argument("Invalid instrument string: missing slash");
        }

        std::string baseStr = str.substr(firstColon + 1, slash - firstColon - 1);
        Token base = EnumTraits<Token>::fromString(baseStr);

        size_t secondColon = str.find(':', slash + 1);
        if (secondColon == std::string::npos) {
            throw std::invalid_argument("Invalid instrument string: missing second colon");
        }

        std::string quoteStr = str.substr(slash + 1, secondColon - slash - 1);
        Token quote = EnumTraits<Token>::fromString(quoteStr);

        std::string typeStr = str.substr(secondColon + 1);

        try {
            FeedType ft = EnumTraits<FeedType>::fromString(typeStr);
            return Instrument(base, quote, ex, ft);
        } catch (const std::invalid_argument&) {
            try {
                FutureExpiration fe = EnumTraits<FutureExpiration>::fromString(typeStr);
                return Instrument(base, quote, ex, FeedType::FUTURES, fe);
            } catch (const std::invalid_argument&) {
                throw std::invalid_argument("Invalid feed type or future expiration: " + typeStr);
            }
        }
    }

    std::string toString() const {
        std::string baseStr = EnumTraits<Exchange>::toString(exchange) + ":" +
                              EnumTraits<Token>::toString(baseSymbol) + "/" +
                              EnumTraits<Token>::toString(quoteSymbol);

        if (futureExpiration != FutureExpiration::NOT_A_FUTURE) {
            return baseStr + ":" + EnumTraits<FutureExpiration>::toString(futureExpiration);
        } else {
            return baseStr + ":" + EnumTraits<FeedType>::toString(feedType);
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const Instrument& instr) {
        return os << instr.toString();
    }
};
