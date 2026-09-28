#include "blocking_queue.hpp"
#include "matching_engine.hpp"
#include "metrics.hpp"
#include "synthetic_feed.hpp"

#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace {
void print_usage() { std::cout << "Usage: trading_engine [--symbols N] [--producers N] [--rate N] [--duration N] [--seed N]\n"; }
bool parse_arguments(int argc, char* argv[], trading::SimulationConfig& config) {
    for (int index = 1; index < argc; index += 2) {
        if (index + 1 >= argc) return false;
        const std::string flag = argv[index];
        const auto value = std::stoull(argv[index + 1]);
        if (flag == "--symbols") config.symbols = static_cast<std::uint32_t>(value);
        else if (flag == "--producers") config.producers = static_cast<std::uint32_t>(value);
        else if (flag == "--rate") config.orders_per_second = value;
        else if (flag == "--duration") config.duration_seconds = static_cast<std::uint32_t>(value);
        else if (flag == "--seed") config.seed = value;
        else return false;
    }
    return config.symbols > 0 && config.producers > 0 && config.orders_per_second > 0 && config.duration_seconds > 0;
}
}  // namespace

int main(int argc, char* argv[]) {
    trading::SimulationConfig config;
    try {
        if (!parse_arguments(argc, argv, config)) { print_usage(); return 1; }
    } catch (const std::exception&) {
        print_usage();
        return 1;
    }

    trading::BlockingQueue<trading::Order> queue;
    trading::MatchingEngine engine;
    trading::Metrics metrics;
    std::atomic<trading::OrderId> next_order_id{1};
    const auto symbols = trading::make_symbols(config.symbols);
    const auto started_at = std::chrono::steady_clock::now();
    std::atomic<bool> producers_finished{false};
    std::atomic<bool> reporter_stop{false};

    std::thread reporter([&] {
        while (!reporter_stop.load()) {
            const auto snapshot = metrics.snapshot();
            std::cout << "\rprocessed=" << snapshot.processed << " trades=" << snapshot.trades << " queue=" << queue.size() << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }
    });

    std::vector<std::thread> producers;
    producers.reserve(config.producers);
    for (std::uint32_t index = 0; index < config.producers; ++index) {
        producers.emplace_back(trading::produce_synthetic_orders, std::cref(config), index, std::cref(symbols), std::ref(next_order_id), std::ref(queue), std::ref(metrics));
    }
    std::thread closer([&] {
        for (auto& producer : producers) producer.join();
        producers_finished = true;
        queue.close();
    });

    while (auto order = queue.wait_pop()) {
        const auto result = engine.process(std::move(*order));
        metrics.record_processed(result.accepted, result.trades.size(), order->enqueued_at);
    }
    closer.join();
    const auto ended_at = std::chrono::steady_clock::now();
    reporter_stop = true;
    reporter.join();

    const auto elapsed = std::chrono::duration<double>(ended_at - started_at).count();
    const auto snapshot = metrics.snapshot();
    const auto latency = metrics.percentiles();
    metrics.write_csv("results", elapsed);
    std::cout << "\n\nSimulation completed in " << std::fixed << std::setprecision(2) << elapsed << " seconds\n"
              << "Generated: " << snapshot.generated << "  Processed: " << snapshot.processed << "  Accepted: " << snapshot.accepted << "  Rejected: " << snapshot.rejected << '\n'
              << "Trades: " << snapshot.trades << "  Throughput: " << (snapshot.processed / elapsed) << " orders/sec\n"
              << "Latency: p50 " << latency.p50_us << " us | p95 " << latency.p95_us << " us | p99 " << latency.p99_us << " us\n";
    for (const auto& symbol : symbols) {
        const auto bid = engine.best_bid(symbol);
        const auto ask = engine.best_ask(symbol);
        std::cout << symbol << "  bid=" << (bid ? std::to_string(bid->price) : "-") << " ask=" << (ask ? std::to_string(ask->price) : "-") << '\n';
    }
    std::cout << "Wrote results/run_summary.csv and results/latency_samples.csv\n";
    return 0;
}
