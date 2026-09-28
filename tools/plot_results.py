"""Create latency and summary charts from a simulator run."""

import csv
import sys
from pathlib import Path

import matplotlib.pyplot as plt


def main() -> None:
    results = Path(sys.argv[1]) if len(sys.argv) > 1 else Path("results")
    with (results / "run_summary.csv").open(newline="") as file:
        summary = next(csv.DictReader(file))
    with (results / "latency_samples.csv").open(newline="") as file:
        latencies_us = [int(row["latency_ns"]) / 1_000 for row in csv.DictReader(file)]

    figure, axes = plt.subplots(1, 2, figsize=(11, 4.5))
    axes[0].hist(latencies_us, bins=80, color="#0f766e", edgecolor="white")
    axes[0].set(title="Order latency distribution", xlabel="Latency (us)", ylabel="Orders")

    labels = ["Throughput", "Trades", "Rejects"]
    values = [float(summary["orders_per_second"]), int(summary["trades"]), int(summary["rejected"])]
    axes[1].bar(labels, values, color=["#2563eb", "#16a34a", "#dc2626"])
    axes[1].set(title="Simulation summary", ylabel="Count / orders per second")
    figure.suptitle(f"p50 {summary['p50_us']} us | p95 {summary['p95_us']} us | p99 {summary['p99_us']} us")
    figure.tight_layout()
    output = results / "performance_summary.png"
    figure.savefig(output, dpi=160)
    print(f"Wrote {output}")


if __name__ == "__main__":
    main()
