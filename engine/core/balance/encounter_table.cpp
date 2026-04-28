#include "engine/core/balance/encounter_table.h"

#include <algorithm>
#include <random>
#include <set>
#include <utility>

namespace urpg::balance {

namespace {

EncounterRegion regionFromJson(const nlohmann::json& json) {
    EncounterRegion region;
    region.id = json.value("id", "");
    region.map_id = json.value("map_id", "");
    region.biome = json.value("biome", "");
    region.min_party_level = json.value("min_party_level", 1);
    region.max_party_level = json.value("max_party_level", 99);
    region.required_flags = json.value("required_flags", std::vector<std::string>{});
    return region;
}

nlohmann::json regionToJson(const EncounterRegion& region) {
    return {{"id", region.id},
            {"map_id", region.map_id},
            {"biome", region.biome},
            {"min_party_level", region.min_party_level},
            {"max_party_level", region.max_party_level},
            {"required_flags", region.required_flags}};
}

EncounterDesignerEntry designerEntryFromJson(const nlohmann::json& json) {
    EncounterDesignerEntry entry;
    entry.id = json.value("id", "");
    entry.enemy_id = json.value("enemy_id", "");
    entry.region_id = json.value("region_id", "");
    entry.weight = json.value("weight", 0);
    entry.difficulty = json.value("difficulty", 1);
    entry.min_party_level = json.value("min_party_level", 1);
    entry.max_party_level = json.value("max_party_level", 99);
    entry.tags = json.value("tags", std::vector<std::string>{});
    entry.rewards = json.value("rewards", std::vector<std::string>{});
    return entry;
}

nlohmann::json designerEntryToJson(const EncounterDesignerEntry& entry) {
    return {{"id", entry.id},
            {"enemy_id", entry.enemy_id},
            {"region_id", entry.region_id},
            {"weight", entry.weight},
            {"difficulty", entry.difficulty},
            {"min_party_level", entry.min_party_level},
            {"max_party_level", entry.max_party_level},
            {"tags", entry.tags},
            {"rewards", entry.rewards}};
}

bool hasAllFlags(const std::vector<std::string>& required_flags, const std::set<std::string>& active_flags) {
    return std::all_of(required_flags.begin(), required_flags.end(), [&](const auto& flag) { return active_flags.contains(flag); });
}

} // namespace

void EncounterTable::addEncounter(EncounterEntry entry) {
    entries_.push_back(std::move(entry));
}

std::vector<EncounterDiagnostic> EncounterTable::validate() const {
    int32_t total_weight = 0;
    for (const auto& entry : entries_) {
        total_weight += std::max(0, entry.weight);
    }
    if (!entries_.empty() && total_weight == 0) {
        return {{"zero_weight_pool", "Encounter weights must include at least one positive weight."}};
    }
    return {};
}

std::vector<std::string> EncounterTable::preview(const std::string& region_id, uint64_t seed, std::size_t count) const {
    std::vector<EncounterEntry> pool;
    for (const auto& entry : entries_) {
        if (entry.region_id == region_id && entry.weight > 0) {
            pool.push_back(entry);
        }
    }
    std::stable_sort(pool.begin(), pool.end(), [](const auto& lhs, const auto& rhs) { return lhs.enemy_id < rhs.enemy_id; });
    std::vector<std::string> result;
    if (pool.empty()) {
        return result;
    }
    int32_t total_weight = 0;
    for (const auto& entry : pool) {
        total_weight += entry.weight;
    }
    std::mt19937_64 rng(seed);
    for (std::size_t i = 0; i < count; ++i) {
        const auto roll = static_cast<int32_t>(rng() % total_weight);
        int32_t cursor = 0;
        for (const auto& entry : pool) {
            cursor += entry.weight;
            if (roll < cursor) {
                result.push_back(entry.enemy_id);
                break;
            }
        }
    }
    return result;
}

std::vector<EncounterDiagnostic> EncounterDesignerDocument::validate(const std::set<std::string>& known_enemies,
                                                                      const std::set<std::string>& known_flags) const {
    std::vector<EncounterDiagnostic> diagnostics;
    std::set<std::string> region_ids;
    for (const auto& region : regions) {
        if (region.id.empty()) {
            diagnostics.push_back({"missing_region_id", "Encounter region is missing an id."});
            continue;
        }
        if (!region_ids.insert(region.id).second) {
            diagnostics.push_back({"duplicate_region_id", "Encounter region id is duplicated: " + region.id});
        }
        if (region.map_id.empty()) {
            diagnostics.push_back({"missing_region_map", "Encounter region is missing a map id: " + region.id});
        }
        if (region.min_party_level > region.max_party_level) {
            diagnostics.push_back({"invalid_region_level_range", "Encounter region has an invalid party level range: " + region.id});
        }
        for (const auto& flag : region.required_flags) {
            if (!known_flags.empty() && !known_flags.contains(flag)) {
                diagnostics.push_back({"unknown_region_flag", "Encounter region requires an unknown flag: " + flag});
            }
        }
    }

    std::set<std::string> encounter_ids;
    for (const auto& entry : encounters) {
        if (entry.id.empty()) {
            diagnostics.push_back({"missing_encounter_id", "Encounter entry is missing an id."});
        } else if (!encounter_ids.insert(entry.id).second) {
            diagnostics.push_back({"duplicate_encounter_id", "Encounter entry id is duplicated: " + entry.id});
        }
        if (entry.enemy_id.empty()) {
            diagnostics.push_back({"missing_enemy_id", "Encounter entry is missing an enemy id: " + entry.id});
        } else if (!known_enemies.empty() && !known_enemies.contains(entry.enemy_id)) {
            diagnostics.push_back({"unknown_enemy_id", "Encounter entry references an unknown enemy: " + entry.enemy_id});
        }
        if (!region_ids.contains(entry.region_id)) {
            diagnostics.push_back({"unknown_encounter_region", "Encounter entry references an unknown region: " + entry.region_id});
        }
        if (entry.weight <= 0) {
            diagnostics.push_back({"invalid_encounter_weight", "Encounter entry weight must be positive: " + entry.id});
        }
        if (entry.difficulty <= 0) {
            diagnostics.push_back({"invalid_encounter_difficulty", "Encounter entry difficulty must be positive: " + entry.id});
        }
        if (entry.min_party_level > entry.max_party_level) {
            diagnostics.push_back({"invalid_encounter_level_range", "Encounter entry has an invalid party level range: " + entry.id});
        }
    }
    return diagnostics;
}

EncounterTable EncounterDesignerDocument::toRuntimeTable() const {
    EncounterTable table;
    for (const auto& entry : encounters) {
        table.addEncounter({entry.enemy_id, entry.region_id, entry.weight, entry.difficulty});
    }
    return table;
}

EncounterDesignerPreview EncounterDesignerDocument::preview(const std::string& region_id,
                                                            int32_t party_level,
                                                            const std::set<std::string>& active_flags,
                                                            uint64_t seed,
                                                            std::size_t count) const {
    EncounterDesignerPreview result;
    result.region_id = region_id;
    result.party_level = party_level;
    result.seed = seed;
    result.diagnostics = validate();

    auto region_it = std::find_if(regions.begin(), regions.end(), [&](const auto& region) { return region.id == region_id; });
    if (region_it == regions.end()) {
        result.diagnostics.push_back({"missing_preview_region", "Preview region does not exist: " + region_id});
        return result;
    }
    if (party_level < region_it->min_party_level || party_level > region_it->max_party_level || !hasAllFlags(region_it->required_flags, active_flags)) {
        result.diagnostics.push_back({"locked_preview_region", "Preview region is locked by party level or required flags: " + region_id});
        return result;
    }

    std::vector<EncounterDesignerEntry> pool;
    for (const auto& entry : encounters) {
        if (entry.region_id == region_id && party_level >= entry.min_party_level && party_level <= entry.max_party_level && entry.weight > 0) {
            pool.push_back(entry);
        }
    }
    std::stable_sort(pool.begin(), pool.end(), [](const auto& lhs, const auto& rhs) { return lhs.id < rhs.id; });
    if (pool.empty()) {
        result.diagnostics.push_back({"empty_preview_pool", "Preview region has no encounters for the selected party level: " + region_id});
        return result;
    }

    int32_t total_weight = 0;
    for (const auto& entry : pool) {
        total_weight += entry.weight;
    }
    std::mt19937_64 rng(seed);
    for (std::size_t index = 0; index < count; ++index) {
        const auto roll = static_cast<int32_t>(rng() % total_weight);
        int32_t cursor = 0;
        for (const auto& entry : pool) {
            cursor += entry.weight;
            if (roll < cursor) {
                result.encounters.push_back({entry.id, entry.enemy_id, entry.difficulty, entry.rewards});
                break;
            }
        }
    }
    return result;
}

nlohmann::json EncounterDesignerDocument::toJson() const {
    nlohmann::json json;
    json["id"] = id;
    json["regions"] = nlohmann::json::array();
    for (const auto& region : regions) {
        json["regions"].push_back(regionToJson(region));
    }
    json["encounters"] = nlohmann::json::array();
    for (const auto& entry : encounters) {
        json["encounters"].push_back(designerEntryToJson(entry));
    }
    return json;
}

EncounterDesignerDocument EncounterDesignerDocument::fromJson(const nlohmann::json& json) {
    EncounterDesignerDocument document;
    document.id = json.value("id", "");
    for (const auto& region_json : json.value("regions", nlohmann::json::array())) {
        document.regions.push_back(regionFromJson(region_json));
    }
    for (const auto& entry_json : json.value("encounters", nlohmann::json::array())) {
        document.encounters.push_back(designerEntryFromJson(entry_json));
    }
    return document;
}

nlohmann::json encounterDesignerPreviewToJson(const EncounterDesignerPreview& preview) {
    nlohmann::json json;
    json["region_id"] = preview.region_id;
    json["party_level"] = preview.party_level;
    json["seed"] = preview.seed;
    json["encounters"] = nlohmann::json::array();
    for (const auto& entry : preview.encounters) {
        json["encounters"].push_back({{"encounter_id", entry.encounter_id},
                                      {"enemy_id", entry.enemy_id},
                                      {"difficulty", entry.difficulty},
                                      {"rewards", entry.rewards}});
    }
    json["diagnostics"] = nlohmann::json::array();
    for (const auto& diagnostic : preview.diagnostics) {
        json["diagnostics"].push_back({{"code", diagnostic.code}, {"message", diagnostic.message}});
    }
    return json;
}

} // namespace urpg::balance
