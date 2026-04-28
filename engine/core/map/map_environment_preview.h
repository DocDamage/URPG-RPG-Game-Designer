#pragma once

#include "engine/core/map/map_region_rules.h"
#include "engine/core/presentation/presentation_runtime.h"

#include <nlohmann/json.hpp>

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace urpg::map {

struct MapEnvironmentRegion {
    std::string id;
    int32_t x = 0;
    int32_t y = 0;
    int32_t width = 1;
    int32_t height = 1;
    std::string weather = "clear";
    std::string lighting = "day";
    presentation::LightProfile light;
    presentation::FogProfile fog;
    presentation::PostFXProfile post_fx;

    bool contains(int32_t tile_x, int32_t tile_y) const;
};

struct MapEnvironmentPreviewDocument {
    std::string map_id;
    int32_t width = 0;
    int32_t height = 0;
    std::string base_weather = "clear";
    presentation::LightProfile base_light;
    presentation::FogProfile base_fog;
    presentation::PostFXProfile base_post_fx;
    std::vector<MapEnvironmentRegion> regions;

    std::vector<MapDiagnostic> validate() const;
    const MapEnvironmentRegion* regionAt(int32_t tile_x, int32_t tile_y) const;
    presentation::PresentationFrameIntent buildRuntimePreview(int32_t tile_x, int32_t tile_y) const;
    nlohmann::json toJson() const;

    static MapEnvironmentPreviewDocument fromJson(const nlohmann::json& json);
    static MapEnvironmentPreviewDocument fromRegionRules(std::string map_id, int32_t width, int32_t height,
                                                         const std::vector<MapRegionRule>& rules);
};

struct MapEnvironmentPreviewResult {
    const MapEnvironmentRegion* region = nullptr;
    presentation::PresentationFrameIntent runtime_intent;
    std::vector<MapDiagnostic> diagnostics;
};

MapEnvironmentPreviewResult PreviewMapEnvironment(const MapEnvironmentPreviewDocument& document, int32_t tile_x,
                                                  int32_t tile_y);

} // namespace urpg::map
