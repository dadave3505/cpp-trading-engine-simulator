param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Release"
)

$ErrorActionPreference = "Stop"
$flags = @("-std=c++17", "-Wall", "-Wextra", "-Wpedantic", "-pthread", "-Iinclude")
if ($Configuration -eq "Release") { $flags += "-O3" } else { $flags += "-g" }
New-Item -ItemType Directory -Force build | Out-Null

$core = @("src/order_book.cpp", "src/risk_engine.cpp", "src/matching_engine.cpp", "src/synthetic_feed.cpp", "src/metrics.cpp")
& g++ @flags @core "src/main.cpp" -o "build/trading_engine.exe"
& g++ @flags @core "tests/order_book_tests.cpp" -o "build/order_book_tests.exe"
& g++ @flags @core "tests/matching_engine_tests.cpp" -o "build/matching_engine_tests.exe"
& g++ @flags @core "tests/pipeline_tests.cpp" -o "build/pipeline_tests.exe"
