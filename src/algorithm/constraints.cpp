#include "constraints.hpp"

#include <nlohmann/json.hpp>

constexpr void ShuffleConfig::setAllowFixedPoints(const bool allow_fixed_points) {
    this->allow_fixed_points = allow_fixed_points;
}

constexpr void ShuffleConfig::setAllowOriginalNeighbors(const bool allow_original_neighbors) {
    this->allow_original_neighbors = allow_original_neighbors;
}

constexpr void ShuffleConfig::setDiagonalsAreNeighbors(const bool diagonals_are_neighbors) {
    this->diagonals_are_neighbors = diagonals_are_neighbors;
}

constexpr void ShuffleConfig::addForbiddenPair(const std::string& a, const std::string& b) {
    custom_forbidden_pairs.emplace_back(a, b);
}

constexpr void ShuffleConfig::forceRow(const std::string& val, const int row_idx) {
    constraints.emplace_back(ForceRow{val, row_idx});
}

constexpr void ShuffleConfig::forbidRow(const std::string& val, const int row_idx) {
    constraints.emplace_back(ForbidRow{val, row_idx});
}

constexpr void ShuffleConfig::forceCol(const std::string& val, const int col_idx) {
    constraints.emplace_back(ForceCol{val, col_idx});
}

constexpr void ShuffleConfig::forbidCol(const std::string& val, const int col_idx) {
    constraints.emplace_back(ForbidCol{val, col_idx});
}

constexpr void ShuffleConfig::forbidShareRow(const std::string& val1, const std::string& val2) {
    constraints.emplace_back(ForbidShareRow{val1, val2});
}

constexpr void ShuffleConfig::forbidShareCol(const std::string& val1, const std::string& val2) {
    constraints.emplace_back(ForbidShareCol{val1, val2});
}

ShuffleConfig ShuffleConfig::from_json(const nlohmann::json& json) {
    ShuffleConfig config;
    if (json.contains("shuffle_settings")) {
        const auto& settings = json["shuffle_settings"];

        if (settings.contains("allow_fixed_points"))
            config.setAllowFixedPoints(settings["allow_fixed_points"].get<bool>());

        if (settings.contains("allow_original_neighbors"))
            config.setAllowOriginalNeighbors(settings["allow_original_neighbors"].get<bool>());

        if (settings.contains("diagonals_are_neighbors"))
            config.setDiagonalsAreNeighbors(settings["diagonals_are_neighbors"].get<bool>());

        if (settings.contains("custom_forbidden_pairs")) {
            for (const auto& pair : settings["custom_forbidden_pairs"]) {
                if (pair.size() == 2) {
                    config.addForbiddenPair(
                        pair[0].get<std::string>(),
                        pair[1].get<std::string>()
                    );
                }
            }
        }
    }

    if (json.contains("constraints")) {
        for (const auto& constraint : json["constraints"]) {
            const auto type = constraint["type"].get<std::string>();

            if (type == "force_row") {
                config.forceRow(
                    constraint["value"].get<std::string>(),
                    constraint["index"].get<int>()
                );
            }
            else if (type == "forbid_row") {
                config.forbidRow(
                    constraint["value"].get<std::string>(),
                    constraint["index"].get<int>()
                );
            }
            else if (type == "force_col") {
                config.forceCol(
                    constraint["value"].get<std::string>(),
                    constraint["index"].get<int>()
                );
            }
            else if (type == "forbid_col") {
                config.forbidCol(
                    constraint["value"].get<std::string>(),
                    constraint["index"].get<int>()
                );
            }
            else if (type == "forbid_share_row") {
                config.forbidShareRow(
                    constraint["value1"].get<std::string>(),
                    constraint["value2"].get<std::string>()
                );
            }
            else if (type == "forbid_share_col") {
                config.forbidShareCol(
                    constraint["value1"].get<std::string>(),
                    constraint["value2"].get<std::string>()
                );
            }
        }
    }

    return config;
}
