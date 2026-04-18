#include "engine/core/level/level_assembly.h"
#include <algorithm>

namespace urpg::level {

bool LevelAssemblyWorkspace::placeBlock(const std::string& blockId, int32_t x, int32_t y, int32_t z) {
    // 1. Collision check (one block per cell for this baseline)
    if (hasBlockAt(x, y, z)) {
        return false;
    }

    // 2. Add to workspace
    m_placedBlocks.push_back({blockId, x, y, z});
    return true;
}

bool LevelAssemblyWorkspace::hasBlockAt(int32_t x, int32_t y, int32_t z) const {
    return std::any_of(m_placedBlocks.begin(), m_placedBlocks.end(), 
        [=](const auto& b) {
            return b.x == x && b.y == y && b.z == z;
        });
}

} // namespace urpg::level
