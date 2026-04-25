#pragma once

#include <cstdint>
#include <set>
#include <vector>

namespace urpg::map {

struct GridCell {
    int32_t x = 0;
    int32_t y = 0;

    friend bool operator<(const GridCell& a, const GridCell& b) {
        if (a.y != b.y) {
            return a.y < b.y;
        }
        return a.x < b.x;
    }
};

std::set<GridCell> PreviewTacticalMoveRange(int32_t width,
                                            int32_t height,
                                            GridCell origin,
                                            int32_t move_range,
                                            const std::set<GridCell>& blocked_cells);

} // namespace urpg::map
