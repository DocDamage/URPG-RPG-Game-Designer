#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace urpg::monster {

struct MonsterSpecies {
    std::string id;
    std::string display_name;
    int32_t base_capture_rate = 0;
    std::string evolves_to;
    int32_t evolution_level = 0;
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
    int32_t target_hp_percent = 100;
    int32_t ball_bonus = 0;
    uint32_t seed = 0;
};

struct CapturePreview {
    bool success = false;
    int32_t chance = 0;
    int32_t roll = 0;
    std::vector<MonsterCollectionDiagnostic> diagnostics;
};

class MonsterCollectionDocument {
public:
    std::vector<MonsterSpecies> species;
    std::vector<CapturedMonster> party;
    std::vector<CapturedMonster> storage;
    size_t party_limit = 6;

    static MonsterCollectionDocument fromJson(const nlohmann::json& json);
    nlohmann::json toJson() const;
    std::vector<MonsterCollectionDiagnostic> validate() const;
    CapturePreview previewCapture(const CaptureAttempt& attempt) const;
    bool capture(const CaptureAttempt& attempt, const std::string& instance_id);
    bool moveToParty(const std::string& instance_id);
    bool moveToStorage(const std::string& instance_id);
    bool evolve(const std::string& instance_id);
};

nlohmann::json monsterCapturePreviewToJson(const CapturePreview& preview);

} // namespace urpg::monster
