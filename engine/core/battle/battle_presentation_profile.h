#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <set>
#include <string>
#include <vector>

namespace urpg::battle {

enum class BattleAuthoringSeverity : uint8_t {
    Info = 0,
    Warning = 1,
    Error = 2,
};

struct BattleAuthoringDiagnostic {
    BattleAuthoringSeverity severity = BattleAuthoringSeverity::Info;
    std::string code;
    std::string message;
    std::string target;
};

struct BattlePresentationCue {
    std::string id;
    std::string type;
    int32_t frame = 0;
    std::string payload;
};

struct BattleHudElement {
    std::string id;
    std::string type;
    int32_t x = 0;
    int32_t y = 0;
    bool visible = true;
};

struct BattlePresentationProfile {
    std::string id;
    std::string battleback1;
    std::string battleback2;
    std::vector<BattleHudElement> hud_elements;
    std::vector<BattlePresentationCue> cue_timeline;
};

struct BattlePresentationValidationResult {
    std::vector<BattleAuthoringDiagnostic> diagnostics;
    std::vector<BattlePresentationCue> replay_cues;
};

BattlePresentationProfile BattlePresentationProfileFromJson(const nlohmann::json& json);
BattlePresentationValidationResult ValidateBattlePresentationProfile(
    const BattlePresentationProfile& profile,
    const std::set<std::string>& available_assets
);
nlohmann::json BattlePresentationProfileToJson(const BattlePresentationProfile& profile);

const char* ToString(BattleAuthoringSeverity severity);

} // namespace urpg::battle
