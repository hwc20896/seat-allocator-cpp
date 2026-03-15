#pragma once

#include <vector>
#include <string>
#include <format>

using Row = std::vector<std::string>;
using Grid = std::vector<Row>;

template <>
struct std::formatter<Grid> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    auto format(const std::vector<std::vector<std::string>>& v, format_context& ctx) const {
        const auto out = std::format_to(ctx.out(), "[");

        for (size_t i = 0; i < v.size(); ++i) {
            if (i > 0) {
                std::format_to(ctx.out(), ",");
            }

            std::format_to(ctx.out(), "\n    [");
            for (size_t j = 0; j < v[i].size(); ++j) {
                std::format_to(ctx.out(), "\"{}\"{}", v[i][j], (j == v[i].size() - 1 ? "" : ", "));
            }
            std::format_to(ctx.out(), "]");
        }

        if (!v.empty()) {
            std::format_to(ctx.out(), "\n");
        }
        return std::format_to(ctx.out(), "]");
    }
};