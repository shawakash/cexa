#include "common/interface.hpp"
#include "observer.hpp"
#include "http.hpp"
#include <string>
#include <sstream>
#include <iomanip>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class DiscordObserver : public IObserver {
    private:
        HttpClient http;
        std::string webhookUrl;

        std::string formatMessage(const Arber& opportunity) {
            std::stringstream ss;
            ss << "🤖 **New Arbitrage Opportunity**\n"
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
        DiscordObserver(const std::string &url) : webhookUrl(url) {}

        void onArbitrageOpportunity(const Arber& opportunity) override {
            if (webhookUrl.empty()) return;

            try {
                if (!opportunity.getExecute()) return;

                json payload;
                payload["content"] = formatMessage(opportunity);

                payload["username"] = "Arbitrage Bot";
                // payload["avatar_url"] = "https://i.imgur.com/your-bot-avatar.png";

                HttpRequestOptions options;
                options.method = HttpMethod::POST;
                options.headers["Content-Type"] = "application/json";
                options.body = payload;

                HttpResponse response = http.fetch(webhookUrl, options);

                if (response.statusCode != 204) {
                    std::cerr << "Failed to send Discord notification [Status: " << response.statusCode
                                << "]: " << response.body << std::endl;
                    std::cerr << "Request body was: " << payload.dump() << std::endl;
                } else {
                    std::cout << "Notified Discord ...\n" << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error sending Discord notification: " << e.what() << std::endl;
            }
        }
};
