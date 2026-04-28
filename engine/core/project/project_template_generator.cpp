#include "engine/core/project/project_template_generator.h"

#include "engine/core/project/template_runtime_profile.h"

#include <algorithm>
#include <array>
#include <cctype>

namespace urpg::project {

namespace {

struct TemplateSpec {
    std::string id;
    std::string display_name;
    std::vector<std::string> tags;
    std::vector<std::string> maps;
    std::string battle_mode;
};

const std::array<std::string, 8> kRequiredSubsystems = {
    "maps",
    "menu",
    "message",
    "battle",
    "save",
    "localization",
    "input",
    "export_profile",
};

TemplateSpec specFromProfile(const TemplateRuntimeProfile& profile) {
    TemplateSpec spec;
    spec.id = profile.id;
    spec.display_name = profile.displayName;
    spec.tags = profile.tags;
    if (profile.id == "visual_novel") {
        spec.maps = {"scene_intro", "scene_choice"};
        spec.battle_mode = "none";
    } else if (profile.id == "tactics_rpg") {
        spec.maps = {"scenario_training_grid", "scenario_fortress"};
        spec.battle_mode = "tactics";
    } else if (profile.id == "arpg") {
        spec.maps = {"map_action_intro", "map_combat_field"};
        spec.battle_mode = "action";
    } else if (profile.id == "monster_collector_rpg") {
        spec.maps = {"map_route_01", "map_capture_grove", "map_ranch"};
        spec.battle_mode = "collection";
    } else if (profile.id == "cozy_life_rpg") {
        spec.maps = {"map_home", "map_town_square", "map_forest"};
        spec.battle_mode = "none";
    } else if (profile.id == "metroidvania_lite") {
        spec.maps = {"room_start", "room_old_well", "room_clock_tower"};
        spec.battle_mode = "action";
    } else if (profile.id == "2_5d_rpg") {
        spec.maps = {"spatial_intro", "spatial_ruins"};
        spec.battle_mode = "spatial";
    } else if (profile.id == "roguelite_dungeon") {
        spec.maps = {"hub_camp", "floor_01_seeded", "boss_gate"};
        spec.battle_mode = "dungeon_run";
    } else if (profile.id == "survival_horror_rpg") {
        spec.maps = {"safe_room", "west_wing", "archive_puzzle"};
        spec.battle_mode = "scarce_resource";
    } else if (profile.id == "farming_adventure_rpg") {
        spec.maps = {"farmstead", "town_market", "spirit_grove"};
        spec.battle_mode = "adventure";
    } else if (profile.id == "card_battler_rpg") {
        spec.maps = {"deck_room", "duel_arena", "reward_draft"};
        spec.battle_mode = "card_battle";
    } else if (profile.id == "platformer_rpg") {
        spec.maps = {"cliffside_tutorial", "bridge_checkpoint", "tower_gate"};
        spec.battle_mode = "side_action";
    } else if (profile.id == "gacha_hero_rpg") {
        spec.maps = {"summon_hall", "hero_roster", "training_arena"};
        spec.battle_mode = "party_battle";
    } else if (profile.id == "mystery_detective_rpg") {
        spec.maps = {"case_office", "clock_tower_scene", "library_interrogation"};
        spec.battle_mode = "none";
    } else if (profile.id == "world_exploration_rpg") {
        spec.maps = {"world_map_capital", "forest_route", "coast_landmark"};
        spec.battle_mode = "exploration";
    } else if (profile.id == "turn_based_rpg") {
        spec.maps = {"map_intro", "map_arena"};
        spec.battle_mode = "turn_based";
    } else {
        spec.maps = {"map_intro", "map_town", "map_field"};
        spec.battle_mode = "turn_based";
    }
    return spec;
}

bool isValidProjectId(const std::string& id) {
    if (id.empty()) {
        return false;
    }
    return std::all_of(id.begin(), id.end(), [](unsigned char c) {
        return std::isalnum(c) || c == '_' || c == '-';
    });
}

nlohmann::json makeMaps(const TemplateSpec& spec) {
    nlohmann::json maps = nlohmann::json::array();
    for (std::size_t i = 0; i < spec.maps.size(); ++i) {
        maps.push_back({
            {"id", spec.maps[i]},
            {"display_name", i == 0 ? "Opening" : "Onboarding " + std::to_string(i)},
            {"tileset", "starter_world"},
            {"spawn", {{"x", 4}, {"y", 6}}},
        });
    }
    return maps;
}

nlohmann::json makeSubsystems(const TemplateSpec& spec, const TemplateRuntimeProfile& profile) {
    nlohmann::json subsystems = {
        {"maps", makeMaps(spec)},
        {"menu", {{"style", spec.id == "visual_novel" ? "minimal" : "classic"}, {"commands", {"items", "skills", "save"}}}},
        {"message", {{"default_speaker", "Guide"}, {"sample_line", "Welcome to your first playable slice."}}},
        {"battle", {{"mode", spec.battle_mode}, {"encounter_id", spec.battle_mode == "none" ? "" : "battle_training_01"}}},
        {"save", {{"slot_count", 3}, {"autosave", true}}},
        {"localization", {{"default_locale", "en-US"}, {"locales", {"en-US"}}}},
        {"input", {{"profile", "keyboard_gamepad"}, {"confirm", "Enter"}, {"cancel", "Escape"}}},
        {"export_profile", {{"target", "Windows_x64"}, {"integrity", "strict"}, {"output_name", spec.id + "_starter"}}},
        {"template_runtime", templateRuntimeProfileToJson(profile)},
    };
    for (const auto& [key, value] : profile.systems.items()) {
        subsystems[key] = value;
    }
    return subsystems;
}

std::vector<std::string> missingSubsystems(const nlohmann::json& project) {
    std::vector<std::string> missing;
    if (!project.contains("subsystems") || !project["subsystems"].is_object()) {
        return std::vector<std::string>(kRequiredSubsystems.begin(), kRequiredSubsystems.end());
    }
    for (const auto& subsystem : kRequiredSubsystems) {
        if (!project["subsystems"].contains(subsystem)) {
            missing.push_back(subsystem);
        }
    }
    return missing;
}

} // namespace

ProjectTemplateResult ProjectTemplateGenerator::generate(const ProjectTemplateRequest& request) {
    ProjectTemplateResult result;
    const auto profile = findTemplateRuntimeProfile(request.template_id);
    if (!profile.has_value()) {
        result.errors.push_back("unknown_template:" + request.template_id);
        return result;
    }
    const TemplateSpec spec = specFromProfile(*profile);
    for (const auto& issue : validateTemplateRuntimeProfile(*profile)) {
        result.errors.push_back("invalid_template_profile:" + issue);
    }
    if (!result.errors.empty()) {
        return result;
    }
    if (!isValidProjectId(request.project_id)) {
        result.errors.push_back("invalid_project_id");
        return result;
    }
    if (request.project_name.empty()) {
        result.errors.push_back("missing_project_name");
        return result;
    }
    if (issued_project_ids_.contains(request.project_id)) {
        result.errors.push_back("duplicate_project_id:" + request.project_id);
        return result;
    }

    result.project = {
        {"schema_version", "urpg.project.v1"},
        {"project_id", request.project_id},
        {"project_name", request.project_name},
        {"template_id", spec.id},
        {"template_display_name", spec.display_name},
        {"tags", spec.tags},
        {"loops", profile->loops},
        {"template_bars", profile->bars},
        {"subsystems", makeSubsystems(spec, *profile)},
    };
    result.audit_report = {
        {"project_id", request.project_id},
        {"template_id", spec.id},
        {"status", "passed"},
        {"checked_subsystems", kRequiredSubsystems},
        {"checked_template_loops", profile->loops},
        {"issues", nlohmann::json::array()},
    };
    issued_project_ids_.insert(request.project_id);
    result.success = true;
    return result;
}

std::vector<std::string> ProjectTemplateGenerator::validateProjectDocument(const nlohmann::json& project) const {
    std::vector<std::string> errors;
    if (!project.is_object()) {
        return {"project_document_not_object"};
    }
    if (project.value("schema_version", "") != "urpg.project.v1") {
        errors.push_back("invalid_project_schema_version");
    }
    if (!isValidProjectId(project.value("project_id", ""))) {
        errors.push_back("invalid_project_id");
    }
    if (!findTemplateRuntimeProfile(project.value("template_id", "")).has_value()) {
        errors.push_back("unknown_template:" + project.value("template_id", ""));
    }
    if (!project.contains("loops") || !project["loops"].is_array() || project["loops"].empty()) {
        errors.push_back("missing_template_loops");
    }
    if (!project.contains("template_bars") || !project["template_bars"].is_object()) {
        errors.push_back("missing_template_bars");
    }
    for (const auto& subsystem : missingSubsystems(project)) {
        errors.push_back("missing_subsystem:" + subsystem);
    }
    return errors;
}

void ProjectTemplateGenerator::resetIssuedProjectIds() {
    issued_project_ids_.clear();
}

} // namespace urpg::project
