#include "../interface.hpp"
#include <cstdint>
#include <exception>
#include <string>
#include <sstream>

class ByBitTool : public IExchange {
    public:
        ByBitTool(std::string url = "https://api.bybit.com/v5") {
            this->url = url;
            this->name = Exchange::BYBIT;
        }

        std::string getTicker(Token& base, Token& quote) override {
            std::stringstream ss;
            ss << base << quote;
            return ss.str();
        }

        BBO getBBO(Token base, Token quote) override {
            try {
                auto& http = getHttp();

                const std::string depthsUrl = this->url + "/market/orderbook?category=spot&symbol=" + getTicker(base, quote);
                HttpRequestOptions options;
                options.headers = {
                    {"Accept", "application/json"}
                };

                HttpResponse res = http.fetch(depthsUrl, options);

                if (res.statusCode != 200) {
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
