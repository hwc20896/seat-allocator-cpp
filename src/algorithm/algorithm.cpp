#include "algorithm.hpp"

#include <random>
#include <utility>
#include <print>
#include <format>
#include <ranges>
#include <algorithm>

GridShuffler::GridShuffler(detail::ShuffleConfig config)
  : rowCount(0), columnCount(0), config(std::move(config)), numItems(0), dim(0)
{
    dirs = std::vector<Position>{{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
    if (config.diagonals_are_neighbors)
        dirs.append_range(std::vector<Position>({{-1, 1}, {1, 1}, {-1, -1}, {1, -1}}));
}

size_t GridShuffler::getSize() const noexcept {
    return data.size();
}

bool GridShuffler::setGrid(const Grid& grid) {
    rowCount = grid.size();
    columnCount = rowCount ? grid[0].size() : 0;

    originalGrid = grid;

    data.clear();
    nodeToPos.clear();
    idToString.clear();
    stringToID.clear();
    graph.clear();
    forbiddenAdjMatrix.clear();
    originalValueAtNode.clear();
    nodesByRow.clear();
    nodesByColumn.clear();
    domainMask.clear();
    numItems = 0;
    dim = 0;

    if (rowCount == 0 || columnCount == 0)
        return false;

    initTopology();

    try {
        initConstraints();
    }
    catch (std::exception& e) {
        std::println("Error: {}", e.what());
        throw;
    }

    initDomains();
    return true;
}

const Grid& GridShuffler::getOriginalGrid() const {
    return originalGrid;
}

const Grid& GridShuffler::getGrid() const {
    return data.empty()? originalGrid : data.back();
}

void GridShuffler::shuffle() {
    if (numItems == 0) return;

    if (dim < numItems) {
        const auto msg = std::format("Unable to shuffle: ({}) is less than quantity of non-null element count ({})", dim, numItems);
        std::println("{}", msg);
        throw std::invalid_argument(msg);
    }

    auto assignment = std::vector<std::optional<int>>(numItems, std::nullopt);
    auto usedValues = std::vector(dim, false);
    static constexpr int MAX_ATTEMPTS = 1000;

    for ([[maybe_unused]] const auto _ : std::views::iota(0, MAX_ATTEMPTS)) {
        if (solve(assignment, usedValues)) {
            auto newGrid = std::vector(rowCount, std::vector(columnCount, std::string()));
            for (const auto& [nodeIdx, value] : assignment | std::views::enumerate) {
                value.and_then([this, &newGrid, nodeIdx](const int valID) -> std::optional<int> {
                    const auto& [row, column] = nodeToPos.at(nodeIdx);
                    newGrid[row][column] = idToString.at(valID);
                    return {};
                });
            }

            data.push_back(newGrid);
            return;
        }

        std::ranges::fill(assignment, std::nullopt);
        std::ranges::fill(usedValues, false);
    }

    static const auto msg = std::format("Shuffle failed within attempt ({}); This might probably mean that the shuffler cannot find a solution.", MAX_ATTEMPTS);
    std::println("{}", msg);
    throw std::runtime_error(msg);
}

bool GridShuffler::validateResult() {
    const auto currentGrid = getGrid();

    for (const auto r : std::views::iota(0ULL, rowCount)) {
        for (const auto c : std::views::iota(0ULL, columnCount)) {
            const auto& currentValue = currentGrid[r][c];
            if (currentValue.empty()) continue;

            if (!config.allow_fixed_points &&
                currentValue == originalGrid[r][c])
                return false;

            const auto u = stringToID.at(currentValue);

            for (const auto& [dr, dc] : dirs) {
                const auto [nr, nc] = std::make_pair(r + dr, c + dc);

                if (nr >= rowCount || nc >= columnCount)
                    continue;

                const auto neighborValue = currentGrid[nr][nc];
                if (neighborValue.empty())
                    continue;

                if (isForbidden(u, stringToID.at(neighborValue)))
                    return false;
            }
        }
    }
    return true;
}

void GridShuffler::clearShuffledGrids() {
    data.clear();
}

const std::vector<Grid>& GridShuffler::getAllGrids() const {
    return data;
}

const Grid& GridShuffler::operator[](const int index) const {
    if (index < 0 || index >= data.size())
        return getGrid();
    return data[index];
}

const Grid& GridShuffler::getGrid(const int index) const {
    if (index < 0 || index >= data.size())
        return getGrid();
    return data[index];
}

//  private methods
void GridShuffler::initTopology() {
    auto gridToNode = std::vector(rowCount, std::vector<std::optional<int>>(columnCount, std::nullopt));//

    for (const auto r : std::views::iota(0ULL, rowCount)) {
        for (const auto c : std::views::iota(0ULL, columnCount)) {
            if (!originalGrid[r][c].empty()) {
                gridToNode[r][c] = numItems;
                nodeToPos.emplace_back(r, c);
                numItems++;
            }
        }
    }

    nodesByRow = std::vector<std::vector<NodeID>>(rowCount);
    nodesByColumn = std::vector<std::vector<NodeID>>(columnCount);
    for (const auto u : std::views::iota(0, numItems)) {
        const auto [r, c] = nodeToPos[u];
        nodesByRow[r].push_back(u);
        nodesByColumn[c].push_back(u);
    }

    graph = std::vector<std::vector<NodeID>>(numItems);

    for (const auto i : std::views::iota(0, numItems)) {
        const auto& [cr, cc] = nodeToPos[i];
        for (const auto& [dr, dc] : dirs) {
            const auto [nr, nc] = std::make_pair(cr + dr, cc + dc);
            if (nr < 0 || nr >= rowCount || nc < 0 || nc >= columnCount)
                continue;
            if (const auto g = gridToNode[nr][nc]; g.has_value())
                graph[i].push_back(*g);
        }
    }
}

void GridShuffler::initConstraints() {
    idToString.clear();
    stringToID.clear();
    originalValueAtNode = std::vector(numItems, 0);

    int valCounter = 0;
    std::vector<std::string> duplicateElements;

    for (const auto r : std::views::iota(0ULL, rowCount)) {
        for (const auto c : std::views::iota(0ULL, columnCount)) {
            const auto& value = originalGrid[r][c];
            if (value.empty()) continue;

            if (stringToID.contains(value)) {
                if (!std::ranges::contains(duplicateElements, value))
                    duplicateElements.push_back(value);
                continue;
            }

            stringToID.emplace(value, valCounter);
            idToString.push_back(value);
            valCounter++;
        }
    }

    if (!duplicateElements.empty()) {
        const auto msg = std::format("Duplicate elements found: {}.\nPlease ensure all element are unique.", duplicateElements);
        std::println("{}",msg);
        throw std::invalid_argument(msg);
    }

    dim = idToString.size();

    if (dim < numItems) {
        const auto msg = std::format("Unable to shuffle: ({}) is less than quantity of non-null element count ({})", dim, numItems);
        std::println("{}",msg);
        throw std::invalid_argument(msg);
    }

    for (const auto i : std::views::iota(0, numItems)) {
        const auto& [r, c] = nodeToPos[i];
        originalValueAtNode[i] = stringToID.at(originalGrid[r][c]);
    }

    const int pow_dim = dim * dim;
    forbiddenAdjMatrix = std::vector(pow_dim, false);

    if (!config.allow_original_neighbors) {
        for (const auto i : std::views::iota(0, numItems)) {
            const int u = originalValueAtNode[i];
            for (const auto j : graph[i]) {
                const int v = originalValueAtNode[j];
                const auto [idx1, idx2] = std::make_pair(u * dim + v, v * dim + u);
                if (idx1 < pow_dim)
                    forbiddenAdjMatrix[idx1] = true;
                if (idx2 < pow_dim)
                    forbiddenAdjMatrix[idx2] = true;
            }
        }
    }

    for (const auto& [s1, s2] : config.custom_forbidden_pairs) {
        const auto [u, v] = std::make_pair(stringToID.at(s1), stringToID.at(s2));
        const auto [idx1, idx2] = std::make_pair(u * dim + v, v * dim + u);
        if (idx1 < pow_dim)
            forbiddenAdjMatrix[idx1] = true;
        if (idx2 < pow_dim)
            forbiddenAdjMatrix[idx2] = true;
    }
}

void GridShuffler::initDomains() {
    domainMask = std::vector(numItems, std::vector(dim, true));

    for (const auto& constraint : config.constraints) {
        std::visit([&]<typename ConstraintType>(const ConstraintType& c){
            constexpr auto isForce = std::is_same_v<ConstraintType, ForceCol> || std::is_same_v<ConstraintType, ForceRow>;
            constexpr auto isForbid = std::is_same_v<ConstraintType, ForbidCol> || std::is_same_v<ConstraintType, ForbidRow>;

            constexpr auto constraintCol = std::is_same_v<ConstraintType, ForceCol> || std::is_same_v<ConstraintType, ForbidCol>;
            constexpr auto constraintRow = std::is_same_v<ConstraintType, ForceRow> || std::is_same_v<ConstraintType, ForbidRow>;

            if constexpr (constraintCol) {
                if (c.second < 0 || c.second >= columnCount) {
                    throw std::invalid_argument(std::format("Constraint Error: Column {} does not exist", c.second));
                }
                if (stringToID.contains(c.first)) {
                    const ValueID val_id = stringToID.at(c.first);
                    if constexpr (isForce) {
                        for (const auto u : std::views::iota(0, numItems)) {
                            const auto [_, nc] = nodeToPos[u];
                            if (nc != c.second) {
                                domainMask[u][val_id] = false;
                            }
                        }
                    }
                    else if constexpr (isForbid) {
                        for (const auto u : nodesByColumn[c.second]) {
                            domainMask[u][val_id] = false;
                        }
                    }
                }
            }
            else if constexpr (constraintRow) {
                if (c.second < 0 || c.second >= rowCount) {
                    throw std::invalid_argument(std::format("Constraint Error: Row {} does not exist", c.second));
                }
                if (stringToID.contains(c.first)) {
                    const ValueID val_id = stringToID.at(c.first);
                    if constexpr (isForce) {
                        for (const auto u : std::views::iota(0, numItems)) {
                            const auto nr = nodeToPos[u].first;
                            if (nr != c.second) {
                                domainMask[u][val_id] = false;
                            }
                        }
                    }
                    else if constexpr (isForbid) {
                        for (const auto u : nodesByRow[c.second]) {
                            domainMask[u][val_id] = false;
                        }
                    }
                }
            }
        }, constraint);
    }
}

bool GridShuffler::checkDynamicConstraints(const NodeID u, const ValueID val, const AssignmentType& assignment) const {
    const auto [r, c] = nodeToPos[u];

    // Helper lambda to process ForbidShareCol constraints
    const auto process_forbid_share = [&](const Graph& targetGraph, const ForbidShareCol& constraint) -> bool {
        if (stringToID.contains(constraint.first) && stringToID.contains(constraint.second)) {
            const ValueID id1 = stringToID.at(constraint.first);
            const ValueID id2 = stringToID.at(constraint.second);

            ValueID target_partner;
            if (val == id1) {
                target_partner = id2;
            } else if (val == id2) {
                target_partner = id1;
            } else {
                return true;  // Neither matches, constraint doesn't apply
            }

            // Check if any other node in the same column has the target partner
            for (const NodeID neighbor_node : targetGraph[c]) {
                if (neighbor_node == u)
                    continue;

                if (assignment[neighbor_node] == target_partner)
                    return false;
            }
        }
        return true;  // Constraint satisfied
    };

    // Process each constraint
    return std::ranges::all_of(config.constraints, [&]<typename Constraint>(const Constraint& constraint) -> bool{
        using ConstraintType = std::remove_cvref_t<Constraint>;

        if constexpr (std::is_same_v<ConstraintType, ForbidShareCol>)
            return process_forbid_share(nodesByColumn, constraint);
        else if constexpr (std::is_same_v<ConstraintType, ForbidShareRow>)
            return process_forbid_share(nodesByRow, constraint);
        return true;
    });
}

bool GridShuffler::isForbidden(const int u, const int v) const {
    const int idx = u * dim + v;
    return idx < forbiddenAdjMatrix.size()? forbiddenAdjMatrix[idx] : false;
}

bool GridShuffler::solve(std::vector<std::optional<ValueID>>& assignment, std::vector<bool>& visited) {
    std::optional<int> targetNode = std::nullopt;
    int64_t maxNeighborsSet = -1;
    bool complete = true;

    for (const auto& i : std::views::iota(0, numItems)) {
        if (assignment[i].has_value())
            continue;

        complete = false;

        const auto assignedNeighbors = std::ranges::count_if(graph[i], [&](const int neighbor) {
            return assignment[neighbor].has_value();
        });

        if (assignedNeighbors > maxNeighborsSet) {
            maxNeighborsSet = assignedNeighbors;
            targetNode = i;
        }
    }

    if (complete)
        return true;

    const int u = targetNode.value();
    auto candidates = std::vector<int>();
    candidates.reserve(dim);

    for (const auto& v : std::views::iota(0, dim)) {
        if (visited[v]) continue;
        if (!domainMask[u][v]) continue;
        if (!config.allow_fixed_points && originalValueAtNode[u] == v) continue;
        if (!checkDynamicConstraints(u, v, assignment)) continue;

        const bool no_conflict = std::ranges::none_of(graph[u], [&](const int neighbor) {
            const auto neighborValue = assignment[neighbor];
            return neighborValue.has_value() && isForbidden(v, *neighborValue);
        });

        if (no_conflict) {
            candidates.push_back(v);
        }
    }

    thread_local std::mt19937 mt(std::random_device{}());

    std::ranges::shuffle(candidates, mt);

    for (const auto val : candidates) {
        assignment[u] = val;
        visited[val] = true;
        if (solve(assignment, visited))
            return true;

        assignment[u] = std::nullopt;
        visited[val] = false;
    }

    return false;
}
