#pragma once

#include "types.hpp"

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <vector>

namespace trading {

struct MetricsSnapshot { std::uint64_t generated{}, processed{}, accepted{}, rejected{}, trades{}; };
struct LatencyPercentiles { double p50_us{}, p95_us{}, p99_us{}; };

class Metrics {
public:
    void record_generated();
    void record_processed(bool accepted, std::size_t trade_count, std::chrono::steady_clock::time_point enqueued_at);
    [[nodiscard]] MetricsSnapshot snapshot() const;
    [[nodiscard]] LatencyPercentiles percentiles() const;
    void write_csv(const std::filesystem::path& directory, double elapsed_seconds) const;

private:
    std::atomic<std::uint64_t> generated_{0}, processed_{0}, accepted_{0}, rejected_{0}, trades_{0};
    std::vector<std::int64_t> latencies_ns_;
};

}  // namespace trading
