#pragma once

#include "engine/core/presentation/presentation_schema.h"
#include "engine/core/presentation/presentation_types.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace urpg::render {

/**
 * @brief Simple 2.5D Raycast Renderer Implementation.
 * Maps a 2D grid to a pseudo-3D perspective.
 */
class RaycastRenderer {
  public:
    struct AuthoringAdapter {
        std::string mapId;
        int32_t width = 0;
        int32_t height = 0;
        std::vector<uint8_t> blockingCells;

        bool isBlocking(int32_t x, int32_t y) const {
            if (x < 0 || y < 0 || x >= width || y >= height) {
                return true;
            }
            const size_t index = static_cast<size_t>(y * width + x);
            return index < blockingCells.size() && blockingCells[index] != 0;
        }
    };

    struct Config {
        int32_t screenWidth = 640;
        int32_t screenHeight = 480;
        float fov = 0.66f; // Field of view
        presentation::PresentationMode presentationMode = presentation::PresentationMode::Classic2D;
    };

    struct Camera {
        float posX, posY;
        float dirX, dirY;
        float planeX, planeY; // 2D camera plane
    };

    struct CastResult {
        int32_t x;          // Screen x
        int32_t mapX, mapY; // What map cell was hit
        int32_t side;       // 0 for N/S, 1 for E/W
        float wallDist;     // Perpendicular distance to wall
    };

    /**
     * @brief Performs raycasting for a single frame.
     */
    static std::vector<CastResult> castFrame(const Camera& cam, const Config& config, const auto& isBlocking) {
        std::vector<CastResult> results;
        if (config.presentationMode != presentation::PresentationMode::Spatial) {
            return results;
        }

        results.reserve(config.screenWidth);

        for (int x = 0; x < config.screenWidth; ++x) {
            // Calculate ray position and direction
            float cameraX = 2 * x / static_cast<float>(config.screenWidth) - 1;
            float rayDirX = cam.dirX + cam.planeX * cameraX;
            float rayDirY = cam.dirY + cam.planeY * cameraX;

            int mapX = static_cast<int>(cam.posX);
            int mapY = static_cast<int>(cam.posY);

            // Distance from ray to next x or y side
            float sideDistX, sideDistY;

            // Distance the ray has to travel to go from 1 x or y-side to the next x or y-side
            float deltaDistX = (rayDirX == 0) ? 1e30f : std::abs(1 / rayDirX);
            float deltaDistY = (rayDirY == 0) ? 1e30f : std::abs(1 / rayDirY);
            float perpWallDist;

            // What direction to step in x or y-direction (either +1 or -1)
            int stepX, stepY;
            int hit = 0; // Was there a wall hit?
            int side;    // Was a NS or a EW wall hit?

            // Calculate step and initial sideDist
            if (rayDirX < 0) {
                stepX = -1;
                sideDistX = (cam.posX - mapX) * deltaDistX;
            } else {
                stepX = 1;
                sideDistX = (mapX + 1.0f - cam.posX) * deltaDistX;
            }
            if (rayDirY < 0) {
                stepY = -1;
                sideDistY = (cam.posY - mapY) * deltaDistY;
            } else {
                stepY = 1;
                sideDistY = (mapY + 1.0f - cam.posY) * deltaDistY;
            }

            // Perform DDA (Digital Differential Analyzer)
            while (hit == 0) {
                // Jump to next map square, either in x-direction, or in y-direction
                if (sideDistX < sideDistY) {
                    sideDistX += deltaDistX;
                    mapX += stepX;
                    side = 0;
                } else {
                    sideDistY += deltaDistY;
                    mapY += stepY;
                    side = 1;
                }
                // Check if ray has hit a wall
                if (isBlocking(mapX, mapY))
                    hit = 1;
            }

            // Calculate distance projected on camera direction (Euclidean distance will give fisheye effect!)
            if (side == 0)
                perpWallDist = (sideDistX - deltaDistX);
            else
                perpWallDist = (sideDistY - deltaDistY);

            results.push_back({x, mapX, mapY, side, perpWallDist});
        }
        return results;
    }

    static AuthoringAdapter buildAuthoringAdapter(const presentation::SpatialMapOverlay& overlay) {
        AuthoringAdapter adapter;
        adapter.mapId = overlay.mapId;
        adapter.width = static_cast<int32_t>(overlay.elevation.width);
        adapter.height = static_cast<int32_t>(overlay.elevation.height);
        adapter.blockingCells.assign(static_cast<size_t>(adapter.width * adapter.height), 0);

        for (int32_t y = 0; y < adapter.height; ++y) {
            for (int32_t x = 0; x < adapter.width; ++x) {
                const size_t index = static_cast<size_t>(y * adapter.width + x);
                if (index < overlay.elevation.levels.size() && overlay.elevation.levels[index] > 0) {
                    adapter.blockingCells[index] = 1;
                }
            }
        }

        return adapter;
    }
};

} // namespace urpg::render
