#pragma once

#include "types.hpp"

#include <deque>
#include <functional>
#include <map>
#include <optional>
#include <vector>

namespace trading {

class OrderBook {
public:
    explicit OrderBook(std::string symbol);

    std::vector<Trade> process(Order order);

    [[nodiscard]] const std::string& symbol() const noexcept;
    [[nodiscard]] std::optional<BestQuote> best_bid() const;
    [[nodiscard]] std::optional<BestQuote> best_ask() const;
    [[nodiscard]] Quantity resting_quantity(Side side) const;

private:
    using PriceLevel = std::deque<Order>;
    using BidLevels = std::map<PriceTicks, PriceLevel, std::greater<PriceTicks>>;
    using AskLevels = std::map<PriceTicks, PriceLevel>;

    std::string symbol_;
    BidLevels bids_;
    AskLevels asks_;
};

}  // namespace trading
