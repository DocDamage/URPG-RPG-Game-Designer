#include "engine/core/battle/boss_profile.h"

#include <algorithm>
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

} // namespace

BossProfile BossProfileFromJson(const nlohmann::json& json) {
    BossProfile profile;
    if (!json.is_object()) {
        return profile;
    }
    profile.id = json.value("id", "");
    if (const auto phases_it = json.find("phases");
        phases_it != json.end() && phases_it->is_array()) {
        for (const auto& row : *phases_it) {
            if (!row.is_object()) {
                continue;
            }
            BossPhase phase;
            phase.id = row.value("id", "");
            phase.hp_below_percent = row.value("hp_below_percent", 100);
            phase.enrage = row.value("enrage", false);
            phase.dialogue_bark = row.value("dialogue_bark", "");
            phase.reward_item = row.value("reward_item", "");
            phase.music_transition = row.value("music_transition", "");
            if (const auto summons_it = row.find("summons");
                summons_it != row.end() && summons_it->is_array()) {
                for (const auto& summon : *summons_it) {
                    if (summon.is_string()) {
                        phase.summons.push_back(summon.get<std::string>());
                    }
                }
            }
            profile.phases.push_back(std::move(phase));
        }
    }
    return profile;
}

BossProfileValidationResult ValidateBossProfile(const BossProfile& profile) {
    BossProfileValidationResult result;
    int32_t previous_threshold = 101;
    std::set<std::string> ids;

    for (const auto& phase : profile.phases) {
        if (phase.id.empty()) {
            addDiagnostic(result.diagnostics, BattleAuthoringSeverity::Error, "missing_phase_id",
                          "Boss phase is missing an id.", profile.id);
        } else if (!ids.insert(phase.id).second) {
            addDiagnostic(result.diagnostics, BattleAuthoringSeverity::Error, "duplicate_phase_id",
                          "Boss phase id is duplicated: " + phase.id, phase.id);
        }
        if (phase.hp_below_percent <= 0 || phase.hp_below_percent > 100) {
            addDiagnostic(result.diagnostics, BattleAuthoringSeverity::Error, "invalid_phase_threshold",
                          "Boss phase threshold must be 1..100.", phase.id);
        }
        if (phase.hp_below_percent >= previous_threshold) {
            addDiagnostic(result.diagnostics, BattleAuthoringSeverity::Error, "phase_threshold_order",
                          "Boss phase thresholds must descend without overlap.", phase.id);
        }
        previous_threshold = phase.hp_below_percent;
    }

    std::stable_sort(result.diagnostics.begin(), result.diagnostics.end(), [](const auto& a, const auto& b) {
        if (a.severity != b.severity) {
            return static_cast<int>(a.severity) > static_cast<int>(b.severity);
        }
        return a.code < b.code;
    });
    return result;
}

nlohmann::json BossProfileToJson(const BossProfile& profile) {
    nlohmann::json json;
    json["id"] = profile.id;
    json["phases"] = nlohmann::json::array();
    for (const auto& phase : profile.phases) {
        json["phases"].push_back({
            {"id", phase.id},
            {"hp_below_percent", phase.hp_below_percent},
            {"summons", phase.summons},
            {"enrage", phase.enrage},
            {"dialogue_bark", phase.dialogue_bark},
            {"reward_item", phase.reward_item},
            {"music_transition", phase.music_transition},
        });
    }
    return json;
}

} // namespace urpg::battle
