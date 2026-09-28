cmake_minimum_required(VERSION 3.20)
project(trading_engine_simulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(trading_core
    src/order_book.cpp
    src/risk_engine.cpp
    src/matching_engine.cpp
    src/synthetic_feed.cpp
    src/metrics.cpp
)

target_include_directories(trading_core PUBLIC include)
target_compile_options(trading_core PRIVATE -Wall -Wextra -Wpedantic)
find_package(Threads REQUIRED)
target_link_libraries(trading_core PUBLIC Threads::Threads)

add_executable(trading_engine src/main.cpp)
target_link_libraries(trading_engine PRIVATE trading_core)

add_executable(order_book_tests tests/order_book_tests.cpp)
target_link_libraries(order_book_tests PRIVATE trading_core)

add_executable(matching_engine_tests tests/matching_engine_tests.cpp)
target_link_libraries(matching_engine_tests PRIVATE trading_core)

add_executable(pipeline_tests tests/pipeline_tests.cpp)
target_link_libraries(pipeline_tests PRIVATE trading_core)

enable_testing()
add_test(NAME order_book_tests COMMAND order_book_tests)
add_test(NAME matching_engine_tests COMMAND matching_engine_tests)
add_test(NAME pipeline_tests COMMAND pipeline_tests)
