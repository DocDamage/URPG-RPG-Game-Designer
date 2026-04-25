#pragma once

#include "engine/core/battle/battle_presentation_profile.h"

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace urpg::battle {

struct BossPhase {
    std::string id;
    int32_t hp_below_percent = 100;
    std::vector<std::string> summons;
    bool enrage = false;
    std::string dialogue_bark;
    std::string reward_item;
    std::string music_transition;
};

struct BossProfile {
    std::string id;
    std::vector<BossPhase> phases;
};

struct BossProfileValidationResult {
    std::vector<BattleAuthoringDiagnostic> diagnostics;
};

BossProfile BossProfileFromJson(const nlohmann::json& json);
BossProfileValidationResult ValidateBossProfile(const BossProfile& profile);
nlohmann::json BossProfileToJson(const BossProfile& profile);

} // namespace urpg::battle
