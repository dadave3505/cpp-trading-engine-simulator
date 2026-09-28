#include "matching_engine.hpp"

#include <cassert>
#include <iostream>

namespace {

trading::Order make_order(std::uint64_t id, const char* client, const char* symbol, trading::Side side,
                          trading::PriceTicks price, trading::Quantity quantity) {
    return {id, client, symbol, side, price, quantity, quantity, {}};
}

void routes_symbols_and_updates_positions() {
    trading::MatchingEngine engine;
    assert(engine.process(make_order(1, "seller", "AAPL", trading::Side::Sell, 10'000, 20)).accepted);
    const auto result = engine.process(make_order(2, "buyer", "AAPL", trading::Side::Buy, 10'005, 20));

    assert(result.accepted);
    assert(result.trades.size() == 1);
    assert(engine.position("buyer", "AAPL") == 20);
    assert(engine.position("seller", "AAPL") == -20);
    assert(!engine.best_bid("AAPL"));
    assert(!engine.best_ask("AAPL"));
}

void rejects_orders_outside_risk_limits() {
    trading::RiskLimits limits;
    limits.max_order_quantity = 10;
    trading::MatchingEngine engine(limits);
    const auto result = engine.process(make_order(1, "buyer", "MSFT", trading::Side::Buy, 20'000, 11));

    assert(!result.accepted);
    assert(result.rejection_reason.has_value());
}

}  // namespace

int main() {
    routes_symbols_and_updates_positions();
    rejects_orders_outside_risk_limits();
    std::cout << "matching_engine_tests passed\n";
    return 0;
}
