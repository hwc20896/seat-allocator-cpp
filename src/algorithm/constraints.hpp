#pragma once

#include <print>
#include <variant>
#include <string>
#include <vector>

//   Constraint definitions
struct ForceRow { std::string first; int second; };

struct ForbidRow { std::string first; int second; };

struct ForceCol { std::string first; int second; };

struct ForbidCol { std::string first; int second; };

struct ForbidShareRow { std::string first; std::string second; };

struct ForbidShareCol { std::string first; std::string second; };

using Constraint = std::variant<ForceRow, ForbidRow, ForceCol, ForbidCol, ForbidShareRow, ForbidShareCol>;
using Constraints = std::vector<Constraint>;

//  Something like factory design pattern
namespace detail {
    struct ShuffleConfig {
        bool allow_fixed_points = false;
        bool allow_original_neighbors = false;
        bool diagonals_are_neighbors = false;
        std::vector<std::pair<std::string, std::string>> custom_forbidden_pairs;
        Constraints constraints;

        constexpr
        explicit ShuffleConfig() = default;

        constexpr
        void set_allow_fixed_points(const bool allow_fixed_points) {
            this->allow_fixed_points = allow_fixed_points;
        }

        constexpr
        void set_allow_original_neighbors(const bool allow_original_neighbors) {
            this->allow_original_neighbors = allow_original_neighbors;
        }

        constexpr
        void set_diagonals_are_neighbors(const bool diagonals_are_neighbors) {
            this->diagonals_are_neighbors = diagonals_are_neighbors;
        }

        constexpr
        void add_forbidden_pair(const std::string& a, const std::string& b) {
            custom_forbidden_pairs.emplace_back(a, b);
        }

        constexpr
        void force_row(const std::string& val, const int row_idx) {
            constraints.emplace_back(ForceRow{val, row_idx});
        }

        constexpr
        void forbid_row(const std::string& val, const int row_idx) {
            constraints.emplace_back(ForbidRow{val, row_idx});
        }

        constexpr
        void force_col(const std::string& val, const int col_idx) {
            constraints.emplace_back(ForceCol{val, col_idx});
        }

        constexpr
        void forbid_col(const std::string& val, const int col_idx) {
            constraints.emplace_back(ForbidCol{val, col_idx});
        }

        constexpr
        void forbid_share_row(const std::string& val1, const std::string& val2) {
            constraints.emplace_back(ForbidShareRow{val1, val2});
        }

        constexpr
        void forbid_share_col(const std::string& val1, const std::string& val2) {
            constraints.emplace_back(ForbidShareCol{val1, val2});
        }
    };
}

using ShuffleConfig = detail::ShuffleConfig;

class ShuffleConfigBuilder {
    public:
        constexpr ShuffleConfigBuilder() = default;

        constexpr
        ShuffleConfigBuilder& allow_fixed_points(const bool allow_fixed_points) {
            config.allow_fixed_points = allow_fixed_points;
            return *this;
        }

        constexpr
        ShuffleConfigBuilder& allow_original_neighbors(const bool allow_original_neighbors) {
            config.allow_original_neighbors = allow_original_neighbors;
            return *this;
        }

        constexpr
        ShuffleConfigBuilder& diagonals_are_neighbors(const bool diagonals_are_neighbors) {
            config.diagonals_are_neighbors = diagonals_are_neighbors;
            return *this;
        }

        constexpr
        ShuffleConfigBuilder& add_forbidden_pair(const std::string& a, const std::string& b) {
            config.add_forbidden_pair(a, b);
            return *this;
        }

        constexpr
        ShuffleConfigBuilder& force_row(const std::string& val, const int row_idx) {
            config.force_row(val, row_idx);
            return *this;
        }

        constexpr
        ShuffleConfigBuilder& forbid_row(const std::string& val, const int row_idx) {
            config.forbid_row(val, row_idx);
            return *this;
        }

        constexpr
        ShuffleConfigBuilder& force_col(const std::string& val, const int col_idx) {
            config.force_col(val, col_idx);
            return *this;
        }

        constexpr
        ShuffleConfigBuilder& forbid_col(const std::string& val, const int col_idx) {
            config.forbid_col(val, col_idx);
            return *this;
        }

        constexpr
        ShuffleConfigBuilder& forbid_share_row(const std::string& val1, const std::string& val2) {
            config.forbid_share_row(val1, val2);
            return *this;
        }

        constexpr
        ShuffleConfigBuilder& forbid_share_col(const std::string& val1, const std::string& val2) {
            config.forbid_share_col(val1, val2);
            return *this;
        }

        [[nodiscard]]
        constexpr
        ShuffleConfig build() const {
            return config;
        }
    private:
        ShuffleConfig config;
};