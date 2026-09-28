#pragma once

#include "order_book.hpp"
#include "risk_engine.hpp"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace trading {

struct ProcessResult {
    bool accepted{};
    std::optional<std::string> rejection_reason;
    std::vector<Trade> trades;
};

class MatchingEngine {
public:
    explicit MatchingEngine(RiskLimits limits = {});

    ProcessResult process(Order order);
    [[nodiscard]] std::optional<BestQuote> best_bid(const std::string& symbol) const;
    [[nodiscard]] std::optional<BestQuote> best_ask(const std::string& symbol) const;
    [[nodiscard]] Quantity position(const std::string& client_id, const std::string& symbol) const;

private:
    RiskEngine risk_engine_;
    std::unordered_map<std::string, OrderBook> books_;
};

}  // namespace trading
