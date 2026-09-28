#include "metrics.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <stdexcept>

namespace trading {
void Metrics::record_generated() { ++generated_; }
void Metrics::record_processed(bool accepted, std::size_t trade_count, std::chrono::steady_clock::time_point enqueued_at) {
    ++processed_;
    if (accepted) ++accepted_; else ++rejected_;
    trades_ += trade_count;
    latencies_ns_.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - enqueued_at).count());
}
MetricsSnapshot Metrics::snapshot() const { return {generated_.load(), processed_.load(), accepted_.load(), rejected_.load(), trades_.load()}; }
LatencyPercentiles Metrics::percentiles() const {
    if (latencies_ns_.empty()) return {};
    auto sorted = latencies_ns_;
    std::sort(sorted.begin(), sorted.end());
    const auto at = [&sorted](double p) { return static_cast<double>(sorted[static_cast<std::size_t>(p * (sorted.size() - 1))]) / 1'000.0; };
    return {at(0.50), at(0.95), at(0.99)};
}
void Metrics::write_csv(const std::filesystem::path& directory, double elapsed_seconds) const {
    std::filesystem::create_directories(directory);
    const auto summary = snapshot();
    const auto latency = percentiles();
    std::ofstream summary_file(directory / "run_summary.csv");
    if (!summary_file) throw std::runtime_error("could not write run summary");
    summary_file << "generated,processed,accepted,rejected,trades,orders_per_second,p50_us,p95_us,p99_us\n";
    summary_file << summary.generated << ',' << summary.processed << ',' << summary.accepted << ',' << summary.rejected << ',' << summary.trades << ',' << std::fixed << std::setprecision(2) << (elapsed_seconds > 0.0 ? static_cast<double>(summary.processed) / elapsed_seconds : 0.0) << ',' << latency.p50_us << ',' << latency.p95_us << ',' << latency.p99_us << '\n';
    std::ofstream latency_file(directory / "latency_samples.csv");
    if (!latency_file) throw std::runtime_error("could not write latency samples");
    latency_file << "latency_ns\n";
    for (const auto value : latencies_ns_) latency_file << value << '\n';
}
}  // namespace trading
