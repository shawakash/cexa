enum class Exchange {
    BINANCE,
    BYBIT,
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

class Instrument {
    Token baseSymbol;
    Token quoteSymbol;
    Exchange exchange;

    Instrument(Token base, Token quote, Exchange exchange):
                baseSymbol(base), quoteSymbol(quote), exchange(exchange) {}

    void fromString(std::string str) {

    }
}
