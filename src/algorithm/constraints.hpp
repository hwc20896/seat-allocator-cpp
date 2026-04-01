#pragma once

#include <print>
#include <variant>
#include <string>
#include <vector>
#include <nlohmann/json_fwd.hpp>

//   Constraint definitions
struct ForceRow { std::string first; int second; };

struct ForbidRow { std::string first; int second; };

struct ForceCol { std::string first; int second; };

struct ForbidCol { std::string first; int second; };

struct ForbidShareRow { std::string first; std::string second; };

struct ForbidShareCol { std::string first; std::string second; };

using Constraint = std::variant<ForceRow, ForbidRow, ForceCol, ForbidCol, ForbidShareRow, ForbidShareCol>;
using Constraints = std::vector<Constraint>;

struct ShuffleConfig {
    bool allow_fixed_points = false;
    bool allow_original_neighbors = false;
    bool diagonals_are_neighbors = false;
    std::vector<std::pair<std::string, std::string>> custom_forbidden_pairs;
    Constraints constraints;

    constexpr
    explicit ShuffleConfig() = default;

    constexpr
    void setAllowFixedPoints(bool allow_fixed_points);

    constexpr
    void setAllowOriginalNeighbors(bool allow_original_neighbors);

    constexpr
    void setDiagonalsAreNeighbors(bool diagonals_are_neighbors);

    constexpr
    void addForbiddenPair(const std::string& a, const std::string& b);

    constexpr
    void forceRow(const std::string& val, int row_idx);

    constexpr
    void forbidRow(const std::string& val, int row_idx);

    constexpr
    void forceCol(const std::string& val, int col_idx);

    constexpr
    void forbidCol(const std::string& val, int col_idx);

    constexpr
    void forbidShareRow(const std::string& val1, const std::string& val2);

    constexpr
    void forbidShareCol(const std::string& val1, const std::string& val2);

    [[nodiscard]]
    static
    ShuffleConfig from_json(const nlohmann::json& json);
};