#include "engine/core/map/grid_part_runtime_compiler.h"

#include "engine/core/scene/map_scene.h"

#include <algorithm>
#include <charconv>
#include <limits>
#include <set>
#include <string_view>

namespace urpg::map {

namespace {

constexpr const char* kTerrainLayer = "terrain";
constexpr const char* kCollisionLayer = "collision";

std::string propertyOr(const PlacedPartInstance& instance, std::string_view key, std::string fallback = {}) {
    const auto found = instance.properties.find(std::string(key));
    return found == instance.properties.end() ? std::move(fallback) : found->second;
}

int32_t intPropertyOr(const PlacedPartInstance& instance, std::string_view key, int32_t fallback = 0) {
    const auto value = propertyOr(instance, key);
    if (value.empty()) {
        return fallback;
    }

    int32_t parsed = fallback;
    const auto* begin = value.data();
    const auto* end = value.data() + value.size();
    const auto [ptr, ec] = std::from_chars(begin, end, parsed);
    return ec == std::errc{} && ptr == end ? parsed : fallback;
}

bool boolPropertyOr(const PlacedPartInstance& instance, std::string_view key, bool fallback = false) {
    const auto value = propertyOr(instance, key);
    if (value == "true" || value == "1") {
        return true;
    }
    if (value == "false" || value == "0") {
        return false;
    }
    return fallback;
}

bool compilesAsProp(GridPartCategory category) {
    switch (category) {
    case GridPartCategory::Door:
    case GridPartCategory::Npc:
    case GridPartCategory::TreasureChest:
    case GridPartCategory::SavePoint:
    case GridPartCategory::Shop:
    case GridPartCategory::QuestItem:
    case GridPartCategory::Prop:
        return true;
    default:
        return false;
    }
}

bool compilesAsRegion(GridPartCategory category) {
    return category == GridPartCategory::Hazard || category == GridPartCategory::Trigger ||
           category == GridPartCategory::CutsceneZone;
}

bool blocksNavigation(const PlacedPartInstance& instance, const GridPartDefinition& definition) {
    return definition.collision_policy == GridPartCollisionPolicy::Solid || definition.footprint.blocks_navigation ||
           instance.category == GridPartCategory::Wall;
}

void fillFootprint(TileLayerDocument& document, const std::string& layerId, const PlacedPartInstance& instance,
                   int32_t tileId) {
    for (int32_t y = instance.grid_y; y < instance.grid_y + instance.height; ++y) {
        for (int32_t x = instance.grid_x; x < instance.grid_x + instance.width; ++x) {
            (void)document.setTile(layerId, x, y, tileId);
        }
    }
}

void addBaseLayers(TileLayerDocument& document) {
    document.addLayer({kTerrainLayer, true, false, false, true, 0, {}});
    document.addLayer({kCollisionLayer, true, false, true, false, 1, {}});
}

presentation::PropInstance toPropInstance(const PlacedPartInstance& instance, const GridPartDefinition& definition) {
    presentation::PropInstance prop;
    prop.instanceId = instance.instance_id;
    prop.assetId = definition.asset_id.empty() ? instance.part_id : definition.asset_id;
    prop.posX = static_cast<float>(instance.grid_x);
    prop.posY = static_cast<float>(instance.grid_z);
    prop.posZ = static_cast<float>(instance.grid_y);
    prop.rotY = instance.rot_y;
    prop.scale = instance.scale;
    return prop;
}

SpawnEntry toSpawnEntry(const PlacedPartInstance& instance, const GridPartDefinition& definition) {
    SpawnEntry entry;
    entry.id = instance.instance_id;
    entry.enemy_id =
        propertyOr(instance, "enemyId", definition.asset_id.empty() ? instance.part_id : definition.asset_id);
    entry.x = instance.grid_x;
    entry.y = instance.grid_y;
    entry.cooldown_seconds = intPropertyOr(instance, "cooldownSeconds", 0);
    entry.persistent = boolPropertyOr(instance, "persistent", false);
    return entry;
}

MapRegionRule toRegionRule(const PlacedPartInstance& instance) {
    MapRegionRule rule;
    rule.id = instance.instance_id;
    rule.x = instance.grid_x;
    rule.y = instance.grid_y;
    rule.width = instance.width;
    rule.height = instance.height;

    if (instance.category == GridPartCategory::Hazard) {
        rule.hazard = propertyOr(instance, "hazard", instance.part_id);
    }
    if (instance.category == GridPartCategory::Trigger || instance.category == GridPartCategory::CutsceneZone) {
        rule.event_id = propertyOr(instance, "eventId", instance.instance_id);
    }
    return rule;
}

void sortOutputs(GridPartRuntimeCompileResult& result) {
    std::sort(result.spatial_overlay.props.begin(), result.spatial_overlay.props.end(),
              [](const auto& left, const auto& right) {
                  if (left.assetId != right.assetId) {
                      return left.assetId < right.assetId;
                  }
                  if (left.posZ != right.posZ) {
                      return left.posZ < right.posZ;
                  }
                  return left.posX < right.posX;
              });

    std::sort(result.spawn_table.entries.begin(), result.spawn_table.entries.end(),
              [](const auto& left, const auto& right) { return left.id < right.id; });

    std::sort(result.region_rules.begin(), result.region_rules.end(),
              [](const auto& left, const auto& right) { return left.id < right.id; });

    std::sort(result.compiled_instance_ids.begin(), result.compiled_instance_ids.end());
    result.compiled_instance_ids.erase(std::unique(result.compiled_instance_ids.begin(),
                                                   result.compiled_instance_ids.end()),
                                       result.compiled_instance_ids.end());
    std::sort(result.compiled_chunks.begin(), result.compiled_chunks.end(),
              [](const auto& left, const auto& right) {
                  if (left.chunk_y != right.chunk_y) {
                      return left.chunk_y < right.chunk_y;
                  }
                  return left.chunk_x < right.chunk_x;
              });
    result.compiled_chunks.erase(std::unique(result.compiled_chunks.begin(), result.compiled_chunks.end()),
                                 result.compiled_chunks.end());
}

bool sameChunk(const GridPartChunkCoord& left, const GridPartChunkCoord& right) {
    return left.chunk_x == right.chunk_x && left.chunk_y == right.chunk_y;
}

bool chunkLess(const GridPartChunkCoord& left, const GridPartChunkCoord& right) {
    if (left.chunk_y != right.chunk_y) {
        return left.chunk_y < right.chunk_y;
    }
    return left.chunk_x < right.chunk_x;
}

bool chunkInsideDocument(const GridPartDocument& document, const GridPartChunkCoord& chunk) {
    return chunk.chunk_x >= 0 && chunk.chunk_y >= 0 && chunk.chunk_x * document.chunkSize() < document.width() &&
           chunk.chunk_y * document.chunkSize() < document.height();
}

uint16_t clampSceneTileId(int32_t tileId) {
    if (tileId <= 0) {
        return 0;
    }
    return static_cast<uint16_t>(std::min<int32_t>(tileId, std::numeric_limits<uint16_t>::max()));
}

std::vector<GridPartChunkCoord> normalizeChunks(const GridPartDocument& document,
                                                const std::vector<GridPartChunkCoord>& chunks) {
    std::vector<GridPartChunkCoord> normalized;
    for (const auto& chunk : chunks) {
        if (chunkInsideDocument(document, chunk)) {
            normalized.push_back(chunk);
        }
    }
    std::sort(normalized.begin(), normalized.end(), chunkLess);
    normalized.erase(std::unique(normalized.begin(), normalized.end()), normalized.end());
    return normalized;
}

bool partIntersectsAnyChunk(const GridPartDocument& document, const PlacedPartInstance& instance,
                            const std::vector<GridPartChunkCoord>& chunks) {
    const auto footprintChunks = document.chunkCoordsForFootprint(instance);
    return std::any_of(footprintChunks.begin(), footprintChunks.end(), [&](const GridPartChunkCoord& footprintChunk) {
        return std::any_of(chunks.begin(), chunks.end(),
                           [&](const GridPartChunkCoord& chunk) { return sameChunk(footprintChunk, chunk); });
    });
}

GridPartRuntimeCompileResult compileGridPartRuntimeFiltered(const GridPartDocument& document,
                                                            const GridPartCatalog& catalog,
                                                            const std::vector<GridPartChunkCoord>* chunks) {
    const auto normalizedChunks = chunks == nullptr ? std::vector<GridPartChunkCoord>{} : normalizeChunks(document, *chunks);
    GridPartRuntimeCompileResult result;
    result.tile_document = TileLayerDocument(document.width(), document.height());
    addBaseLayers(result.tile_document);
    result.spatial_overlay.mapId = document.mapId();
    result.spawn_table.id = document.mapId() + ":grid_parts";
    if (chunks != nullptr) {
        result.partial_compile = true;
        result.compiled_chunks = normalizedChunks;
    }

    const auto validation = ValidateGridPartDocument(document, catalog);
    result.diagnostics = validation.diagnostics;
    if (!validation.ok) {
        result.ok = false;
        return result;
    }

    for (const auto& instance : document.parts()) {
        if (chunks != nullptr && !partIntersectsAnyChunk(document, instance, normalizedChunks)) {
            continue;
        }

        const auto* definition = catalog.find(instance.part_id);
        if (definition == nullptr) {
            continue;
        }

        result.compiled_instance_ids.push_back(instance.instance_id);
        if (chunks == nullptr) {
            for (const auto& coord : document.chunkCoordsForFootprint(instance)) {
                result.compiled_chunks.push_back(coord);
            }
        }

        if (instance.category == GridPartCategory::Tile || instance.category == GridPartCategory::Wall) {
            fillFootprint(result.tile_document, kTerrainLayer, instance, definition->tile_id);
        }
        if (blocksNavigation(instance, *definition)) {
            fillFootprint(result.tile_document, kCollisionLayer, instance, 1);
        }
        if (compilesAsProp(instance.category)) {
            result.spatial_overlay.props.push_back(toPropInstance(instance, *definition));
        }
        if (instance.category == GridPartCategory::Enemy) {
            result.spawn_table.entries.push_back(toSpawnEntry(instance, *definition));
        }
        if (compilesAsRegion(instance.category)) {
            result.region_rules.push_back(toRegionRule(instance));
        }
    }

    sortOutputs(result);
    return result;
}

} // namespace

GridPartRuntimeCompileResult CompileGridPartRuntime(const GridPartDocument& document, const GridPartCatalog& catalog) {
    return compileGridPartRuntimeFiltered(document, catalog, nullptr);
}

GridPartRuntimeCompileResult CompileGridPartRuntimeForChunks(const GridPartDocument& document,
                                                             const GridPartCatalog& catalog,
                                                             const std::vector<GridPartChunkCoord>& chunks) {
    return compileGridPartRuntimeFiltered(document, catalog, &chunks);
}

GridPartRuntimeCompileResult CompileGridPartRuntimeForDirtyChunks(const GridPartDocument& document,
                                                                  const GridPartCatalog& catalog) {
    std::vector<GridPartChunkCoord> chunks;
    for (const auto& chunk : document.dirtyChunks()) {
        chunks.push_back(chunk.coord);
    }
    return CompileGridPartRuntimeForChunks(document, catalog, chunks);
}

bool ApplyGridPartRuntimeToMapScene(const GridPartRuntimeCompileResult& result, scene::MapScene& scene) {
    if (!result.ok || result.partial_compile || result.tile_document.width() != scene.getWidth() ||
        result.tile_document.height() != scene.getHeight()) {
        return false;
    }

    std::vector<int> terrain;
    terrain.reserve(static_cast<size_t>(result.tile_document.width() * result.tile_document.height()));
    for (int32_t y = 0; y < result.tile_document.height(); ++y) {
        for (int32_t x = 0; x < result.tile_document.width(); ++x) {
            terrain.push_back(clampSceneTileId(result.tile_document.tileAt(kTerrainLayer, x, y).value_or(0)));
        }
    }
    scene.setLayerData(0, terrain);

    for (int32_t y = 0; y < result.tile_document.height(); ++y) {
        for (int32_t x = 0; x < result.tile_document.width(); ++x) {
            const auto tileId = result.tile_document.tileAt(kTerrainLayer, x, y).value_or(0);
            const auto collision = result.tile_document.tileAt(kCollisionLayer, x, y).value_or(0) != 0;
            scene.setTile(x, y, clampSceneTileId(tileId), !collision);
            scene.setTilePassable(x, y, !collision);
        }
    }

    return true;
}

} // namespace urpg::map
