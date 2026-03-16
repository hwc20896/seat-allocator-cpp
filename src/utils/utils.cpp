#include "utils/utils.hpp"

#include <sstream>
#include <fstream>
#include <print>
#include <ranges>
#include <random>
#include <algorithm>
#include <spdlog/spdlog.h>
#include <filesystem>

Grid readCSV(const std::string& filePath) {
    Grid grid;
    std::ifstream file(filePath);

    if (!file.is_open()) {
        std::println("Error: Cannot open file {}", filePath);
        return grid;
    }

    std::string line;
    while (std::getline(file, line)) {
        Row row;
        std::stringstream ss(line);
        std::string cell;

        while (std::getline(ss, cell, ',')) {
            // Trim whitespace if needed
            size_t start = cell.find_first_not_of(" \t\r\n");

            if (size_t end = cell.find_last_not_of(" \t\r\n");
                start != std::string::npos && end != std::string::npos
            )
                cell = cell.substr(start, end - start + 1);
            else if (start == std::string::npos)
                cell = "";

            row.push_back(cell);
        }

        // Handle trailing comma (empty last cell)
        if (!line.empty() && line.back() == ',')
            row.emplace_back("");

        grid.push_back(row);
    }

    return grid;
}

void shuffleGrid(Grid& grid) {
    auto nonEmptyCells = grid | std::views::join | std::views::filter([](const std::string& cell) {return !cell.empty();}) | std::ranges::to<Row>();

    std::ranges::shuffle(nonEmptyCells, std::mt19937{std::random_device{}()});

    std::ranges::copy(nonEmptyCells, (
        grid | std::views::join | std::views::filter([](const std::string& cell) {return !cell.empty();})
    ).begin());
}

void writeCSV(const std::string& filePath, const Grid& grid) {
    std::ofstream file(filePath);

    if (!file.is_open()) {
        spdlog::error("Cannot open file {}", filePath);
        return;
    }

    for (const auto& row : grid) {
        for (size_t i = 0; i < row.size(); ++i) {
            file << row[i];
            if (i != row.size() - 1)
                file << ',';
        }
    }
}

std::string getFileBasename(const std::string& filePath) {
    return std::filesystem::path(filePath).filename().string();
}