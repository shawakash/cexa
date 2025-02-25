# Crypto Arbitrage Bot

A high-performance cryptocurrency arbitrage bot that monitors multiple exchanges for price differences and identifies trading opportunities in real-time.

## Features

### Current Features
- **Multi-Exchange Support**:
  - Binance
  - ByBit
  - Coinbase
  - OKX
  - Easily extendable to more exchanges

- **Real-Time Monitoring**:
  - Continuous market scanning
  - Configurable scan intervals
  - Graceful shutdown handling

- **Decorator Pattern Implementation**:
  - Exchange Decorator: Base wrapper for exchange operations
  - Logging Decorator: Tracks all exchange interactions
  - Latency Decorator: Monitors performance metrics

- **Comprehensive Logging**:
  - Exchange operations logging
  - Arbitrage opportunities tracking
  - System latency monitoring
  - Performance metrics
  - Cleaning

- **Flexible Architecture**:
  - Modular design
  - Easy to extend and modify
  - Clean separation of concerns

- **Observation Pattern**:
  - Planned implementation for real-time notifications
  - Multiple notification channels
  - Include Slack
  - Future integrations with Discord, Telegram

### Example Opportunity
```plaintext
=== Arbitrage Opportunity Found! ===
Buy from: OKX at 96508.1
Sell to: COINBASE at 96516.4
Amount: 7047.61
Spread: 8.27
Profit: 0.00856923 %
==============================
```

### Planned Features

#### 1. Real-Time Notification System
- Implementation of Observer Pattern for opportunity notifications
- WebSocket integration for real-time updates
- Multiple notification channels (Discord, Telegram, Email)

#### 2. Order Execution Engine
- Smart order routing
- Position management
- Order splitting and aggregation
- Transaction cost analysis

#### 3. Risk Management System
- Position size limits
- Exchange exposure limits
- Price slippage protection
- Balance management
- Stop-loss mechanisms

## Technical Architecture

```plaintext
├── cexa/
│ └── src/
│     ├── exchange/
│     │   ├── binance.cpp
│     │   ├── bybit.cpp
│     │   ├── coinbase.cpp
│     │   └── okx.cpp
│     │
│     ├── utils/
│     │   ├── http.hpp
│     │   └── http.cpp
│     │
│     ├─ interface.hpp
│     ├── decorator.cpp
│     ├── arber.bot.cpp
│     ├── main.cpp
```

## Installation

```bash
# Clone the repository
git clone https://github.com/shawakash/cexa.git

# Build the project
cd cexa
mkdir build
cd build
cmake ..
make

# Run the bot
./cexa
```

## Usage

```cpp
// Initialize the bot with trade size
ArbitrageBot(0.005, 0.001);  // 0.005% min profit 0.001 BTC trade size

// Add exchanges with decorators
bot->addExchange(
    new LatencyDecorator(
        new LoggingDecorator(
            new BinanceTool()
        )
    )
);

// Run the bot
bot->run(Token::BTC, Token::USDC, 1000);  // Scan every 1000ms
```

## Configuration

Key parameters can be configured in the main application:
- Trade size
- Scan interval
- Target tokens
- Exchange endpoints

## Logging

The system maintains three types of logs:
1. `exchange_logs.txt`: Individual exchange operations
2. `arbitrage_logs.txt`: Discovered opportunities
3. `arbitrage_latency.txt`: System performance metrics

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## Future Roadmap

### Phase 1: Enhanced Monitoring
- [ ] WebSocket integration for real-time price updates
- [ ] Implementation of Observer pattern for notifications
- [ ] Additional exchange support

### Phase 2: Order Execution
- [ ] Smart order routing system
- [ ] Position management
- [ ] Order execution engine
- [ ] Transaction cost analysis

### Phase 3: Risk Management
- [ ] Position sizing logic
- [ ] Risk limits implementation
- [ ] Automated stop-loss mechanisms
- [ ] Balance management system

## Disclaimer

This software is for educational purposes only. Use at your own risk. The authors and contributors are not responsible for any financial losses incurred through the use of this software.
