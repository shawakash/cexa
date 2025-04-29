#include "common/Gateway.hpp"
#include "common/Instrument.hpp"
#include "common/AsyncHttp.hpp"

#include <cstdint>
#include <exception>
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class ByBitGateway : public Gateway {
    public:
        ByBitGateway(std::string url = "https://api.bybit.com/v5") {
            this->url = url;
            this->name = Exchange::BYBIT;
        }

        std::string getTicker(Token& buyToken, Token& sellToken) override {
            std::stringstream ss;
            ss << buyToken << sellToken;
            return ss.str();
        }

        BBO getBBO(Token buyToken, Token sellToken) override {
            try {
                auto& http = getHttp();

                const std::string depthsUrl = this->url + "/market/orderbook?category=spot&symbol=" + getTicker(buyToken, sellToken);
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

                double bidPrice = std::stod(data["result"]["b"][0][0].get<std::string>());
                double bidSize = std::stod(data["result"]["b"][0][1].get<std::string>());

                double askPrice = std::stod(data["result"]["a"][0][0].get<std::string>());
                double askSize = std::stod(data["result"]["a"][0][1].get<std::string>());

                bbo.bid = PriceLevel{bidPrice, bidSize};
                bbo.ask = PriceLevel{askPrice, askSize};

                bbo.timestamp = data["time"];

                return bbo;

            } catch(const std::exception& e) {
                std::cerr << "[ERROR] Exception fetching BBO for " << this->name << " details: " << e.what() << std::endl;
                return BBO();
            }
        }

};
