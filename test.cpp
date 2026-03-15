#include <print>
#include <ranges>
#include <thread>
#include <chrono>

#include "algorithm.hpp"
#include "src/utils/utils.hpp"

//#define TEST_ALGORITHM
#define TEST_SHUFFLE_ANIMATION

using namespace std::chrono_literals;

const Grid hardcoded_grid = {
    {"1", "2", "3"},
    {"4", "5", "6"},
    {"7", "8", ""}
};

const auto config = ShuffleConfigBuilder().forbid_col("A3", 0).forbid_col("A5", 0).build();

int main() {
    Grid grid = readCSV("2026-01.csv");
    if (grid.empty()) grid = hardcoded_grid;

    std::println("Original grid look like this:\n{}", grid);

#ifdef TEST_ALGORITHM
    for ([[maybe_unused]] const auto _ : std::views::iota(0, 10)) {
        try {
            GridShuffler shuffler(config);
            const bool result = shuffler.setGrid(grid);

            std::println("Set Grid Result: {}", result);

            shuffler.shuffle();

            std::println("Shuffle successfully.");
            std::println("Result: {}", shuffler.getGrid());

            std::println("Validate result: {}", shuffler.validateResult());
        }
        catch (const std::exception& e) {
            std::println("{}", e.what());
        }
    }
#endif

#ifdef TEST_SHUFFLE_ANIMATION
    for ([[maybe_unused]]const auto i : std::views::iota(1, 25)) {
        shuffleGrid(grid);
        std::println("ATTEMPT no. {}: \n{}\n\n", i, grid);
        std::this_thread::sleep_for(125ms);
    }
#endif
}