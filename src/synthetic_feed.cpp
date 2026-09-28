#include "synthetic_feed.hpp"

#include <algorithm>
#include <chrono>
#include <random>
#include <thread>

namespace trading {
std::vector<std::string> make_symbols(std::uint32_t count) {
    static const std::vector<std::string> names{"AAPL", "MSFT", "NVDA", "AMZN", "GOOG", "TSLA", "META", "AMD"};
    std::vector<std::string> symbols;
    symbols.reserve(count);
    for (std::uint32_t index = 0; index < count; ++index) symbols.push_back(index < names.size() ? names[index] : "SYM" + std::to_string(index + 1));
    return symbols;
}
void produce_synthetic_orders(const SimulationConfig& config, std::uint32_t producer_index, const std::vector<std::string>& symbols, std::atomic<OrderId>& next_order_id, BlockingQueue<Order>& queue, Metrics& metrics) {
    std::mt19937_64 rng(config.seed + producer_index * 7'919);
    std::uniform_int_distribution<std::size_t> symbol_distribution(0, symbols.size() - 1);
    std::uniform_int_distribution<int> binary(0, 1), drift(-2, 2), offset(1, 6), aggressive_roll(0, 99), quantity_distribution(1, 100);
    std::vector<PriceTicks> reference_prices(symbols.size());
    for (std::size_t index = 0; index < symbols.size(); ++index) reference_prices[index] = 10'000 + static_cast<PriceTicks>(index) * 2'500;
    const auto rate = std::max<std::uint64_t>(1, config.orders_per_second / config.producers);
    const auto interval = std::chrono::nanoseconds(1'000'000'000ULL / rate);
    const auto end_time = std::chrono::steady_clock::now() + std::chrono::seconds(config.duration_seconds);
    auto next_emit = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() < end_time) {
        const auto index = symbol_distribution(rng);
        auto& reference = reference_prices[index];
        reference = std::max<PriceTicks>(100, reference + drift(rng));
        const Side side = binary(rng) == 0 ? Side::Buy : Side::Sell;
        const PriceTicks price_offset = offset(rng);
        const bool aggressive = aggressive_roll(rng) < 18;
        const PriceTicks price = side == Side::Buy ? reference + (aggressive ? price_offset : -price_offset) : reference + (aggressive ? -price_offset : price_offset);
        const Quantity quantity = quantity_distribution(rng);
        queue.push({next_order_id.fetch_add(1), "client-" + std::to_string(producer_index), symbols[index], side, price, quantity, quantity, std::chrono::steady_clock::now()});
        metrics.record_generated();
        next_emit += interval;
        std::this_thread::sleep_until(next_emit);
    }
}
}  // namespace trading
