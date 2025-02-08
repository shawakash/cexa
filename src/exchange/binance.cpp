#include "../interface.hpp"
#include <cstdint>
#include <exception>
#include <vector>
#include <chrono>
#include <sstream>

struct BinanceDepths {
    uint64_t lastUpdateId;
    std::vector<std::pair<std::string, std::string> > bids;
    std::vector<std::pair<std::string, std::string> > asks;
};

class BinanceTool : public IExchange {
    public:
        BinanceTool(std::string url = "https://api.binance.com/api/v3") {
            this->url = url;
            this->name = Exchange::BINANCE;
        }

        std::string getTicker(Token& base, Token& quote) override {
            std::stringstream ss;
            ss << base << quote;
            return ss.str();
        }

        BBO getBBO(Token base, Token quote) override {
            try {
                auto& http = getHttp();

                const std::string depthsUrl = this->url + "/depth?symbol=" + getTicker(base, quote);
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
