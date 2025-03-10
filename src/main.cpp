#include "interface.hpp"
#include "exchange/binance.cpp"
#include "exchange/okx.cpp"
#include "exchange/coinbase.cpp"
#include "exchange/bybit.cpp"
#include "arber.bot.cpp"
#include "utils/env.hpp"
#include "utils/slack.cpp"
#include "utils/discord.cpp"
#include "risk/risk_calculator.hpp"
#include <csignal>

volatile sig_atomic_t stop_flag = 0;

void signal_handler(int sig) {
    stop_flag = 1;
}

int main() {
    // Set up signal handling
    signal(SIGINT, signal_handler);

    ArbitrageBot* bot = new ArbitrageBot(0.005, 1);  // 0.005% min profit, 0.001 BTC trade size
    RiskCalculator riskCalc(100000.0); // Initialize with $100k

    // Add exchanges with decorators
    bot->addExchange(
        new LatencyDecorator(
            new LoggingDecorator(
                new BinanceTool()
            )
        )
    );

    bot->addExchange(
        new LatencyDecorator(
            new LoggingDecorator(
                new ByBitTool()
            )
        )
    );

    bot->addExchange(
        new LatencyDecorator(
            new LoggingDecorator(
                new CoinbaseTool()
            )
        )
    );

    bot->addExchange(
        new LatencyDecorator(
            new LoggingDecorator(
                new OkxTool()
            )
        )
    );

    const std::string slackWebhookUrl = Environment::getVar("SLACK_WEBHOOK_URL", "https://hooks.slack.com/services/...");
    auto slackObserver = std::make_unique<SlackObserver>(slackWebhookUrl);
    bot->addObserver(std::move(slackObserver));

    // const std::string discordWebhookUrl = Environment::getVar("DISCORD_WEBHOOK_URL", "https://discord.com/api/webhooks/...");
    // auto discordObserver = std::make_unique<DiscordObserver>(discordWebhookUrl);
    // bot->addObserver(std::move(discordObserver));

    RiskMetrics updatedMetrics {
        .maxDrawdown = riskCalc.calculateDrawdown(),
        .dailyVolume = riskCalc.calculateDailyVolume(),
        .exposurePerTrade = riskCalc.calculateExposure(),
        .totalExposure = riskCalc.calculateTotalExposure(),
        .profitLoss = riskCalc.calculatePnL()
    };

    bot->updateRiskMetrics(updatedMetrics);

    std::cout << "Press Ctrl+C to stop the bot" << std::endl;

    std::thread bot_thread([&bot]() {
        bot->run(Token::BTC, Token::USDC, 1000);
    });

    std::thread risk_thread([&bot, &riskCalc]() {
        while (!stop_flag) {
            RiskMetrics metrics {
                .maxDrawdown = riskCalc.calculateDrawdown(),
                .dailyVolume = riskCalc.calculateDailyVolume(),
                .exposurePerTrade = riskCalc.calculateExposure(),
                .totalExposure = riskCalc.calculateTotalExposure(),
                .profitLoss = riskCalc.calculatePnL()
            };
            bot->updateRiskMetrics(metrics);
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    });

    while (!stop_flag) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    bot->stop();
    delete bot;

    bot_thread.join();
    risk_thread.join();

    std::cout << "\nBot stopped successfully" << std::endl;

    return 0;
}
