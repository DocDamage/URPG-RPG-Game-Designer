#include "engine/core/battle/battle_presentation_profile.h"

#include <algorithm>
#include <cctype>
#include <set>

namespace urpg::battle {

namespace {

void addDiagnostic(std::vector<BattleAuthoringDiagnostic>& diagnostics,
                   BattleAuthoringSeverity severity,
                   std::string code,
                   std::string message,
                   std::string target) {
    diagnostics.push_back({severity, std::move(code), std::move(message), std::move(target)});
}

bool assetAvailable(const std::string& path, const std::set<std::string>& available_assets) {
    return path.empty() || available_assets.contains(path);
}

} // namespace

BattlePresentationProfile BattlePresentationProfileFromJson(const nlohmann::json& json) {
    BattlePresentationProfile profile;
    if (!json.is_object()) {
        return profile;
    }
    profile.id = json.value("id", "");
    profile.battleback1 = json.value("battleback1", "");
    profile.battleback2 = json.value("battleback2", "");

    if (const auto hud_it = json.find("hud");
        hud_it != json.end() && hud_it->is_array()) {
        for (const auto& row : *hud_it) {
            if (!row.is_object()) {
                continue;
            }
            profile.hud_elements.push_back({
                row.value("id", ""),
                row.value("type", ""),
                row.value("x", 0),
                row.value("y", 0),
                row.value("visible", true),
            });
        }
    }

    if (const auto cue_it = json.find("cue_timeline");
        cue_it != json.end() && cue_it->is_array()) {
        for (const auto& row : *cue_it) {
            if (!row.is_object()) {
                continue;
            }
            profile.cue_timeline.push_back({
                row.value("id", ""),
                row.value("type", ""),
                row.value("frame", 0),
                row.value("payload", ""),
            });
        }
    }
    return profile;
}

BattlePresentationValidationResult ValidateBattlePresentationProfile(
    const BattlePresentationProfile& profile,
    const std::set<std::string>& available_assets
) {
    BattlePresentationValidationResult result;

    if (!assetAvailable(profile.battleback1, available_assets)) {
        addDiagnostic(result.diagnostics, BattleAuthoringSeverity::Error, "missing_battleback",
                      "Battleback asset is not available: " + profile.battleback1, profile.battleback1);
    }
    if (!assetAvailable(profile.battleback2, available_assets)) {
        addDiagnostic(result.diagnostics, BattleAuthoringSeverity::Error, "missing_battleback",
                      "Battleback asset is not available: " + profile.battleback2, profile.battleback2);
    }

    std::set<std::string> lower_assets;
    for (const auto& asset : available_assets) {
        std::string lowered = asset;
        std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        lower_assets.insert(lowered);
    }
    for (const auto& battleback : {profile.battleback1, profile.battleback2}) {
        std::string lowered = battleback;
        std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        if (!battleback.empty() && !available_assets.contains(battleback) && lower_assets.contains(lowered)) {
            addDiagnostic(result.diagnostics, BattleAuthoringSeverity::Warning, "battleback_case_mismatch",
                          "Battleback differs by case from an available asset: " + battleback, battleback);
        }
    }

    const std::set<std::string> expected_hud_types = {
        "gauge", "state_icon", "turn_order", "damage_popup", "guard_marker",
    };
    for (const auto& expected : expected_hud_types) {
        const bool found = std::any_of(profile.hud_elements.begin(), profile.hud_elements.end(), [&](const auto& element) {
            return element.type == expected;
        });
        if (!found) {
            addDiagnostic(result.diagnostics, BattleAuthoringSeverity::Warning, "missing_hud_element",
                          "HUD layout is missing expected element type: " + expected, expected);
        }
    }

    result.replay_cues = profile.cue_timeline;
    std::stable_sort(result.replay_cues.begin(), result.replay_cues.end(), [](const auto& a, const auto& b) {
        if (a.frame != b.frame) {
            return a.frame < b.frame;
        }
        return a.id < b.id;
    });

    int32_t previous_frame = -1;
    for (const auto& cue : result.replay_cues) {
        if (cue.frame < 0) {
            addDiagnostic(result.diagnostics, BattleAuthoringSeverity::Error, "negative_cue_frame",
                          "Cue timeline frame cannot be negative.", cue.id);
        }
        if (cue.frame < previous_frame) {
            addDiagnostic(result.diagnostics, BattleAuthoringSeverity::Error, "cue_order_regression",
                          "Cue timeline replay order regressed.", cue.id);
        }
        previous_frame = std::max(previous_frame, cue.frame);
    }

    std::stable_sort(result.diagnostics.begin(), result.diagnostics.end(), [](const auto& a, const auto& b) {
        if (a.severity != b.severity) {
            return static_cast<int>(a.severity) > static_cast<int>(b.severity);
        }
        if (a.code != b.code) {
            return a.code < b.code;
        }
        return a.target < b.target;
    });
    return result;
}

nlohmann::json BattlePresentationProfileToJson(const BattlePresentationProfile& profile) {
    nlohmann::json json;
    json["id"] = profile.id;
    json["battleback1"] = profile.battleback1;
    json["battleback2"] = profile.battleback2;
    json["hud"] = nlohmann::json::array();
    for (const auto& element : profile.hud_elements) {
        json["hud"].push_back({
            {"id", element.id},
            {"type", element.type},
            {"x", element.x},
            {"y", element.y},
            {"visible", element.visible},
        });
    }
    json["cue_timeline"] = nlohmann::json::array();
    for (const auto& cue : profile.cue_timeline) {
        json["cue_timeline"].push_back({
            {"id", cue.id},
            {"type", cue.type},
            {"frame", cue.frame},
            {"payload", cue.payload},
        });
    }
    return json;
}

const char* ToString(BattleAuthoringSeverity severity) {
    switch (severity) {
    case BattleAuthoringSeverity::Info:
        return "info";
    case BattleAuthoringSeverity::Warning:
        return "warning";
    case BattleAuthoringSeverity::Error:
        return "error";
    }
    return "unknown";
}

} // namespace urpg::battle
