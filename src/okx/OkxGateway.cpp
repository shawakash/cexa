#include "common/Gateway.hpp"
#include "common/Instrument.hpp"
#include "common/AsyncHttp.hpp"

#include <cstdint>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class OkxGateway : public Gateway {
    public:
        OkxGateway(std::string url = "https://www.okx.com/api/v5") {
            this->url = url;
            this->name = Exchange::OKX;
        }

        std::string getTicker(Token& buyToken, Token& sellToken) override {
            std::stringstream ss;
            ss << buyToken << "-" << sellToken;
            return ss.str();
        }

        BBO getBBO(Token buyToken, Token sellToken) override {
            try {
                auto& http = getHttp();

                const std::string depthsUrl = this->url + "/market/books?instId=" + getTicker(buyToken, sellToken);
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

                const auto& orderbook = data["data"][0];

                double bidPrice = std::stod(orderbook["bids"][0][0].get<std::string>());
                double bidSize = std::stod(orderbook["bids"][0][1].get<std::string>());

                double askPrice = std::stod(orderbook["asks"][0][0].get<std::string>());
                double askSize = std::stod(orderbook["asks"][0][1].get<std::string>());

                bbo.bid = PriceLevel{bidPrice, bidSize};
                bbo.ask = PriceLevel{askPrice, askSize};

                bbo.timestamp = std::stoull(orderbook["ts"].get<std::string>());

                return bbo;

            } catch(const std::exception& e) {
                std::cerr << "[ERROR] Exception fetching BBO for " << this->name << " details: " << e.what() << std::endl;
                return BBO();
            }
        }

};
