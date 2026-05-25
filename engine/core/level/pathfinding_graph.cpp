#include "engine/core/level/pathfinding_graph.h"

#include <algorithm>
#include <cstdlib>
#include <limits>
#include <queue>
#include <set>

namespace urpg::level {
namespace {

int32_t manhattan(PathGridPoint lhs, PathGridPoint rhs) {
    return std::abs(lhs.x - rhs.x) + std::abs(lhs.y - rhs.y);
}

std::vector<PathGridPoint> neighbors(PathGridPoint point) {
    return {
        {point.x + 1, point.y},
        {point.x, point.y + 1},
        {point.x - 1, point.y},
        {point.x, point.y - 1},
    };
}

std::string defaultBlockReason(PathGridPoint point) {
    return "blocked:" + std::to_string(point.x) + ":" + std::to_string(point.y);
}

} // namespace

PathfindingGraph::PathfindingGraph(int32_t width, int32_t height)
    : width_(std::max(0, width)), height_(std::max(0, height)),
      cells_(static_cast<size_t>(width_) * static_cast<size_t>(height_)) {}

bool PathfindingGraph::contains(PathGridPoint point) const {
    return point.x >= 0 && point.y >= 0 && point.x < width_ && point.y < height_;
}

bool PathfindingGraph::blocked(PathGridPoint point) const {
    return !contains(point) || cells_[index(point)].blocked;
}

int32_t PathfindingGraph::cellCost(PathGridPoint point) const {
    if (!contains(point)) {
        return std::numeric_limits<int32_t>::max();
    }
    return cells_[index(point)].cost;
}

std::string PathfindingGraph::blockReason(PathGridPoint point) const {
    if (!contains(point)) {
        return "out_of_bounds";
    }
    const auto& cell = cells_[index(point)];
    if (!cell.blocked) {
        return {};
    }
    return cell.reason.empty() ? defaultBlockReason(point) : cell.reason;
}

bool PathfindingGraph::setBlocked(int32_t x, int32_t y, bool isBlocked, std::string reason) {
    const PathGridPoint point{x, y};
    if (!contains(point)) {
        return false;
    }
    auto& cell = cells_[index(point)];
    cell.blocked = isBlocked;
    cell.reason = isBlocked ? std::move(reason) : std::string{};
    return true;
}

bool PathfindingGraph::setCellCost(int32_t x, int32_t y, int32_t cost) {
    const PathGridPoint point{x, y};
    if (!contains(point)) {
        return false;
    }
    cells_[index(point)].cost = std::max(1, cost);
    return true;
}

PathfindingResult PathfindingGraph::findPath(PathGridPoint start, PathGridPoint goal) const {
    PathfindingResult result;
    if (!contains(start)) {
        result.reason = "start_out_of_bounds";
        result.diagnostics.push_back({"start_out_of_bounds", start, "out_of_bounds"});
        return result;
    }
    if (!contains(goal)) {
        result.reason = "goal_out_of_bounds";
        result.diagnostics.push_back({"goal_out_of_bounds", goal, "out_of_bounds"});
        return result;
    }
    if (blocked(start)) {
        result.reason = "start_blocked";
        result.diagnostics.push_back({"start_blocked", start, blockReason(start)});
        return result;
    }
    if (blocked(goal)) {
        result.reason = "goal_blocked";
        result.diagnostics.push_back({"goal_blocked", goal, blockReason(goal)});
        return result;
    }

    struct QueueNode {
        PathGridPoint point;
        int32_t cost = 0;
        int32_t estimate = 0;
        int32_t sequence = 0;
    };
    struct Compare {
        bool operator()(const QueueNode& lhs, const QueueNode& rhs) const {
            if (lhs.estimate != rhs.estimate) {
                return lhs.estimate > rhs.estimate;
            }
            if (lhs.cost != rhs.cost) {
                return lhs.cost > rhs.cost;
            }
            return lhs.sequence > rhs.sequence;
        }
    };

    const size_t cellCount = cells_.size();
    std::vector<int32_t> bestCost(cellCount, std::numeric_limits<int32_t>::max());
    std::vector<int32_t> previous(cellCount, -1);
    std::priority_queue<QueueNode, std::vector<QueueNode>, Compare> frontier;
    std::set<std::pair<int32_t, int32_t>> reportedBlocked;
    std::vector<PathfindingDiagnostic> blockedDiagnostics;
    int32_t sequence = 0;

    bestCost[index(start)] = 0;
    frontier.push({start, 0, manhattan(start, goal), sequence++});

    while (!frontier.empty()) {
        const auto current = frontier.top();
        frontier.pop();
        if (current.cost != bestCost[index(current.point)]) {
            continue;
        }
        if (current.point == goal) {
            result.found = true;
            result.reason = "found";
            result.total_cost = current.cost;

            std::vector<PathGridPoint> reversed;
            int32_t cursor = static_cast<int32_t>(index(goal));
            while (cursor >= 0) {
                const int32_t x = cursor % width_;
                const int32_t y = cursor / width_;
                reversed.push_back({x, y});
                cursor = previous[static_cast<size_t>(cursor)];
            }
            result.nodes.assign(reversed.rbegin(), reversed.rend());
            return result;
        }

        for (const auto next : neighbors(current.point)) {
            if (!contains(next)) {
                continue;
            }
            if (blocked(next)) {
                if (reportedBlocked.insert({next.x, next.y}).second) {
                    blockedDiagnostics.push_back({"neighbor_blocked", next, blockReason(next)});
                }
                continue;
            }

            const int32_t nextCost = current.cost + cellCost(next);
            const size_t nextIndex = index(next);
            if (nextCost >= bestCost[nextIndex]) {
                continue;
            }
            bestCost[nextIndex] = nextCost;
            previous[nextIndex] = static_cast<int32_t>(index(current.point));
            frontier.push({next, nextCost, nextCost + manhattan(next, goal), sequence++});
        }
    }

    result.reason = "no_route";
    result.diagnostics = std::move(blockedDiagnostics);
    return result;
}

size_t PathfindingGraph::index(PathGridPoint point) const {
    return static_cast<size_t>(point.y) * static_cast<size_t>(width_) + static_cast<size_t>(point.x);
}

} // namespace urpg::level
