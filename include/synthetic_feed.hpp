#pragma once

#include "blocking_queue.hpp"
#include "metrics.hpp"

#include <atomic>
#include <cstdint>
#include <string>
#include <vector>

namespace trading {
struct SimulationConfig { std::uint32_t symbols{4}, producers{2}; std::uint64_t orders_per_second{10'000}; std::uint32_t duration_seconds{5}; std::uint64_t seed{42}; };
std::vector<std::string> make_symbols(std::uint32_t count);
void produce_synthetic_orders(const SimulationConfig& config, std::uint32_t producer_index, const std::vector<std::string>& symbols, std::atomic<OrderId>& next_order_id, BlockingQueue<Order>& queue, Metrics& metrics);
}  // namespace trading
