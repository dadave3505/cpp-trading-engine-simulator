# C++17 Event-Driven Trading Engine Simulator

A single-writer matching engine that processes a real-time synthetic order stream. The project models the core backend path of a trading venue: asynchronous order ingress, pre-trade risk controls, multi-symbol limit order books, price-time-priority matching, and latency reporting.

## Architecture

```text
Synthetic producer threads
          |
          v
  Blocking ingress queue
          |
          v
Single matching-engine thread
  |        |         |
  v        v         v
Risk   Symbol router  Limit order books
          |
          v
Trades, positions, live metrics, CSV artifacts
```

Order books and risk state have one owner: the matching-engine thread. Producers only enqueue immutable order messages, so the matching path does not take locks or expose mutable book state across threads.

## Features

- C++17 multi-symbol limit order books backed by price-level maps and FIFO queues.
- Price-time priority, partial fills, resting orders, and execution at the resting price.
- Client-aware pre-trade checks for valid fields, maximum quantity, maximum notional, and reserved position exposure.
- Concurrent producer-consumer ingestion with controlled queue shutdown.
- Synthetic order flow with price random walks, passive liquidity, and aggressive orders that cross the spread.
- Live terminal progress plus CSV latency samples and percentile reporting.

## Build and Run

The checked-in `build.ps1` works with the MSYS2 GCC toolchain used during development:

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1 -Configuration Release
.\build\trading_engine.exe --symbols 4 --producers 2 --rate 10000 --duration 10 --seed 42
```

The simulator writes `results/run_summary.csv` and `results/latency_samples.csv`. Results are intentionally ignored by Git because they are machine- and workload-dependent.

To produce a chart after installing `matplotlib`:

```powershell
python tools/plot_results.py results
```

For CMake-based environments:

```powershell
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

## Tests

```powershell
.\build\order_book_tests.exe
.\build\matching_engine_tests.exe
.\build\pipeline_tests.exe
```

The test suite verifies price-time priority, partial fills, price crossing, multi-symbol routing, risk rejection, position updates, and that concurrent producers do not lose orders in the single-writer pipeline.

## Design Notes

Prices use integer ticks, not floating point. The ingress queue uses a mutex and condition variable because it keeps the concurrency boundary readable and correct. A production evolution would benchmark a bounded ring buffer, batching, memory pools, CPU affinity, cancellation handling, persistence, and market-data distribution before introducing more complexity.