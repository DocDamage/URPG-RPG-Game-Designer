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
    return !path.empty() && available_assets.contains(path);
}

bool isHexColor(const std::string& value) {
    if (value.size() != 7 || value[0] != '#') {
        return false;
    }
    return std::all_of(value.begin() + 1, value.end(), [](unsigned char ch) {
        return std::isxdigit(ch) != 0;
    });
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

    if (const auto media_it = json.find("media_layers");
        media_it != json.end() && media_it->is_array()) {
        for (const auto& row : *media_it) {
            if (!row.is_object()) {
                continue;
            }
            profile.media_layers.push_back({
                row.value("id", ""),
                row.value("role", "background"),
                row.value("asset", ""),
                row.value("loop", true),
                row.value("playback_rate", 1.0f),
                row.value("opacity", 1.0f),
                row.value("region_id", 0),
                row.value("fade_in_frames", 0),
            });
        }
    }

    if (const auto light_it = json.find("light_cues");
        light_it != json.end() && light_it->is_array()) {
        for (const auto& row : *light_it) {
            if (!row.is_object()) {
                continue;
            }
            profile.light_cues.push_back({
                row.value("id", ""),
                row.value("target", "all"),
                row.value("radius", 150),
                row.value("color", "#ffffff"),
                row.value("opacity", 0.5f),
                row.value("pulsate", false),
                row.value("flicker", false),
                row.value("hide_on_death", true),
            });
        }
    }

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

    if (profile.battleback1.empty()) {
        addDiagnostic(result.diagnostics, BattleAuthoringSeverity::Error, "required_battleback",
                      "Battle presentation profile must define battleback1 for release.", "battleback1");
    }
    if (!assetAvailable(profile.battleback1, available_assets)) {
        addDiagnostic(result.diagnostics, BattleAuthoringSeverity::Error, "missing_battleback",
                      "Battleback asset is not available: " + profile.battleback1, profile.battleback1);
    }
    if (!profile.battleback2.empty() && !assetAvailable(profile.battleback2, available_assets)) {
        addDiagnostic(result.diagnostics, BattleAuthoringSeverity::Error, "missing_battleback",
                      "Battleback asset is not available: " + profile.battleback2, profile.battleback2);
    }
    const std::set<std::string> media_roles = {"background", "foreground", "overlay"};
    for (const auto& layer : profile.media_layers) {
        if (layer.id.empty()) {
            addDiagnostic(result.diagnostics, BattleAuthoringSeverity::Error, "missing_media_layer_id",
                          "Battle media layer requires an id.", layer.asset);
        }
        if (!media_roles.contains(layer.role)) {
            addDiagnostic(result.diagnostics, BattleAuthoringSeverity::Error, "invalid_media_layer_role",
                          "Battle media layer role must be background, foreground, or overlay.", layer.id);
        }
        if (!assetAvailable(layer.asset, available_assets)) {
            addDiagnostic(result.diagnostics, BattleAuthoringSeverity::Error, "missing_media_asset",
                          "Battle media asset is not available: " + layer.asset, layer.asset);
        }
        if (layer.playback_rate <= 0.0f) {
            addDiagnostic(result.diagnostics, BattleAuthoringSeverity::Error, "invalid_media_playback_rate",
                          "Battle media playback rate must be positive.", layer.id);
        }
        if (layer.opacity < 0.0f || layer.opacity > 1.0f) {
            addDiagnostic(result.diagnostics, BattleAuthoringSeverity::Error, "invalid_media_opacity",
                          "Battle media opacity must be between 0 and 1.", layer.id);
        }
        if (layer.region_id < 0) {
            addDiagnostic(result.diagnostics, BattleAuthoringSeverity::Error, "invalid_media_region",
                          "Battle media region id cannot be negative.", layer.id);
        }
        if (layer.fade_in_frames < 0) {
            addDiagnostic(result.diagnostics, BattleAuthoringSeverity::Error, "invalid_media_fade",
                          "Battle media fade-in frames cannot be negative.", layer.id);
        }
    }
    for (const auto& light : profile.light_cues) {
        if (light.id.empty()) {
            addDiagnostic(result.diagnostics, BattleAuthoringSeverity::Error, "missing_light_id",
                          "Battle light cue requires an id.", light.target);
        }
        if (light.radius <= 0) {
            addDiagnostic(result.diagnostics, BattleAuthoringSeverity::Error, "invalid_light_radius",
                          "Battle light radius must be positive.", light.id);
        }
        if (!isHexColor(light.color)) {
            addDiagnostic(result.diagnostics, BattleAuthoringSeverity::Error, "invalid_light_color",
                          "Battle light color must be #rrggbb.", light.id);
        }
        if (light.opacity < 0.0f || light.opacity > 1.0f) {
            addDiagnostic(result.diagnostics, BattleAuthoringSeverity::Error, "invalid_light_opacity",
                          "Battle light opacity must be between 0 and 1.", light.id);
        }
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
    json["media_layers"] = nlohmann::json::array();
    for (const auto& layer : profile.media_layers) {
        json["media_layers"].push_back({
            {"id", layer.id},
            {"role", layer.role},
            {"asset", layer.asset},
            {"loop", layer.loop},
            {"playback_rate", layer.playback_rate},
            {"opacity", layer.opacity},
            {"region_id", layer.region_id},
            {"fade_in_frames", layer.fade_in_frames},
        });
    }
    json["light_cues"] = nlohmann::json::array();
    for (const auto& light : profile.light_cues) {
        json["light_cues"].push_back({
            {"id", light.id},
            {"target", light.target},
            {"radius", light.radius},
            {"color", light.color},
            {"opacity", light.opacity},
            {"pulsate", light.pulsate},
            {"flicker", light.flicker},
            {"hide_on_death", light.hide_on_death},
        });
    }
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
