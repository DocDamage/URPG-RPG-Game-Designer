#pragma once

#include "engine/core/map/tile_layer_document.h"

#include <string>

namespace urpg::map {

struct ProceduralMapProfile {
    std::string id;
    std::string type = "dungeon";
    int32_t width = 16;
    int32_t height = 16;
    uint32_t seed = 1;
    bool require_boss = false;
    bool require_key = false;
    bool require_shop = false;
};

struct ProceduralMapResult {
    TileLayerDocument document;
    std::vector<MapDiagnostic> diagnostics;
};

ProceduralMapResult GenerateProceduralMap(const ProceduralMapProfile& profile);

} // namespace urpg::map
