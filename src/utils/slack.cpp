#include "../interface.hpp"
#include "../observer.hpp"
#include "http.hpp"
#include "env.hpp"
#include <string>
#include <sstream>
#include <iomanip>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class SlackBody {

};

class SlackObserver : public IObserver {
private:
    HttpClient http;
    std::string webhookUrl;

    std::string formatMessage(const Arber& opportunity) {
        std::stringstream ss;
        ss << "🤖 *New Arbitrage Opportunity*\n"
           << "```\n"
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
    SlackObserver(const std::string &url) : webhookUrl(url) {}

    void onArbitrageOpportunity(const Arber& opportunity) override {
        if (webhookUrl.empty()) return;

        try {
            if (!opportunity.getExecute()) return;

            json payload;
            payload["text"] = formatMessage(opportunity);

            HttpRequestOptions options;
            options.method = HttpMethod::POST;
            options.headers["Content-Type"] = "application/json";
            options.body = payload;

            std::cout << "Sending to Slack: " << payload.dump() << std::endl;

            HttpResponse response = http.fetch(webhookUrl, options);

            if (response.statusCode != 200) {
                std::cerr << "Failed to send Slack notification [Status: " << response.statusCode
                          << "]: " << response.body << std::endl;
                std::cerr << "Request body was: " << payload.dump() << std::endl;
            } else {
                std::cout << "Successfully sent notification to Slack" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error sending Slack notification: " << e.what() << std::endl;
        }
    }
};
