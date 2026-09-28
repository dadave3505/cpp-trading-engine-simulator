#include "matching_engine.hpp"

namespace trading {

MatchingEngine::MatchingEngine(RiskLimits limits) : risk_engine_(limits) {}

ProcessResult MatchingEngine::process(Order order) {
    if (const auto rejection = risk_engine_.validate(order)) {
        return {false, rejection, {}};
    }

    risk_engine_.reserve(order);
    const auto [book, inserted] = books_.try_emplace(order.symbol, order.symbol);
    (void)inserted;
    auto trades = book->second.process(std::move(order));
    for (const auto& trade : trades) {
        risk_engine_.on_trade(trade);
    }
    return {true, std::nullopt, std::move(trades)};
}

std::optional<BestQuote> MatchingEngine::best_bid(const std::string& symbol) const {
    const auto found = books_.find(symbol);
    return found == books_.end() ? std::nullopt : found->second.best_bid();
}

std::optional<BestQuote> MatchingEngine::best_ask(const std::string& symbol) const {
    const auto found = books_.find(symbol);
    return found == books_.end() ? std::nullopt : found->second.best_ask();
}

Quantity MatchingEngine::position(const std::string& client_id, const std::string& symbol) const {
    return risk_engine_.position(client_id, symbol);
}

}  // namespace trading
