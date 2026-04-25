#pragma once

#include "engine/core/map/tile_layer_document.h"

#include <string>
#include <vector>

namespace urpg::map {

struct MapRegionRule {
    std::string id;
    int32_t x = 0;
    int32_t y = 0;
    int32_t width = 1;
    int32_t height = 1;
    std::string encounter_table;
    std::string ambient_audio;
    std::string weather;
    std::string hazard;
    std::string movement_rule;
    std::string event_id;
};

std::vector<MapDiagnostic> ValidateMapRegionRules(const std::vector<MapRegionRule>& rules);

} // namespace urpg::map
