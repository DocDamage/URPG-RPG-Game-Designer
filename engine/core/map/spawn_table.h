#pragma once

#include "engine/core/map/tile_layer_document.h"

#include <nlohmann/json.hpp>

#include <set>
#include <string>
#include <vector>

namespace urpg::map {

struct SpawnEntry {
    std::string id;
    std::string enemy_id;
    int32_t x = 0;
    int32_t y = 0;
    int32_t cooldown_seconds = 0;
    bool persistent = false;
};

struct SpawnTable {
    std::string id;
    std::vector<SpawnEntry> entries;
};

nlohmann::json SpawnTableToJson(const SpawnTable& table);
SpawnTable SpawnTableFromJson(const nlohmann::json& json);
std::vector<MapDiagnostic> ValidateSpawnTable(const SpawnTable& table, const std::set<std::string>& known_enemies);

} // namespace urpg::map
