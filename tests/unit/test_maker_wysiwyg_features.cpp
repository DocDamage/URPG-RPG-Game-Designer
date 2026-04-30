#include "engine/core/editor/editor_panel_registry.h"
#include "engine/core/maker/maker_wysiwyg_feature.h"
#include "editor/maker/maker_wysiwyg_panel.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>

namespace {

nlohmann::json loadFixture(const std::string& name) {
    const auto path = std::filesystem::path(URPG_SOURCE_DIR) / "content" / "fixtures" / (name + "_fixture.json");
    std::ifstream stream(path);
    REQUIRE(stream.good());
    return nlohmann::json::parse(stream);
}

const std::map<std::string, std::string>& fixtureByFeature() {
    static const std::map<std::string, std::string> fixtures = {
        {"project_search_everywhere", "project_search_everywhere"},
        {"broken_reference_repair", "broken_reference_repair"},
        {"mass_edit_batch_operations", "mass_edit_batch_operations"},
        {"project_diff_change_review", "project_diff_change_review"},
        {"template_instance_sync", "template_instance_sync"},
        {"parallax_mapping_editor", "parallax_mapping_editor"},
        {"collision_passability_visualizer", "collision_passability_visualizer"},
        {"world_map_route_planner", "world_map_route_planner"},
        {"secret_hidden_object_authoring", "secret_hidden_object_authoring"},
        {"biome_rule_system", "biome_rule_system"},
        {"dialogue_relationship_matrix", "dialogue_relationship_matrix"},
        {"branch_coverage_preview", "branch_coverage_preview"},
        {"cutscene_blocking_tool", "cutscene_blocking_tool"},
        {"localization_context_review", "localization_context_review"},
        {"damage_formula_visual_lab", "damage_formula_visual_lab"},
        {"enemy_troop_timeline_preview", "enemy_troop_timeline_preview"},
        {"skill_combo_chain_builder", "skill_combo_chain_builder"},
        {"bestiary_discovery_system", "bestiary_discovery_system"},
        {"formation_positioning_system", "formation_positioning_system"},
        {"menu_builder", "menu_builder"},
        {"journal_hub", "journal_hub"},
        {"notification_toast_system", "notification_toast_system"},
        {"input_prompt_skinner", "input_prompt_skinner"},
        {"playtest_session_recorder", "playtest_session_recorder"},
        {"bug_report_packager", "bug_report_packager"},
        {"performance_heatmap", "performance_heatmap"},
        {"release_checklist_dashboard", "release_checklist_dashboard"},
        {"store_page_asset_generator", "store_page_asset_generator"},
        {"mod_conflict_visualizer", "mod_conflict_visualizer"},
        {"mod_packaging_wizard", "mod_packaging_wizard"},
        {"plugin_to_native_migration_advisor", "plugin_to_native_migration_advisor"},
        {"fast_travel_system", "fast_travel_system"},
        {"platformer_game_type", "platformer_game_type"},
        {"gacha_system", "gacha_system"},
        {"map_zoom_system", "map_zoom_system"},
        {"pictures_ui_creator", "pictures_ui_creator"},
        {"realtime_event_effects_builder", "realtime_event_effects_builder"},
        {"directional_shadow_system", "directional_shadow_system"},
        {"dynamic_environment_effects_builder", "dynamic_environment_effects_builder"},
        {"camera_director", "camera_director"},
        {"screen_filter_post_fx_builder", "screen_filter_post_fx_builder"},
        {"weather_composer", "weather_composer"},
        {"lighting_painter", "lighting_painter"},
        {"platformer_physics_lab", "platformer_physics_lab"},
        {"side_view_action_combat_kit", "side_view_action_combat_kit"},
        {"summon_gacha_banner_builder", "summon_gacha_banner_builder"},
        {"fast_travel_map_builder", "fast_travel_map_builder"},
        {"quest_board_mission_builder", "quest_board_mission_builder"},
        {"relationship_event_scheduler", "relationship_event_scheduler"},
        {"farming_garden_plot_builder", "farming_garden_plot_builder"},
        {"fishing_minigame_builder", "fishing_minigame_builder"},
        {"mount_vehicle_system", "mount_vehicle_system"},
        {"stealth_system", "stealth_system"},
        {"puzzle_logic_board", "puzzle_logic_board"},
        {"in_game_phone_messenger", "in_game_phone_messenger"},
        {"crafting_recipe_graph", "crafting_recipe_graph"},
        {"housing_decoration_editor", "housing_decoration_editor"},
        {"companion_command_wheel", "companion_command_wheel"},
        {"title_save_screen_builder", "title_save_screen_builder"},
        {"credits_ending_builder", "credits_ending_builder"},
        {"tutorial_overlay_builder", "tutorial_overlay_builder"},
        {"accessibility_preview_lab", "accessibility_preview_lab"},
        {"economy_balance_simulator", "economy_balance_simulator"},
        {"enemy_encounter_zone_painter", "enemy_encounter_zone_painter"},
        {"random_dungeon_room_stitcher", "random_dungeon_room_stitcher"},
        {"achievement_visual_builder", "achievement_visual_builder"},
        {"mod_marketplace_packager", "mod_marketplace_packager"},
        {"proximity_compass_objective_radar", "proximity_compass_objective_radar"},
        {"resource_manager_unused_asset_cleaner", "resource_manager_unused_asset_cleaner"},
        {"visual_inventory_builder", "visual_inventory_builder"},
        {"animated_pictures_timeline", "animated_pictures_timeline"},
        {"horror_fx_builder", "horror_fx_builder"},
        {"cutscene_skip_fast_forward", "cutscene_skip_fast_forward"},
        {"translation_helper_workspace", "translation_helper_workspace"},
        {"deployment_cook_dashboard", "deployment_cook_dashboard"},
        {"break_shield_system", "break_shield_system"},
        {"boost_point_system", "boost_point_system"},
        {"battle_weapon_swap", "battle_weapon_swap"},
        {"order_turn_battle_system", "order_turn_battle_system"},
        {"item_concoction_system", "item_concoction_system"},
        {"unison_attack_builder", "unison_attack_builder"},
        {"timed_attack_qte_builder", "timed_attack_qte_builder"},
        {"action_sequence_impact_builder", "action_sequence_impact_builder"},
        {"side_battle_status_ui_builder", "side_battle_status_ui_builder"},
        {"victory_screen_builder", "victory_screen_builder"},
        {"state_tooltip_display", "state_tooltip_display"},
        {"animated_loading_screen_builder", "animated_loading_screen_builder"},
        {"popup_builder", "popup_builder"},
        {"animated_window_builder", "animated_window_builder"},
        {"video_player_surface", "video_player_surface"},
        {"tactical_battle_system", "tactical_battle_system"},
        {"bullet_hell_maker", "bullet_hell_maker"},
        {"duelist_card_battle_system", "duelist_card_battle_system"},
        {"spin_top_battle_system", "spin_top_battle_system"},
        {"recruiting_board", "recruiting_board"},
        {"step_rewards_system", "step_rewards_system"},
        {"event_spawner_solution", "event_spawner_solution"},
        {"smart_event_timers_progress", "smart_event_timers_progress"},
        {"actor_construct", "actor_construct"},
        {"equip_construct_visible_equipment", "equip_construct_visible_equipment"},
        {"skill_equip_system", "skill_equip_system"},
        {"front_view_battler_mode", "front_view_battler_mode"},
    };
    return fixtures;
}

bool containsOperation(const std::vector<std::string>& operations, const std::string& operation) {
    return std::find(operations.begin(), operations.end(), operation) != operations.end();
}

} // namespace

TEST_CASE("Maker WYSIWYG features expose runtime editor saved-data and registry hooks",
          "[maker][wysiwyg][features]") {
    const auto features = urpg::maker::makerWysiwygFeatureTypes();
    REQUIRE(features.size() == 102);

    for (const auto& feature : features) {
        CAPTURE(feature);
        const auto document = urpg::maker::MakerWysiwygFeatureDocument::fromJson(loadFixture(fixtureByFeature().at(feature)));

        REQUIRE(document.feature_type == feature);
        REQUIRE(document.validate().empty());
        REQUIRE_FALSE(document.visual_layers.empty());
        REQUIRE_FALSE(document.actions.empty());

        const auto* entry = urpg::editor::findEditorPanelRegistryEntry(feature);
        REQUIRE(entry != nullptr);
        REQUIRE(entry->exposure == urpg::editor::EditorPanelExposure::Deferred);

        urpg::maker::MakerFeatureRuntimeState state;
        const auto trigger = document.actions.front().trigger;
        const auto preview = document.preview(state, trigger);
        REQUIRE(preview.active_actions.size() == 1);
        REQUIRE(preview.resulting_state.emitted_operations.size() == 1);

        const auto executed = document.execute(state, trigger);
        REQUIRE(executed.active_actions == preview.active_actions);
        REQUIRE_FALSE(state.emitted_operations.empty());

        urpg::editor::maker::MakerWysiwygPanel panel;
        panel.loadDocument(document);
        panel.setPreviewContext({}, trigger);
        panel.render();

        REQUIRE(panel.snapshot().feature_type == feature);
        REQUIRE(panel.snapshot().visual_layer_count == document.visual_layers.size());
        REQUIRE(panel.snapshot().active_action_count == 1);
        REQUIRE(panel.snapshot().emitted_operation_count == 1);
        REQUIRE(panel.saveProjectData() == document.toJson());
        REQUIRE(urpg::maker::makerFeaturePreviewToJson(panel.preview())["active_actions"].size() == 1);
    }
}

TEST_CASE("Maker WYSIWYG feature-specific operations match the requested maker backlog",
          "[maker][wysiwyg][features]") {
    std::map<std::string, std::pair<std::string, std::string>> expected = {
        {"project_search_everywhere", {"search_query_changed", "search_project:project_index"}},
        {"broken_reference_repair", {"scan_references", "repair_reference:missing_asset"}},
        {"parallax_mapping_editor", {"map_preview", "render_parallax_layers:current_map"}},
        {"damage_formula_visual_lab", {"formula_changed", "plot_damage_formula:formula_graph"}},
        {"playtest_session_recorder", {"playtest_started", "start_session_recording:playtest_trace"}},
        {"release_checklist_dashboard", {"release_check", "evaluate_release_checklist:release_dashboard"}},
        {"mod_packaging_wizard", {"package_mod", "build_mod_package:mod_manifest"}},
        {"plugin_to_native_migration_advisor", {"analyze_plugin", "suggest_native_migration:plugin_report"}},
        {"fast_travel_system", {"destination_selected", "preview_fast_travel:fast_travel_runtime"}},
        {"platformer_game_type", {"platformer_map_loaded", "preview_platformer_scene:platformer_runtime"}},
        {"gacha_system", {"pull_preview", "simulate_gacha_pull:gacha_runtime"}},
        {"map_zoom_system", {"zoom_changed", "apply_map_zoom:map_camera"}},
        {"pictures_ui_creator", {"ui_layout_preview", "render_picture_ui:picture_ui_runtime"}},
        {"realtime_event_effects_builder", {"effect_changed", "preview_event_effect:event_effect_runtime"}},
        {"directional_shadow_system", {"sun_angle_changed", "cast_directional_shadow:shadow_runtime"}},
        {"dynamic_environment_effects_builder", {"environment_preset_preview", "render_environment_effects:environment_fx_runtime"}},
        {"camera_director", {"camera_cue_scrubbed", "preview_camera_cue:camera_runtime"}},
        {"screen_filter_post_fx_builder", {"filter_stack_changed", "apply_post_fx_stack:post_fx_runtime"}},
        {"weather_composer", {"weather_preview", "compose_weather:weather_runtime"}},
        {"lighting_painter", {"light_brush_changed", "paint_map_lighting:lighting_runtime"}},
        {"platformer_physics_lab", {"physics_value_changed", "simulate_platformer_physics:platformer_physics_runtime"}},
        {"side_view_action_combat_kit", {"attack_timeline_scrubbed", "preview_action_combat:action_combat_runtime"}},
        {"summon_gacha_banner_builder", {"banner_preview", "build_summon_banner:summon_banner_runtime"}},
        {"fast_travel_map_builder", {"route_node_selected", "render_fast_travel_map:fast_travel_map_runtime"}},
        {"quest_board_mission_builder", {"mission_selected", "preview_mission_board:mission_board_runtime"}},
        {"relationship_event_scheduler", {"affinity_event_preview", "schedule_relationship_event:relationship_schedule_runtime"}},
        {"farming_garden_plot_builder", {"crop_rule_changed", "preview_crop_growth:farming_runtime"}},
        {"fishing_minigame_builder", {"fishing_spot_selected", "preview_fishing_minigame:fishing_runtime"}},
        {"mount_vehicle_system", {"vehicle_selected", "preview_mount_vehicle:vehicle_runtime"}},
        {"stealth_system", {"guard_route_preview", "simulate_stealth_state:stealth_runtime"}},
        {"puzzle_logic_board", {"puzzle_node_changed", "evaluate_puzzle_logic:puzzle_logic_runtime"}},
        {"in_game_phone_messenger", {"message_thread_selected", "render_phone_message:phone_runtime"}},
        {"crafting_recipe_graph", {"recipe_selected", "preview_recipe_graph:crafting_graph_runtime"}},
        {"housing_decoration_editor", {"furniture_dragged", "place_housing_decor:housing_runtime"}},
        {"companion_command_wheel", {"wheel_opened", "render_companion_wheel:companion_wheel_runtime"}},
        {"title_save_screen_builder", {"scene_preview", "render_title_save_screen:title_save_runtime"}},
        {"credits_ending_builder", {"ending_selected", "preview_ending_sequence:ending_runtime"}},
        {"tutorial_overlay_builder", {"tutorial_step_selected", "render_tutorial_overlay:tutorial_runtime"}},
        {"accessibility_preview_lab", {"accessibility_profile_changed", "preview_accessibility_profile:accessibility_runtime"}},
        {"economy_balance_simulator", {"economy_model_changed", "simulate_economy_balance:economy_runtime"}},
        {"enemy_encounter_zone_painter", {"encounter_zone_painted", "paint_encounter_zone:encounter_zone_runtime"}},
        {"random_dungeon_room_stitcher", {"dungeon_seed_changed", "generate_dungeon_layout:dungeon_stitcher_runtime"}},
        {"achievement_visual_builder", {"achievement_selected", "render_achievement_visual:achievement_runtime"}},
        {"mod_marketplace_packager", {"marketplace_package_preview", "package_marketplace_mod:mod_marketplace_runtime"}},
        {"proximity_compass_objective_radar", {"objective_marker_changed", "render_objective_radar:compass_runtime"}},
        {"resource_manager_unused_asset_cleaner", {"asset_scan_requested", "scan_unused_assets:resource_manager_runtime"}},
        {"visual_inventory_builder", {"inventory_layout_preview", "render_visual_inventory:inventory_runtime"}},
        {"animated_pictures_timeline", {"picture_timeline_scrubbed", "play_picture_animation:picture_animation_runtime"}},
        {"horror_fx_builder", {"horror_cue_selected", "preview_horror_fx:horror_fx_runtime"}},
        {"cutscene_skip_fast_forward", {"skip_rule_changed", "preview_cutscene_skip:cutscene_skip_runtime"}},
        {"translation_helper_workspace", {"translation_export_requested", "export_translation_sheet:translation_runtime"}},
        {"deployment_cook_dashboard", {"cook_requested", "cook_release_package:deployment_runtime"}},
        {"break_shield_system", {"shield_rule_changed", "simulate_break_shield:break_shield_runtime"}},
        {"boost_point_system", {"boost_rule_preview", "simulate_boost_points:boost_runtime"}},
        {"battle_weapon_swap", {"weapon_slot_selected", "preview_weapon_swap:weapon_swap_runtime"}},
        {"order_turn_battle_system", {"turn_order_simulated", "preview_turn_order:order_turn_runtime"}},
        {"item_concoction_system", {"concoction_pair_selected", "preview_item_concoction:concoction_runtime"}},
        {"unison_attack_builder", {"unison_attack_selected", "preview_unison_attack:unison_runtime"}},
        {"timed_attack_qte_builder", {"qte_sequence_preview", "preview_timed_attack:qte_runtime"}},
        {"action_sequence_impact_builder", {"impact_timeline_scrubbed", "preview_action_impact:action_impact_runtime"}},
        {"side_battle_status_ui_builder", {"battle_status_layout_changed", "render_side_battle_status:side_status_runtime"}},
        {"victory_screen_builder", {"victory_layout_preview", "render_victory_screen:victory_runtime"}},
        {"state_tooltip_display", {"state_icon_hovered", "render_state_tooltip:state_tooltip_runtime"}},
        {"animated_loading_screen_builder", {"loading_screen_preview", "render_loading_screen:loading_runtime"}},
        {"popup_builder", {"popup_style_preview", "render_popup:popup_runtime"}},
        {"animated_window_builder", {"window_animation_preview", "render_animated_window:animated_window_runtime"}},
        {"video_player_surface", {"video_preview_requested", "play_video_surface:video_runtime"}},
        {"tactical_battle_system", {"tactical_map_preview", "preview_tactical_battle:tactical_runtime"}},
        {"bullet_hell_maker", {"bullet_pattern_scrubbed", "preview_bullet_pattern:bullet_hell_runtime"}},
        {"duelist_card_battle_system", {"card_duel_preview", "preview_card_duel:card_battle_runtime"}},
        {"spin_top_battle_system", {"spin_top_arena_preview", "simulate_spin_top_battle:spin_top_runtime"}},
        {"recruiting_board", {"candidate_selected", "preview_recruiting_board:recruiting_runtime"}},
        {"step_rewards_system", {"step_milestone_selected", "preview_step_reward:step_reward_runtime"}},
        {"event_spawner_solution", {"spawn_rule_preview", "spawn_template_event:event_spawner_runtime"}},
        {"smart_event_timers_progress", {"timer_rule_changed", "preview_event_timer:event_timer_runtime"}},
        {"actor_construct", {"actor_part_changed", "preview_actor_construct:actor_construct_runtime"}},
        {"equip_construct_visible_equipment", {"equipment_layer_changed", "preview_visible_equipment:visible_equipment_runtime"}},
        {"skill_equip_system", {"skill_loadout_changed", "preview_skill_equip:skill_equip_runtime"}},
        {"front_view_battler_mode", {"front_view_layout_preview", "render_front_view_battle:front_view_battle_runtime"}},
    };

    for (const auto& [feature, triggerAndOperation] : expected) {
        CAPTURE(feature);
        auto document = urpg::maker::MakerWysiwygFeatureDocument::fromJson(loadFixture(fixtureByFeature().at(feature)));
        urpg::maker::MakerFeatureRuntimeState state;
        const auto preview = document.execute(state, triggerAndOperation.first);
        REQUIRE(preview.active_actions.size() == 1);
        REQUIRE(containsOperation(state.emitted_operations, triggerAndOperation.second));
    }
}

TEST_CASE("Maker WYSIWYG features report broken authoring diagnostics",
          "[maker][wysiwyg][features]") {
    urpg::maker::MakerWysiwygFeatureDocument document;
    document.id = "broken";
    document.feature_type = "project_search_everywhere";
    document.display_name = "Broken";
    document.actions.push_back({"", "", "", "", "", {}, {}});

    const auto diagnostics = document.validate();
    REQUIRE(diagnostics.size() >= 5);

    const auto preview = document.preview({}, "missing_trigger");
    REQUIRE(preview.active_actions.empty());
    REQUIRE_FALSE(preview.diagnostics.empty());
}
