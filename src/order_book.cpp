#include "order_book.hpp"

#include <numeric>
#include <stdexcept>
#include <utility>

namespace trading {

OrderBook::OrderBook(std::string symbol) : symbol_(std::move(symbol)) {}

std::vector<Trade> OrderBook::process(Order order) {
    if (order.symbol != symbol_) {
        throw std::invalid_argument("order symbol does not match this book");
    }
    if (order.remaining <= 0) {
        order.remaining = order.quantity;
    }

    std::vector<Trade> trades;
    if (order.side == Side::Buy) {
        while (order.remaining > 0 && !asks_.empty() && asks_.begin()->first <= order.price) {
            auto level = asks_.begin();
            auto& resting_orders = level->second;
            auto& resting = resting_orders.front();
            const Quantity fill = std::min(order.remaining, resting.remaining);
            trades.push_back({order.id, resting.id, order.client_id, resting.client_id, symbol_, resting.price, fill});
            order.remaining -= fill;
            resting.remaining -= fill;
            if (resting.remaining == 0) {
                resting_orders.pop_front();
            }
            if (resting_orders.empty()) {
                asks_.erase(level);
            }
        }
        if (order.remaining > 0) {
            bids_[order.price].push_back(std::move(order));
        }
    } else {
        while (order.remaining > 0 && !bids_.empty() && bids_.begin()->first >= order.price) {
            auto level = bids_.begin();
            auto& resting_orders = level->second;
            auto& resting = resting_orders.front();
            const Quantity fill = std::min(order.remaining, resting.remaining);
            trades.push_back({resting.id, order.id, resting.client_id, order.client_id, symbol_, resting.price, fill});
            order.remaining -= fill;
            resting.remaining -= fill;
            if (resting.remaining == 0) {
                resting_orders.pop_front();
            }
            if (resting_orders.empty()) {
                bids_.erase(level);
            }
        }
        if (order.remaining > 0) {
            asks_[order.price].push_back(std::move(order));
        }
    }
    return trades;
}

const std::string& OrderBook::symbol() const noexcept { return symbol_; }

std::optional<BestQuote> OrderBook::best_bid() const {
    if (bids_.empty()) {
        return std::nullopt;
    }
    const auto& [price, orders] = *bids_.begin();
    return BestQuote{price, std::accumulate(orders.begin(), orders.end(), Quantity{0},
                                             [](Quantity total, const Order& order) {
                                                 return total + order.remaining;
                                             })};
}

std::optional<BestQuote> OrderBook::best_ask() const {
    if (asks_.empty()) {
        return std::nullopt;
    }
    const auto& [price, orders] = *asks_.begin();
    return BestQuote{price, std::accumulate(orders.begin(), orders.end(), Quantity{0},
                                             [](Quantity total, const Order& order) {
                                                 return total + order.remaining;
                                             })};
}

Quantity OrderBook::resting_quantity(Side side) const {
    Quantity total = 0;
    const auto accumulate_levels = [&total](const auto& levels) {
        for (const auto& [price, orders] : levels) {
            (void)price;
            for (const auto& order : orders) {
                total += order.remaining;
            }
        }
    };
    if (side == Side::Buy) {
        accumulate_levels(bids_);
    } else {
        accumulate_levels(asks_);
    }
    return total;
}

}  // namespace trading
