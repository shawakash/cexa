#include "common/Gateway.hpp"
#include "common/Instrument.hpp"
#include "common/AsyncHttp.hpp"

#include <cstdint>
#include <exception>
#include <iostream>
#include <string>
#include <chrono>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class CoinbaseGateway : public Gateway {
    public:
        CoinbaseGateway(std::string url = "https://api.exchange.coinbuyToken.com/products") {
            this->url = url;
            this->name = Exchange::COINBASE;
        }

        std::string getTicker(Token& buyToken, Token& sellToken) override {
            std::stringstream ss;
            if (buyToken == Token::USDC || buyToken == Token::USDT) {
                ss << "USD-" << sellToken ;
            } else if (sellToken == Token::USDC || sellToken == Token::USDT) {
                ss << buyToken <<"-USD";
            } else {
                ss << buyToken << "-" << sellToken;
            }
            return ss.str();
        }

        BBO getBBO(Token buyToken, Token sellToken) override {
            try {
                auto& http = getHttp();

                const std::string depthsUrl = this->url + "/" + getTicker(buyToken, sellToken) + "/book";
                std::map<std::string, std::string> headers = {
                    {"Accept", "application/json"}
                };

                auto response_future = http.get_raw(depthsUrl, headers);
                auto res = response_future.get();

                if (res.status_code != 200) {
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
