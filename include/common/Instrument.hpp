#pragma once

#include <string>
#include <iostream>
#include <unordered_map>
#include <stdexcept>
#include <vector>

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

const std::unordered_map<std::string, Exchange> exchangeFromString = {
    {"BINANCE", Exchange::BINANCE},
    {"BYBIT", Exchange::BYBIT},
    {"DYDX", Exchange::DYDX},
    {"COINBASE", Exchange::COINBASE},
    {"OKX", Exchange::OKX},
};

const std::unordered_map<std::string, Token> tokenFromString = {
    {"BTC", Token::BTC},
    {"USDC", Token::USDC},
    {"USDT", Token::USDT},
    {"ETH", Token::ETH},
};

const std::unordered_map<std::string, FeedType> feedTypeFromString = {
    {"SWAP", FeedType::SWAP},
    {"OPTIONS", FeedType::OPTIONS},
    {"FUTURES", FeedType::FUTURES},
    {"SPOT", FeedType::SPOT},
    {"UNKNOWN", FeedType::UNKNOWN}
};

const std::unordered_map<std::string, FutureExpiration> futureExpirationFromString = {
    {"NOT_A_FUTURE", FutureExpiration::NOT_A_FUTURE},
    {"WEEKLY", FutureExpiration::WEEKLY},
    {"BIWEEKLY", FutureExpiration::BIWEEKLY},
    {"QUATERLY", FutureExpiration::QUATERLY},
    {"BIQUATERLY", FutureExpiration::BIQUATERLY},
    {"PERP", FutureExpiration::PERP}
};

inline Exchange exchangeFromStr(const std::string& ex) {
    auto it = exchangeFromString.find(ex);
    if (it != exchangeFromString.end()) {
        return it->second;
    }
    throw std::invalid_argument("Invalid exchange string: " + ex);
}

inline Token tokenFromStr(const std::string& tk) {
    auto it = tokenFromString.find(tk);
    if (it != tokenFromString.end()) {
        return it->second;
    }

    throw std::invalid_argument("Invalid token string: " + tk);
}

inline FeedType feedTypeFromStr(const std::string& ft) {
    auto it = feedTypeFromString.find(ft);
    if (it != feedTypeFromString.end()) {
        return it->second;
    }
    throw std::invalid_argument("Invalid feed type string: " + ft);
}

inline FutureExpiration futureExpirationFromStr(const std::string& fe) {
    auto it = futureExpirationFromString.find(fe);
    if (it != futureExpirationFromString.end()) {
        return it->second;
    }
    throw std::invalid_argument("Invalid future expiration string: " + fe);
}

inline std::ostream& operator<<(std::ostream& os, Exchange ex) {
    switch (ex) {
        case Exchange::BINANCE: return os << "BINANCE";
        case Exchange::BYBIT: return os << "BYBIT";
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

inline std::ostream& operator<<(std::ostream& os, FeedType ft) {
    switch (ft) {
        case FeedType::SWAP: return os << "SWAP";
        case FeedType::OPTIONS: return os << "OPTIONS";
        case FeedType::FUTURES: return os << "FUTURES";
        case FeedType::SPOT: return os << "SPOT";
        case FeedType::UNKNOWN: return os << "UNKNOWN";
    }
    return os;
}

inline std::ostream& operator<<(std::ostream& os, FutureExpiration fe) {
    switch (fe) {
        case FutureExpiration::NOT_A_FUTURE: return os << "NOT_A_FUTURE";
        case FutureExpiration::WEEKLY: return os << "WEEKLY";
        case FutureExpiration::BIWEEKLY: return os << "BIWEEKLY";
        case FutureExpiration::QUATERLY: return os << "QUATERLY";
        case FutureExpiration::BIQUATERLY: return os << "BIQUATERLY";
        case FutureExpiration::PERP: return os << "PERP";
    }
    return os;
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
        Exchange ex = exchangeFromStr(exchangeStr);

        size_t slash = str.find('/', firstColon + 1);
        if (slash == std::string::npos) {
            throw std::invalid_argument("Invalid instrument string: missing slash");
        }

        std::string baseStr = str.substr(firstColon + 1, slash - firstColon - 1);
        Token base = tokenFromStr(baseStr);

        size_t secondColon = str.find(':', slash + 1);
        if (secondColon == std::string::npos) {
            throw std::invalid_argument("Invalid instrument string: missing second colon");
        }

        std::string quoteStr = str.substr(slash + 1, secondColon - slash - 1);
        Token quote = tokenFromStr(quoteStr);

        std::string typeStr = str.substr(secondColon + 1);

        try {
            FeedType ft = feedTypeFromStr(typeStr);
            return Instrument(base, quote, ex, ft);
        } catch (const std::invalid_argument&) {
            try {
                FutureExpiration fe = futureExpirationFromStr(typeStr);
                return Instrument(base, quote, ex, FeedType::FUTURES, fe);
            } catch (const std::invalid_argument&) {
                throw std::invalid_argument("Invalid feed type or future expiration: " + typeStr);
            }
        }
    }

    std::string toString() const {
        std::string baseStr = std::string(exchange_to_string(exchange)) + ":" +
                              token_to_string(baseSymbol) + "/" + token_to_string(quoteSymbol);

        if (futureExpiration != FutureExpiration::NOT_A_FUTURE) {
            return baseStr + ":" + future_expiration_to_string(futureExpiration);
        } else {
            return baseStr + ":" + feed_type_to_string(feedType);
        }
    }

    static const char* exchange_to_string(Exchange ex) {
        switch (ex) {
            case Exchange::BINANCE: return "BINANCE";
            case Exchange::BYBIT: return "BYBIT";
            case Exchange::DYDX: return "DYDX";
            case Exchange::COINBASE: return "COINBASE";
            case Exchange::OKX: return "OKX";
            default: return "UNKNOWN";
        }
    }

    static const char* token_to_string(Token token) {
        switch (token) {
            case Token::BTC: return "BTC";
            case Token::ETH: return "ETH";
            case Token::USDC: return "USDC";
            case Token::USDT: return "USDT";
            default: return "UNKNOWN";
        }
    }

    static const char* feed_type_to_string(FeedType ft) {
        switch (ft) {
            case FeedType::SWAP: return "SWAP";
            case FeedType::OPTIONS: return "OPTIONS";
            case FeedType::FUTURES: return "FUTURES";
            case FeedType::SPOT: return "SPOT";
            case FeedType::UNKNOWN: return "UNKNOWN";
            default: return "UNKNOWN";
        }
    }

    static const char* future_expiration_to_string(FutureExpiration fe) {
        switch (fe) {
            case FutureExpiration::NOT_A_FUTURE: return "NOT_A_FUTURE";
            case FutureExpiration::WEEKLY: return "WEEKLY";
            case FutureExpiration::BIWEEKLY: return "BIWEEKLY";
            case FutureExpiration::QUATERLY: return "QUATERLY";
            case FutureExpiration::BIQUATERLY: return "BIQUATERLY";
            case FutureExpiration::PERP: return "PERP";
            default: return "UNKNOWN";
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const Instrument& instr) {
        return os << instr.toString();
    }
};
