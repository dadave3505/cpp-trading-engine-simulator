#include "risk_engine.hpp"

#include <cstdlib>

namespace trading {

RiskEngine::RiskEngine(RiskLimits limits) : limits_(limits) {}

std::optional<std::string> RiskEngine::validate(const Order& order) const {
    if (order.id == 0 || order.client_id.empty() || order.symbol.empty()) {
        return "missing order id, client id, or symbol";
    }
    if (order.price <= 0 || order.quantity <= 0) {
        return "price and quantity must be positive";
    }
    if (order.quantity > limits_.max_order_quantity) {
        return "order quantity exceeds configured limit";
    }
    if (order.price > limits_.max_order_notional / order.quantity) {
        return "order notional exceeds configured limit";
    }

    const auto exposure_key = key(order.client_id, order.symbol);
    const Quantity signed_quantity = order.side == Side::Buy ? order.quantity : -order.quantity;
    const Quantity current_exposure = positions_.count(exposure_key) == 0 ? 0 : positions_.at(exposure_key);
    const Quantity reserved = reserved_exposure_.count(exposure_key) == 0 ? 0 : reserved_exposure_.at(exposure_key);
    if (std::llabs(current_exposure + reserved + signed_quantity) > limits_.max_absolute_position) {
        return "position limit would be exceeded";
    }
    return std::nullopt;
}

void RiskEngine::reserve(const Order& order) {
    const Quantity signed_quantity = order.side == Side::Buy ? order.quantity : -order.quantity;
    reserved_exposure_[key(order.client_id, order.symbol)] += signed_quantity;
}

void RiskEngine::on_trade(const Trade& trade) {
    apply_fill(trade.buy_client_id, trade.symbol, Side::Buy, trade.quantity);
    apply_fill(trade.sell_client_id, trade.symbol, Side::Sell, trade.quantity);
}

Quantity RiskEngine::position(const std::string& client_id, const std::string& symbol) const {
    const auto found = positions_.find(key(client_id, symbol));
    return found == positions_.end() ? 0 : found->second;
}

std::string RiskEngine::key(const std::string& client_id, const std::string& symbol) {
    return client_id + '\x1f' + symbol;
}

void RiskEngine::apply_fill(const std::string& client_id, const std::string& symbol, Side side, Quantity quantity) {
    const auto exposure_key = key(client_id, symbol);
    const Quantity signed_quantity = side == Side::Buy ? quantity : -quantity;
    reserved_exposure_[exposure_key] -= signed_quantity;
    positions_[exposure_key] += signed_quantity;
}

}  // namespace trading
