#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace urpg::monster {

struct MonsterSpecies {
    std::string id;
    std::string display_name;
    int32_t actor_id = 0;
    int32_t base_capture_rate = 0;
    std::string evolves_to;
    int32_t evolution_level = 0;
};

struct CaptureItemRule {
    std::string id;
    std::string display_name;
    int32_t capture_bonus = 0;
    bool enabled = true;
};

struct CapturedMonster {
    std::string instance_id;
    std::string species_id;
    int32_t level = 1;
};

struct MonsterCollectionDiagnostic {
    std::string code;
    std::string message;
    std::string id;
};

struct CaptureAttempt {
    std::string species_id;
    std::string item_id;
    int32_t target_hp_percent = 100;
    int32_t ball_bonus = 0;
    uint32_t seed = 0;
};

struct CapturePreview {
    bool success = false;
    int32_t chance = 0;
    int32_t roll = 0;
    int32_t actor_id = 0;
    bool requires_capture_item = false;
    bool suppress_exp_gold_drops = false;
    std::vector<MonsterCollectionDiagnostic> diagnostics;
};

struct CaptureResolution {
    bool captured = false;
    bool remove_enemy_from_battle = false;
    bool suppress_exp_gold_drops = false;
    int32_t actor_id_added = 0;
    std::string instance_id;
    std::vector<MonsterCollectionDiagnostic> diagnostics;
};

class MonsterCollectionDocument {
public:
    std::vector<MonsterSpecies> species;
    std::vector<CaptureItemRule> capture_items;
    std::vector<CapturedMonster> party;
    std::vector<CapturedMonster> storage;
    size_t party_limit = 6;

    static MonsterCollectionDocument fromJson(const nlohmann::json& json);
    nlohmann::json toJson() const;
    std::vector<MonsterCollectionDiagnostic> validate() const;
    CapturePreview previewCapture(const CaptureAttempt& attempt) const;
    bool capture(const CaptureAttempt& attempt, const std::string& instance_id);
    CaptureResolution resolveItemCapture(const CaptureAttempt& attempt, const std::string& instance_id);
    bool moveToParty(const std::string& instance_id);
    bool moveToStorage(const std::string& instance_id);
    bool evolve(const std::string& instance_id);
};

nlohmann::json monsterCapturePreviewToJson(const CapturePreview& preview);
nlohmann::json monsterCaptureResolutionToJson(const CaptureResolution& resolution);

} // namespace urpg::monster
