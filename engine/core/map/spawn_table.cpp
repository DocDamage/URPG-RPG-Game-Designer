#include "engine/core/map/spawn_table.h"

namespace urpg::map {

nlohmann::json SpawnTableToJson(const SpawnTable& table) {
    nlohmann::json json;
    json["id"] = table.id;
    json["entries"] = nlohmann::json::array();
    for (const auto& entry : table.entries) {
        json["entries"].push_back({
            {"id", entry.id},
            {"enemy_id", entry.enemy_id},
            {"x", entry.x},
            {"y", entry.y},
            {"cooldown_seconds", entry.cooldown_seconds},
            {"persistent", entry.persistent},
        });
    }
    return json;
}

SpawnTable SpawnTableFromJson(const nlohmann::json& json) {
    SpawnTable table;
    if (!json.is_object()) {
        return table;
    }
    table.id = json.value("id", "");
    if (const auto entries = json.find("entries"); entries != json.end() && entries->is_array()) {
        for (const auto& row : *entries) {
            if (!row.is_object()) {
                continue;
            }
            table.entries.push_back({
                row.value("id", ""),
                row.value("enemy_id", ""),
                row.value("x", 0),
                row.value("y", 0),
                row.value("cooldown_seconds", 0),
                row.value("persistent", false),
            });
        }
    }
    return table;
}

std::vector<MapDiagnostic> ValidateSpawnTable(const SpawnTable& table, const std::set<std::string>& known_enemies) {
    std::vector<MapDiagnostic> diagnostics;
    for (const auto& entry : table.entries) {
        if (!known_enemies.contains(entry.enemy_id)) {
            diagnostics.push_back({"missing_spawn_enemy", "Spawn table references missing enemy.", entry.x, entry.y, entry.enemy_id});
        }
    }
    return diagnostics;
}

} // namespace urpg::map
