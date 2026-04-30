#pragma once

#include "engine/core/map/grid_part_catalog.h"
#include "engine/core/map/grid_part_document.h"
#include "engine/core/map/grid_part_validator.h"
#include "engine/core/map/map_region_rules.h"
#include "engine/core/map/spawn_table.h"
#include "engine/core/map/tile_layer_document.h"
#include "engine/core/presentation/presentation_schema.h"

namespace urpg::scene {
class MapScene;
}

namespace urpg::map {

struct GridPartRuntimeCompileResult {
    bool ok = true;
    bool partial_compile = false;
    std::vector<GridPartDiagnostic> diagnostics;
    std::vector<GridPartChunkCoord> compiled_chunks;
    std::vector<std::string> compiled_instance_ids;

    TileLayerDocument tile_document;
    presentation::SpatialMapOverlay spatial_overlay;
    SpawnTable spawn_table;
    std::vector<MapRegionRule> region_rules;
};

GridPartRuntimeCompileResult CompileGridPartRuntime(const GridPartDocument& document, const GridPartCatalog& catalog);
GridPartRuntimeCompileResult CompileGridPartRuntimeForChunks(const GridPartDocument& document,
                                                             const GridPartCatalog& catalog,
                                                             const std::vector<GridPartChunkCoord>& chunks);
GridPartRuntimeCompileResult CompileGridPartRuntimeForDirtyChunks(const GridPartDocument& document,
                                                                  const GridPartCatalog& catalog);

bool ApplyGridPartRuntimeToMapScene(const GridPartRuntimeCompileResult& result, scene::MapScene& scene);

} // namespace urpg::map
