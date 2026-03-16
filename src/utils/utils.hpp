#pragma once

#include "utiltypes.hpp"

/**
 * @brief Reads a CSV file and converts it into a 2D grid structure.
 *
 * @param filePath Path to the CSV file to read.
 * @return Grid A 2D vector of strings representing the CSV content.
 *         Returns an empty grid if the file cannot be opened or is empty.
 *
 * @note Each row in the CSV becomes a Row (vector<string>) in the Grid.
 *       Empty cells are represented as empty strings "".
 *       Whitespace around values is trimmed.
 */

std::string getFileBasename(const std::string& filePath);

Grid readCSV(const std::string& filePath);

void writeCSV(const std::string& filePath, const Grid& grid);

Grid readXLSX(const std::string& filePath);

void writeXLSX(const std::string& filePath, const Grid& grid);

void shuffleGrid(Grid& grid);