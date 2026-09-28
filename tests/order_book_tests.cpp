#include "order_book.hpp"

#include <cassert>
#include <iostream>

using trading::Order;
using trading::OrderBook;
using trading::Side;

namespace {

Order make_order(std::uint64_t id, Side side, trading::PriceTicks price, trading::Quantity quantity) {
    return {id, "test-client", "AAPL", side, price, quantity, quantity, {}};
}

void matches_at_resting_price() {
    OrderBook book("AAPL");
    book.process(make_order(1, Side::Sell, 10'005, 100));
    const auto trades = book.process(make_order(2, Side::Buy, 10'010, 40));

    assert(trades.size() == 1);
    assert(trades[0].price == 10'005);
    assert(trades[0].quantity == 40);
    assert(book.best_ask()->price == 10'005);
    assert(book.best_ask()->quantity == 60);
}

void preserves_fifo_at_same_price() {
    OrderBook book("AAPL");
    book.process(make_order(1, Side::Sell, 10'005, 30));
    book.process(make_order(2, Side::Sell, 10'005, 30));
    const auto trades = book.process(make_order(3, Side::Buy, 10'005, 45));

    assert(trades.size() == 2);
    assert(trades[0].sell_order_id == 1);
    assert(trades[0].quantity == 30);
    assert(trades[1].sell_order_id == 2);
    assert(trades[1].quantity == 15);
    assert(book.best_ask()->quantity == 15);
}

void rests_when_prices_do_not_cross() {
    OrderBook book("AAPL");
    book.process(make_order(1, Side::Buy, 10'000, 25));
    const auto trades = book.process(make_order(2, Side::Sell, 10'001, 25));

    assert(trades.empty());
    assert(book.best_bid()->price == 10'000);
    assert(book.best_ask()->price == 10'001);
}

}  // namespace

int main() {
    matches_at_resting_price();
    preserves_fifo_at_same_price();
    rests_when_prices_do_not_cross();
    std::cout << "order_book_tests passed\n";
    return 0;
}
