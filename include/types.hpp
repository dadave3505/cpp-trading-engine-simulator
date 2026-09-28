#pragma once

#include <chrono>
#include <cstdint>
#include <string>

namespace trading {

using OrderId = std::uint64_t;
using Quantity = std::int64_t;
using PriceTicks = std::int64_t;

enum class Side { Buy, Sell };

struct Order {
    OrderId id{};
    std::string client_id;
    std::string symbol;
    Side side{};
    PriceTicks price{};
    Quantity quantity{};
    Quantity remaining{};
    std::chrono::steady_clock::time_point enqueued_at{};
};

struct Trade {
    OrderId buy_order_id{};
    OrderId sell_order_id{};
    std::string buy_client_id;
    std::string sell_client_id;
    std::string symbol;
    PriceTicks price{};
    Quantity quantity{};
};

struct BestQuote {
    PriceTicks price{};
    Quantity quantity{};
};

}  // namespace trading
