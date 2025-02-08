#include "../interface.hpp"
#include <cstdint>
#include <exception>
#include <sstream>

class OkxTool : public IExchange {
    public:
        OkxTool(std::string url = "https://www.okx.com/api/v5") {
            this->url = url;
            this->name = Exchange::OKX;
        }

        std::string getTicker(Token& base, Token& quote) override {
            std::stringstream ss;
            ss << base << "-" << quote;
            return ss.str();
        }

        BBO getBBO(Token base, Token quote) override {
            try {
                auto& http = getHttp();

                const std::string depthsUrl = this->url + "/market/books?instId=" + getTicker(base, quote);
                HttpRequestOptions options;
                options.headers = {
                    {"Accept", "application/json"},
                };

                HttpResponse res = http.fetch(depthsUrl, options);

                if (res.statusCode != 200) {
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
