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
        auto out = ctx.out();
        out = std::format_to(ctx.out(), "[");

        for (size_t i = 0; i < v.size(); ++i) {
            if (i > 0) {
                out = std::format_to(out, ",");
            }

            out = std::format_to(out, "\n    [");
            for (size_t j = 0; j < v[i].size(); ++j) {
                out = std::format_to(out, "\"{}\"{}", v[i][j], (j == v[i].size() - 1 ? "" : ", "));
            }
            out = std::format_to(out, "]");
        }

        if (!v.empty()) {
            out = std::format_to(out, "\n");
        }
        return out = std::format_to(out, "]");
    }
};