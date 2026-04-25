#include "engine/core/map/tactical_grid_preview.h"

#include <queue>

namespace urpg::map {

std::set<GridCell> PreviewTacticalMoveRange(int32_t width,
                                            int32_t height,
                                            GridCell origin,
                                            int32_t move_range,
                                            const std::set<GridCell>& blocked_cells) {
    std::set<GridCell> visited;
    if (width <= 0 || height <= 0 || move_range < 0 || blocked_cells.contains(origin)) {
        return visited;
    }

    std::queue<std::pair<GridCell, int32_t>> queue;
    visited.insert(origin);
    queue.push({origin, 0});

    const GridCell deltas[] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    while (!queue.empty()) {
        const auto [cell, dist] = queue.front();
        queue.pop();
        if (dist >= move_range) {
            continue;
        }
        for (const auto& delta : deltas) {
            GridCell next{cell.x + delta.x, cell.y + delta.y};
            if (next.x < 0 || next.y < 0 || next.x >= width || next.y >= height ||
                blocked_cells.contains(next) || visited.contains(next)) {
                continue;
            }
            visited.insert(next);
            queue.push({next, dist + 1});
        }
    }
    return visited;
}

} // namespace urpg::map
