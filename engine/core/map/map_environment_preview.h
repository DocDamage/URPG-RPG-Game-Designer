#pragma once

#include "engine/core/map/map_region_rules.h"
#include "engine/core/map/spawn_table.h"
#include "engine/core/map/tactical_grid_preview.h"
#include "engine/core/map/tile_layer_document.h"
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

struct MapTacticalOverlayPreview {
    bool enabled = false;
    int32_t origin_x = 0;
    int32_t origin_y = 0;
    int32_t move_range = 0;
};

struct MapParallaxLayer {
    std::string id;
    std::string asset;
    std::string lock_mode = "map";
    std::string blend_mode = "normal";
    float opacity = 1.0f;
    float scroll_x = 0.0f;
    float scroll_y = 0.0f;
    int32_t region_id = 0;
    int32_t draw_order = 0;
};

struct MapSmokeEmitter {
    std::string id;
    int32_t region_id = 0;
    int32_t intensity = 0;
    float size = 1.0f;
    int32_t opacity = 128;
    std::string color = "#ffffff";
    bool bloom = false;
    float drift_x = 0.0f;
    float drift_y = -0.25f;
};

struct MapTerrainMeshRule {
    std::string id;
    int32_t region_id = 0;
    float height = 0.0f;
    float curve = 0.0f;
    bool walkable = true;
};

struct MapEdgeScrollCameraProfile {
    bool enabled = false;
    bool keyboard_pan = false;
    bool click_follow = false;
    float speed_tiles_per_second = 8.0f;
    int32_t margin_pixels = 24;
    std::string recenter_hotkey = "Space";
};

struct MapEnvironmentPreviewDocument {
    std::string map_id;
    int32_t width = 0;
    int32_t height = 0;
    std::string base_weather = "clear";
    presentation::LightProfile base_light;
    presentation::FogProfile base_fog;
    presentation::PostFXProfile base_post_fx;
    std::vector<TileLayer> tile_layers;
    std::vector<MapEnvironmentRegion> regions;
    std::vector<MapParallaxLayer> parallax_layers;
    std::vector<MapSmokeEmitter> smoke_emitters;
    std::vector<MapTerrainMeshRule> terrain_mesh_rules;
    MapEdgeScrollCameraProfile edge_scroll_camera;
    MapTacticalOverlayPreview tactical_overlay;
    SpawnTable spawn_table;

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
    std::vector<std::string> runtime_overlay_commands;
    std::vector<MapDiagnostic> diagnostics;
    std::vector<GridCell> tactical_reachable_cells;
    size_t visible_tile_layer_count = 0;
    size_t collision_tile_count = 0;
    size_t region_overlay_count = 0;
    size_t parallax_layer_count = 0;
    size_t smoke_emitter_count = 0;
    size_t terrain_mesh_rule_count = 0;
    bool edge_scroll_enabled = false;
    size_t tactical_reachable_count = 0;
    size_t spawn_entry_count = 0;
    bool selected_tile_blocked = false;
};

MapEnvironmentPreviewResult PreviewMapEnvironment(const MapEnvironmentPreviewDocument& document, int32_t tile_x,
                                                  int32_t tile_y);

} // namespace urpg::map
