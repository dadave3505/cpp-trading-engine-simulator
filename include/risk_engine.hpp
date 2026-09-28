#pragma once

#include "types.hpp"

#include <optional>
#include <string>
#include <unordered_map>

namespace trading {

struct RiskLimits {
    Quantity max_order_quantity{10'000};
    PriceTicks max_order_notional{100'000'000};
    Quantity max_absolute_position{50'000};
};

class RiskEngine {
public:
    explicit RiskEngine(RiskLimits limits = {});

    [[nodiscard]] std::optional<std::string> validate(const Order& order) const;
    void reserve(const Order& order);
    void on_trade(const Trade& trade);

    [[nodiscard]] Quantity position(const std::string& client_id, const std::string& symbol) const;

private:
    [[nodiscard]] static std::string key(const std::string& client_id, const std::string& symbol);
    void apply_fill(const std::string& client_id, const std::string& symbol, Side side, Quantity quantity);

    RiskLimits limits_;
    std::unordered_map<std::string, Quantity> positions_;
    std::unordered_map<std::string, Quantity> reserved_exposure_;
};

}  // namespace trading
