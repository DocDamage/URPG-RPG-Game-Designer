#include "engine/core/monster/monster_collection.h"

#include <algorithm>
#include <set>

namespace urpg::monster {
namespace {

MonsterSpecies speciesFromJson(const nlohmann::json& json) {
    return {json.value("id", ""),
            json.value("display_name", ""),
            json.value("actor_id", 0),
            json.value("base_capture_rate", 0),
            json.value("evolves_to", ""),
            json.value("evolution_level", 0)};
}

nlohmann::json speciesToJson(const MonsterSpecies& species) {
    return {{"id", species.id},
            {"display_name", species.display_name},
            {"actor_id", species.actor_id},
            {"base_capture_rate", species.base_capture_rate},
            {"evolves_to", species.evolves_to},
            {"evolution_level", species.evolution_level}};
}

CaptureItemRule captureItemFromJson(const nlohmann::json& json) {
    return {json.value("id", ""),
            json.value("display_name", ""),
            json.value("capture_bonus", 0),
            json.value("enabled", true)};
}

nlohmann::json captureItemToJson(const CaptureItemRule& item) {
    return {{"id", item.id},
            {"display_name", item.display_name},
            {"capture_bonus", item.capture_bonus},
            {"enabled", item.enabled}};
}

CapturedMonster monsterFromJson(const nlohmann::json& json) {
    return {json.value("instance_id", ""), json.value("species_id", ""), json.value("level", 1)};
}

nlohmann::json monsterToJson(const CapturedMonster& monster) {
    return {{"instance_id", monster.instance_id}, {"species_id", monster.species_id}, {"level", monster.level}};
}

} // namespace

MonsterCollectionDocument MonsterCollectionDocument::fromJson(const nlohmann::json& json) {
    MonsterCollectionDocument document;
    document.party_limit = json.value("party_limit", 6U);
    if (json.contains("species") && json["species"].is_array()) {
        for (const auto& entry : json["species"]) {
            document.species.push_back(speciesFromJson(entry));
        }
    }
    if (json.contains("capture_items") && json["capture_items"].is_array()) {
        for (const auto& entry : json["capture_items"]) {
            document.capture_items.push_back(captureItemFromJson(entry));
        }
    }
    if (json.contains("party") && json["party"].is_array()) {
        for (const auto& entry : json["party"]) {
            document.party.push_back(monsterFromJson(entry));
        }
    }
    if (json.contains("storage") && json["storage"].is_array()) {
        for (const auto& entry : json["storage"]) {
            document.storage.push_back(monsterFromJson(entry));
        }
    }
    return document;
}

nlohmann::json MonsterCollectionDocument::toJson() const {
    nlohmann::json species_array = nlohmann::json::array();
    for (const auto& entry : species) {
        species_array.push_back(speciesToJson(entry));
    }
    nlohmann::json capture_item_array = nlohmann::json::array();
    for (const auto& entry : capture_items) {
        capture_item_array.push_back(captureItemToJson(entry));
    }
    nlohmann::json party_array = nlohmann::json::array();
    for (const auto& entry : party) {
        party_array.push_back(monsterToJson(entry));
    }
    nlohmann::json storage_array = nlohmann::json::array();
    for (const auto& entry : storage) {
        storage_array.push_back(monsterToJson(entry));
    }
    return {{"schema_version", "urpg.monster_collection.v1"},
            {"party_limit", party_limit},
            {"species", std::move(species_array)},
            {"capture_items", std::move(capture_item_array)},
            {"party", std::move(party_array)},
            {"storage", std::move(storage_array)}};
}

std::vector<MonsterCollectionDiagnostic> MonsterCollectionDocument::validate() const {
    std::vector<MonsterCollectionDiagnostic> diagnostics;
    std::set<std::string> species_ids;
    for (const auto& entry : species) {
        if (entry.id.empty() || entry.base_capture_rate <= 0) {
            diagnostics.push_back({"invalid_species", "Species requires id and positive capture rate.", entry.id});
        }
        if (!entry.id.empty() && !species_ids.insert(entry.id).second) {
            diagnostics.push_back({"duplicate_species", "Species id is duplicated.", entry.id});
        }
        if (entry.actor_id < 0) {
            diagnostics.push_back({"invalid_actor_id", "Capture actor id cannot be negative.", entry.id});
        }
    }
    std::set<std::string> capture_item_ids;
    for (const auto& item : capture_items) {
        if (item.id.empty()) {
            diagnostics.push_back({"invalid_capture_item", "Capture item requires an id.", item.id});
        } else if (!capture_item_ids.insert(item.id).second) {
            diagnostics.push_back({"duplicate_capture_item", "Capture item id is duplicated.", item.id});
        }
    }
    for (const auto& entry : species) {
        if (!entry.evolves_to.empty() && !species_ids.contains(entry.evolves_to)) {
            diagnostics.push_back({"missing_evolution_species", "Evolution target species does not exist.", entry.id});
        }
    }
    for (const auto& monster : party) {
        if (!species_ids.contains(monster.species_id)) {
            diagnostics.push_back({"missing_party_species", "Party monster species does not exist.",
                                   monster.instance_id});
        }
    }
    for (const auto& monster : storage) {
        if (!species_ids.contains(monster.species_id)) {
            diagnostics.push_back({"missing_storage_species", "Storage monster species does not exist.",
                                   monster.instance_id});
        }
    }
    if (party.size() > party_limit) {
        diagnostics.push_back({"party_limit_exceeded", "Party exceeds configured party limit.", ""});
    }
    return diagnostics;
}

CapturePreview MonsterCollectionDocument::previewCapture(const CaptureAttempt& attempt) const {
    CapturePreview preview;
    preview.diagnostics = validate();
    if (!preview.diagnostics.empty()) {
        return preview;
    }
    const auto it = std::find_if(species.begin(), species.end(), [&](const MonsterSpecies& entry) {
        return entry.id == attempt.species_id;
    });
    if (it == species.end()) {
        preview.diagnostics.push_back({"missing_capture_species", "Capture species does not exist.",
                                       attempt.species_id});
        return preview;
    }
    const CaptureItemRule* item_rule = nullptr;
    if (!attempt.item_id.empty()) {
        const auto item_it = std::find_if(capture_items.begin(), capture_items.end(), [&](const CaptureItemRule& entry) {
            return entry.id == attempt.item_id;
        });
        if (item_it == capture_items.end()) {
            preview.diagnostics.push_back({"missing_capture_item", "Capture item does not exist.", attempt.item_id});
            return preview;
        }
        if (!item_it->enabled) {
            preview.diagnostics.push_back({"disabled_capture_item", "Capture item is disabled.", attempt.item_id});
            return preview;
        }
        item_rule = &(*item_it);
        preview.requires_capture_item = true;
    }
    const int32_t hp_bonus = std::clamp(100 - attempt.target_hp_percent, 0, 100) / 2;
    const int32_t item_bonus = item_rule ? item_rule->capture_bonus : 0;
    preview.chance = std::clamp(it->base_capture_rate + hp_bonus + attempt.ball_bonus + item_bonus, 1, 95);
    preview.roll = static_cast<int32_t>((attempt.seed * 1103515245U + 12345U) % 100U) + 1;
    preview.success = preview.roll <= preview.chance;
    preview.actor_id = it->actor_id;
    preview.suppress_exp_gold_drops = preview.success;
    return preview;
}

bool MonsterCollectionDocument::capture(const CaptureAttempt& attempt, const std::string& instance_id) {
    const auto preview = previewCapture(attempt);
    if (!preview.success || !preview.diagnostics.empty() || instance_id.empty()) {
        return false;
    }
    CapturedMonster monster{instance_id, attempt.species_id, 1};
    if (party.size() < party_limit) {
        party.push_back(std::move(monster));
    } else {
        storage.push_back(std::move(monster));
    }
    return true;
}

CaptureResolution MonsterCollectionDocument::resolveItemCapture(const CaptureAttempt& attempt,
                                                                const std::string& instance_id) {
    CaptureResolution resolution;
    const auto preview = previewCapture(attempt);
    resolution.diagnostics = preview.diagnostics;
    if (!preview.success || !preview.diagnostics.empty() || instance_id.empty()) {
        if (instance_id.empty()) {
            resolution.diagnostics.push_back({"missing_instance_id", "Captured monster requires an instance id.", ""});
        }
        return resolution;
    }
    resolution.captured = capture(attempt, instance_id);
    resolution.remove_enemy_from_battle = resolution.captured;
    resolution.suppress_exp_gold_drops = resolution.captured;
    resolution.actor_id_added = resolution.captured ? preview.actor_id : 0;
    resolution.instance_id = resolution.captured ? instance_id : "";
    return resolution;
}

bool MonsterCollectionDocument::moveToParty(const std::string& instance_id) {
    if (party.size() >= party_limit) {
        return false;
    }
    const auto it = std::find_if(storage.begin(), storage.end(), [&](const CapturedMonster& monster) {
        return monster.instance_id == instance_id;
    });
    if (it == storage.end()) {
        return false;
    }
    party.push_back(*it);
    storage.erase(it);
    return true;
}

bool MonsterCollectionDocument::moveToStorage(const std::string& instance_id) {
    const auto it = std::find_if(party.begin(), party.end(), [&](const CapturedMonster& monster) {
        return monster.instance_id == instance_id;
    });
    if (it == party.end()) {
        return false;
    }
    storage.push_back(*it);
    party.erase(it);
    return true;
}

bool MonsterCollectionDocument::evolve(const std::string& instance_id) {
    auto evolve_monster = [&](CapturedMonster& monster) {
        const auto species_it = std::find_if(species.begin(), species.end(), [&](const MonsterSpecies& entry) {
            return entry.id == monster.species_id;
        });
        if (species_it == species.end() || species_it->evolves_to.empty() ||
            monster.level < species_it->evolution_level) {
            return false;
        }
        monster.species_id = species_it->evolves_to;
        return true;
    };
    for (auto& monster : party) {
        if (monster.instance_id == instance_id) {
            return evolve_monster(monster);
        }
    }
    for (auto& monster : storage) {
        if (monster.instance_id == instance_id) {
            return evolve_monster(monster);
        }
    }
    return false;
}

nlohmann::json monsterCapturePreviewToJson(const CapturePreview& preview) {
    nlohmann::json diagnostics = nlohmann::json::array();
    for (const auto& diagnostic : preview.diagnostics) {
        diagnostics.push_back({{"code", diagnostic.code}, {"message", diagnostic.message}, {"id", diagnostic.id}});
    }
    return {{"success", preview.success},
            {"chance", preview.chance},
            {"roll", preview.roll},
            {"actor_id", preview.actor_id},
            {"requires_capture_item", preview.requires_capture_item},
            {"suppress_exp_gold_drops", preview.suppress_exp_gold_drops},
            {"diagnostics", std::move(diagnostics)}};
}

nlohmann::json monsterCaptureResolutionToJson(const CaptureResolution& resolution) {
    nlohmann::json diagnostics = nlohmann::json::array();
    for (const auto& diagnostic : resolution.diagnostics) {
        diagnostics.push_back({{"code", diagnostic.code}, {"message", diagnostic.message}, {"id", diagnostic.id}});
    }
    return {{"captured", resolution.captured},
            {"remove_enemy_from_battle", resolution.remove_enemy_from_battle},
            {"suppress_exp_gold_drops", resolution.suppress_exp_gold_drops},
            {"actor_id_added", resolution.actor_id_added},
            {"instance_id", resolution.instance_id},
            {"diagnostics", std::move(diagnostics)}};
}

} // namespace urpg::monster
