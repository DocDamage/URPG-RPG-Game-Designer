#pragma once

#include <cmath>
#include <cstdint>
#include <vector>

namespace urpg::level {

/**
 * @brief Simple Grid-based FOV (Field of View) implementation.
 * Uses recursive shadowcasting or basic raycasting for the baseline.
 */
class FOVSystem {
  public:
    struct Point {
        int32_t x, y;
    };

    /**
     * @brief Computes visible cells from an origin within a max radius.
     */
    static std::vector<Point> computeFOV(int32_t originX, int32_t originY, int32_t radius, const auto& isBlocking) {
        std::vector<Point> visible;
        visible.push_back({originX, originY});

        // Basic ray-casting approach for the baseline
        for (int i = 0; i < 360; i += 2) {
            float rad = i * 0.0174533f;
            float dx = std::cos(rad);
            float dy = std::sin(rad);

            float cx = originX + 0.5f;
            float cy = originY + 0.5f;

            for (int r = 0; r < radius; ++r) {
                cx += dx;
                cy += dy;
                int32_t tx = static_cast<int32_t>(cx);
                int32_t ty = static_cast<int32_t>(cy);

                visible.push_back({tx, ty});
                if (isBlocking(tx, ty))
                    break;
            }
        }
        return visible;
    }
};

} // namespace urpg::level
