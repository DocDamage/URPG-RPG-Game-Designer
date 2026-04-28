#include "engine/core/maker/maker_wysiwyg_feature.h"

#include <algorithm>

namespace urpg::maker {

namespace {

MakerFeatureAction actionFromJson(const nlohmann::json& json) {
    MakerFeatureAction action;
    action.id = json.value("id", "");
    action.label = json.value("label", "");
    action.trigger = json.value("trigger", "");
    action.operation = json.value("operation", "");
    action.target = json.value("target", "");
    action.required_flags = json.value("required_flags", std::vector<std::string>{});
    action.parameters = json.value("parameters", std::map<std::string, std::string>{});
    return action;
}

nlohmann::json actionToJson(const MakerFeatureAction& action) {
    return {{"id", action.id},
            {"label", action.label},
            {"trigger", action.trigger},
            {"operation", action.operation},
            {"target", action.target},
            {"required_flags", action.required_flags},
            {"parameters", action.parameters}};
}

bool hasAllFlags(const MakerFeatureRuntimeState& state, const std::vector<std::string>& flags) {
    return std::all_of(flags.begin(), flags.end(), [&](const auto& flag) { return state.flags.contains(flag); });
}

void applyAction(MakerFeatureRuntimeState& state, const MakerFeatureAction& action) {
    state.emitted_operations.push_back(action.operation + ":" + action.target);
    for (const auto& [key, value] : action.parameters) {
        state.variables[key] = value;
    }
}

nlohmann::json stateToJson(const MakerFeatureRuntimeState& state) {
    return {{"flags", state.flags}, {"variables", state.variables}, {"emitted_operations", state.emitted_operations}};
}

} // namespace

std::vector<MakerFeatureDiagnostic> MakerWysiwygFeatureDocument::validate() const {
    std::vector<MakerFeatureDiagnostic> diagnostics;
    if (schema_version != "urpg.maker_wysiwyg.v1") {
        diagnostics.push_back({"invalid_schema_version", "Maker WYSIWYG feature has an unsupported schema version.", id});
    }
    if (id.empty()) {
        diagnostics.push_back({"missing_document_id", "Maker WYSIWYG feature is missing an id.", id});
    }
    if (feature_type.empty()) {
        diagnostics.push_back({"missing_feature_type", "Maker WYSIWYG feature is missing a feature type.", id});
    }
    if (display_name.empty()) {
        diagnostics.push_back({"missing_display_name", "Maker WYSIWYG feature is missing a display name.", id});
    }
    if (visual_layers.empty()) {
        diagnostics.push_back({"missing_visual_layers", "Maker WYSIWYG feature needs at least one visual authoring layer.", id});
    }

    std::set<std::string> action_ids;
    for (const auto& action : actions) {
        if (action.id.empty()) {
            diagnostics.push_back({"missing_action_id", "Maker WYSIWYG action is missing an id.", action.id});
        } else if (!action_ids.insert(action.id).second) {
            diagnostics.push_back({"duplicate_action_id", "Maker WYSIWYG action id is duplicated.", action.id});
        }
        if (action.trigger.empty()) {
            diagnostics.push_back({"missing_action_trigger", "Maker WYSIWYG action is missing a trigger.", action.id});
        }
        if (action.operation.empty()) {
            diagnostics.push_back({"missing_action_operation", "Maker WYSIWYG action is missing an operation.", action.id});
        }
        if (action.target.empty()) {
            diagnostics.push_back({"missing_action_target", "Maker WYSIWYG action is missing a target.", action.id});
        }
    }
    return diagnostics;
}

MakerFeaturePreview MakerWysiwygFeatureDocument::preview(const MakerFeatureRuntimeState& state,
                                                         const std::string& trigger) const {
    MakerFeaturePreview preview;
    preview.feature_type = feature_type;
    preview.trigger = trigger;
    preview.resulting_state = state;
    preview.diagnostics = validate();
    for (const auto& action : actions) {
        if (action.trigger == trigger && hasAllFlags(state, action.required_flags)) {
            preview.active_actions.push_back(action);
            applyAction(preview.resulting_state, action);
        }
    }
    if (preview.active_actions.empty()) {
        preview.diagnostics.push_back({"no_active_actions", "No maker WYSIWYG actions activate for trigger: " + trigger, id});
    }
    return preview;
}

MakerFeaturePreview MakerWysiwygFeatureDocument::execute(MakerFeatureRuntimeState& state, const std::string& trigger) const {
    auto result = preview(state, trigger);
    state = result.resulting_state;
    return result;
}

nlohmann::json MakerWysiwygFeatureDocument::toJson() const {
    nlohmann::json json;
    json["schema_version"] = schema_version;
    json["id"] = id;
    json["feature_type"] = feature_type;
    json["display_name"] = display_name;
    json["visual_layers"] = visual_layers;
    json["actions"] = nlohmann::json::array();
    for (const auto& action : actions) {
        json["actions"].push_back(actionToJson(action));
    }
    return json;
}

MakerWysiwygFeatureDocument MakerWysiwygFeatureDocument::fromJson(const nlohmann::json& json) {
    MakerWysiwygFeatureDocument document;
    document.schema_version = json.value("schema_version", "urpg.maker_wysiwyg.v1");
    document.id = json.value("id", "");
    document.feature_type = json.value("feature_type", "");
    document.display_name = json.value("display_name", "");
    document.visual_layers = json.value("visual_layers", std::vector<std::string>{});
    for (const auto& action_json : json.value("actions", nlohmann::json::array())) {
        document.actions.push_back(actionFromJson(action_json));
    }
    return document;
}

std::vector<std::string> makerWysiwygFeatureTypes() {
    return {"project_search_everywhere",
            "broken_reference_repair",
            "mass_edit_batch_operations",
            "project_diff_change_review",
            "template_instance_sync",
            "parallax_mapping_editor",
            "collision_passability_visualizer",
            "world_map_route_planner",
            "secret_hidden_object_authoring",
            "biome_rule_system",
            "dialogue_relationship_matrix",
            "branch_coverage_preview",
            "cutscene_blocking_tool",
            "localization_context_review",
            "damage_formula_visual_lab",
            "enemy_troop_timeline_preview",
            "skill_combo_chain_builder",
            "bestiary_discovery_system",
            "formation_positioning_system",
            "menu_builder",
            "journal_hub",
            "notification_toast_system",
            "input_prompt_skinner",
            "playtest_session_recorder",
            "bug_report_packager",
            "performance_heatmap",
            "release_checklist_dashboard",
            "store_page_asset_generator",
            "mod_conflict_visualizer",
            "mod_packaging_wizard",
            "plugin_to_native_migration_advisor",
            "fast_travel_system",
            "platformer_game_type",
            "gacha_system",
            "map_zoom_system",
            "pictures_ui_creator",
            "realtime_event_effects_builder",
            "directional_shadow_system",
            "dynamic_environment_effects_builder",
            "camera_director",
            "screen_filter_post_fx_builder",
            "weather_composer",
            "lighting_painter",
            "platformer_physics_lab",
            "side_view_action_combat_kit",
            "summon_gacha_banner_builder",
            "fast_travel_map_builder",
            "quest_board_mission_builder",
            "relationship_event_scheduler",
            "farming_garden_plot_builder",
            "fishing_minigame_builder",
            "mount_vehicle_system",
            "stealth_system",
            "puzzle_logic_board",
            "in_game_phone_messenger",
            "crafting_recipe_graph",
            "housing_decoration_editor",
            "companion_command_wheel",
            "title_save_screen_builder",
            "credits_ending_builder",
            "tutorial_overlay_builder",
            "accessibility_preview_lab",
            "economy_balance_simulator",
            "enemy_encounter_zone_painter",
            "random_dungeon_room_stitcher",
            "achievement_visual_builder",
            "mod_marketplace_packager"};
}

nlohmann::json makerFeaturePreviewToJson(const MakerFeaturePreview& preview) {
    nlohmann::json json{{"feature_type", preview.feature_type},
                        {"trigger", preview.trigger},
                        {"resulting_state", stateToJson(preview.resulting_state)}};
    json["active_actions"] = nlohmann::json::array();
    for (const auto& action : preview.active_actions) {
        json["active_actions"].push_back(actionToJson(action));
    }
    json["diagnostics"] = nlohmann::json::array();
    for (const auto& diagnostic : preview.diagnostics) {
        json["diagnostics"].push_back({{"code", diagnostic.code}, {"message", diagnostic.message}, {"id", diagnostic.id}});
    }
    return json;
}

} // namespace urpg::maker
