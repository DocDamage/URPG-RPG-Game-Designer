#include "engine/core/map/procedural_map_generator.h"

#include <algorithm>

namespace urpg::map {

ProceduralMapResult GenerateProceduralMap(const ProceduralMapProfile& profile) {
    ProceduralMapResult result;
    const int32_t width = std::max(1, profile.width);
    const int32_t height = std::max(1, profile.height);
    result.document = TileLayerDocument(width, height);
    result.document.addLayer({"terrain", true, false, false, true, 0, {}});
    result.document.addLayer({"collision", true, false, true, false, 1, {}});

    if ((profile.require_boss || profile.require_key || profile.require_shop) && (width * height < 25)) {
        result.diagnostics.push_back({"unsatisfied_constraints", "Map is too small for required boss/key/shop constraints.", -1, -1, profile.id});
    }

    for (int32_t y = 0; y < height; ++y) {
        for (int32_t x = 0; x < width; ++x) {
            const bool border = x == 0 || y == 0 || x == width - 1 || y == height - 1;
            result.document.setTile("terrain", x, y, border ? 1 : 2);
            if (border) {
                result.document.setTile("collision", x, y, 1);
            }
        }
    }

    return result;
}

} // namespace urpg::map
