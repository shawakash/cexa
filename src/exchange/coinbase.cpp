#include "../interface.hpp"
#include <cstdint>
#include <exception>
#include <vector>
#include <chrono>
#include <sstream>

class CoinBaseTool : public IExchange {
    public:
        CoinBaseTool(std::string url = "https://api.exchange.coinbase.com/products") {
            this->url = url;
            this->name = Exchange::COINBASE;
        }

        std::string getTicker(Token& base, Token& quote) override {
            std::stringstream ss;
            if (base == Token::USDC || base == Token::USDT) {
                ss << "USD-" << quote ;
            } else if (quote == Token::USDC || quote == Token::USDT) {
                ss << base <<"-USD";
            } else {
                ss << base << "-" << quote;
            }
            return ss.str();
        }

        BBO getBBO(Token base, Token quote) override {
            try {
                auto& http = getHttp();

                const std::string depthsUrl = this->url + "/" + getTicker(base, quote) + "/book";
                HttpRequestOptions options;
                options.headers = {
                    {"Content-Type", "application/json"},
                    {"User-Agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/132.0.0.0 Safari/537.36"}
                };

                HttpResponse res = http.fetch(depthsUrl, options);

                if (res.statusCode != 200) {
                    std::cerr << "[ERROR] Exception fetching BBO for " << this->name << " details: " << res.body << std::endl;
                    return BBO();
                }

                json data = json::parse(res.body);
                BBO bbo;

                double bidPrice = std::stod(data["bids"][0][0].get<std::string>());
                double bidSize = std::stod(data["bids"][0][1].get<std::string>());

                double askPrice = std::stod(data["asks"][0][0].get<std::string>());
                double askSize = std::stod(data["asks"][0][1].get<std::string>());

                bbo.bid = PriceLevel{bidPrice, bidSize};
                bbo.ask = PriceLevel{askPrice, askSize};

                auto now = std::chrono::system_clock::now();
                bbo.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now.time_since_epoch()
                ).count();

                return bbo;

            } catch(const std::exception& e) {
                std::cerr << "[ERROR] Exception fetching BBO for " << this->name << " details: " << e.what() << std::endl;
                return BBO();
            }
        }

};
