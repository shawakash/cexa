#include "common/Arber.hpp"
#include "common/AsyncHttp.hpp"
#include "observer.hpp"
#include "utils/env.hpp"

#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <iomanip>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class SlackBody {

};

class SlackObserver : public IObserver {
private:
    AsyncHttp http;
    std::string webhookUrl;

    std::string formatMessage(const Arber& opportunity) {
        std::stringstream ss;
        ss << "🤖 *New Arbitrage Opportunity*\n"
           << "```\n"
           << "Ticker: " << opportunity.buyToken << "->" << opportunity.sellToken << "\n"
           << "Buy Exchange: " << opportunity.buyExchange << "\n"
           << "Sell Exchange: " << opportunity.sellExchange << "\n"
           << "Profit: " << std::fixed << std::setprecision(4) << opportunity.profit << "%\n"
           << "Amount: $" << std::fixed << std::setprecision(2) << opportunity.amount << "\n"
           << "Buy Price: $" << opportunity.buyBBO.ask.price << "\n"
           << "Sell Price: $" << opportunity.sellBBO.bid.price << "\n"
           << "```";
        return ss.str();
    }

public:
    SlackObserver(const std::string &url) : webhookUrl(url) {
        http.init(2);
    }

    ~SlackObserver(){
        http.destroy();
    }

    void onArbitrageOpportunity(const Arber& opportunity) override {
        if (webhookUrl.empty()) return;

        try {
            if (!opportunity.getExecute()) return;

            json payload;
            payload["text"] = formatMessage(opportunity);

            std::map<std::string, std::string> headers = {
                {"Content-Type", "application/json"}
            };

            auto response_future = http.post_raw(webhookUrl, payload, headers);
            auto response = response_future.get();

            if (response.status_code != 200) {
                std::cerr << "Failed to send Slack notification [Status: " << response.status_code
                          << "]: " << response.body << std::endl;
                std::cerr << "Request body was: " << payload.dump() << std::endl;
            } else {
                std::cout << "Notified Slack ...\n" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error sending Slack notification: " << e.what() << std::endl;
        }
    }
};
