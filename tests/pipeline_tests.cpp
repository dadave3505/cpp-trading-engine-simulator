#include "blocking_queue.hpp"
#include "matching_engine.hpp"

#include <atomic>
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>

int main() {
    constexpr std::uint32_t producer_count = 4;
    constexpr std::uint32_t orders_per_producer = 250;
    trading::BlockingQueue<trading::Order> queue;
    trading::MatchingEngine engine;
    std::atomic<trading::OrderId> next_id{1};
    std::vector<std::thread> producers;

    for (std::uint32_t producer = 0; producer < producer_count; ++producer) {
        producers.emplace_back([&, producer] {
            for (std::uint32_t index = 0; index < orders_per_producer; ++index) {
                const auto side = index % 2 == 0 ? trading::Side::Buy : trading::Side::Sell;
                queue.push({next_id.fetch_add(1), "client-" + std::to_string(producer), "AAPL", side,
                            10'000, 1, 1, std::chrono::steady_clock::now()});
            }
        });
    }
    std::thread closer([&] {
        for (auto& producer : producers) producer.join();
        queue.close();
    });

    std::uint32_t processed = 0;
    std::uint32_t accepted = 0;
    while (auto order = queue.wait_pop()) {
        const auto result = engine.process(std::move(*order));
        ++processed;
        accepted += result.accepted ? 1 : 0;
    }
    closer.join();

    assert(processed == producer_count * orders_per_producer);
    assert(accepted == processed);
    std::cout << "pipeline_tests passed\n";
    return 0;
}
