#include "engine/core/editor/editor_panel_registry.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <set>
#include <string>

namespace {

bool ContainsId(const std::vector<std::string>& ids, const std::string& id) {
    return std::find(ids.begin(), ids.end(), id) != ids.end();
}

} // namespace

TEST_CASE("Editor panel registry exposes canonical top-level panels", "[editor][panel][registry]") {
    const auto ids = urpg::editor::requiredTopLevelPanelIds();

    REQUIRE(ids.size() == 146);
    REQUIRE(ContainsId(ids, "diagnostics"));
    REQUIRE(ContainsId(ids, "assets"));
    REQUIRE(ContainsId(ids, "ability"));
    REQUIRE(ContainsId(ids, "patterns"));
    REQUIRE(ContainsId(ids, "mod"));
    REQUIRE(ContainsId(ids, "analytics"));
    REQUIRE(ContainsId(ids, "status_effect_designer"));
    REQUIRE(ContainsId(ids, "enemy_ai_behavior_tree"));
    REQUIRE(ContainsId(ids, "boss_phase_script"));
    REQUIRE(ContainsId(ids, "equipment_set_bonus"));
    REQUIRE(ContainsId(ids, "dungeon_room_flow"));
    REQUIRE(ContainsId(ids, "companion_banter"));
    REQUIRE(ContainsId(ids, "quest_choice_consequence"));
    REQUIRE(ContainsId(ids, "shop_economy_sim_lab"));
    REQUIRE(ContainsId(ids, "puzzle_mechanic_builder"));
    REQUIRE(ContainsId(ids, "world_state_timeline"));
    REQUIRE(ContainsId(ids, "tactical_terrain_effects"));
    REQUIRE(ContainsId(ids, "procedural_content_rules"));
    REQUIRE(ContainsId(ids, "smart_event_workflow"));
    REQUIRE(ContainsId(ids, "event_template_library"));
    REQUIRE(ContainsId(ids, "interaction_prompt_system"));
    REQUIRE(ContainsId(ids, "message_log_history"));
    REQUIRE(ContainsId(ids, "minimap_fog_of_war"));
    REQUIRE(ContainsId(ids, "picture_hotspot_common_event"));
    REQUIRE(ContainsId(ids, "common_event_menu_builder"));
    REQUIRE(ContainsId(ids, "developer_debug_overlay"));
    REQUIRE(ContainsId(ids, "switch_variable_inspector"));
    REQUIRE(ContainsId(ids, "asset_dlc_library_manager"));
    REQUIRE(ContainsId(ids, "hud_maker"));
    REQUIRE(ContainsId(ids, "plugin_conflict_resolver"));
    REQUIRE(ContainsId(ids, "project_search_everywhere"));
    REQUIRE(ContainsId(ids, "broken_reference_repair"));
    REQUIRE(ContainsId(ids, "mass_edit_batch_operations"));
    REQUIRE(ContainsId(ids, "project_diff_change_review"));
    REQUIRE(ContainsId(ids, "template_instance_sync"));
    REQUIRE(ContainsId(ids, "parallax_mapping_editor"));
    REQUIRE(ContainsId(ids, "collision_passability_visualizer"));
    REQUIRE(ContainsId(ids, "world_map_route_planner"));
    REQUIRE(ContainsId(ids, "secret_hidden_object_authoring"));
    REQUIRE(ContainsId(ids, "biome_rule_system"));
    REQUIRE(ContainsId(ids, "dialogue_relationship_matrix"));
    REQUIRE(ContainsId(ids, "branch_coverage_preview"));
    REQUIRE(ContainsId(ids, "cutscene_blocking_tool"));
    REQUIRE(ContainsId(ids, "localization_context_review"));
    REQUIRE(ContainsId(ids, "damage_formula_visual_lab"));
    REQUIRE(ContainsId(ids, "enemy_troop_timeline_preview"));
    REQUIRE(ContainsId(ids, "skill_combo_chain_builder"));
    REQUIRE(ContainsId(ids, "bestiary_discovery_system"));
    REQUIRE(ContainsId(ids, "formation_positioning_system"));
    REQUIRE(ContainsId(ids, "menu_builder"));
    REQUIRE(ContainsId(ids, "journal_hub"));
    REQUIRE(ContainsId(ids, "notification_toast_system"));
    REQUIRE(ContainsId(ids, "input_prompt_skinner"));
    REQUIRE(ContainsId(ids, "playtest_session_recorder"));
    REQUIRE(ContainsId(ids, "bug_report_packager"));
    REQUIRE(ContainsId(ids, "performance_heatmap"));
    REQUIRE(ContainsId(ids, "release_checklist_dashboard"));
    REQUIRE(ContainsId(ids, "store_page_asset_generator"));
    REQUIRE(ContainsId(ids, "mod_conflict_visualizer"));
    REQUIRE(ContainsId(ids, "mod_packaging_wizard"));
    REQUIRE(ContainsId(ids, "plugin_to_native_migration_advisor"));
    REQUIRE(ContainsId(ids, "fast_travel_system"));
    REQUIRE(ContainsId(ids, "platformer_game_type"));
    REQUIRE(ContainsId(ids, "gacha_system"));
    REQUIRE(ContainsId(ids, "map_zoom_system"));
    REQUIRE(ContainsId(ids, "pictures_ui_creator"));
    REQUIRE(ContainsId(ids, "realtime_event_effects_builder"));
    REQUIRE(ContainsId(ids, "directional_shadow_system"));
    REQUIRE(ContainsId(ids, "dynamic_environment_effects_builder"));
    REQUIRE(ContainsId(ids, "camera_director"));
    REQUIRE(ContainsId(ids, "screen_filter_post_fx_builder"));
    REQUIRE(ContainsId(ids, "weather_composer"));
    REQUIRE(ContainsId(ids, "lighting_painter"));
    REQUIRE(ContainsId(ids, "platformer_physics_lab"));
    REQUIRE(ContainsId(ids, "side_view_action_combat_kit"));
    REQUIRE(ContainsId(ids, "summon_gacha_banner_builder"));
    REQUIRE(ContainsId(ids, "fast_travel_map_builder"));
    REQUIRE(ContainsId(ids, "quest_board_mission_builder"));
    REQUIRE(ContainsId(ids, "relationship_event_scheduler"));
    REQUIRE(ContainsId(ids, "farming_garden_plot_builder"));
    REQUIRE(ContainsId(ids, "fishing_minigame_builder"));
    REQUIRE(ContainsId(ids, "mount_vehicle_system"));
    REQUIRE(ContainsId(ids, "stealth_system"));
    REQUIRE(ContainsId(ids, "puzzle_logic_board"));
    REQUIRE(ContainsId(ids, "in_game_phone_messenger"));
    REQUIRE(ContainsId(ids, "crafting_recipe_graph"));
    REQUIRE(ContainsId(ids, "housing_decoration_editor"));
    REQUIRE(ContainsId(ids, "companion_command_wheel"));
    REQUIRE(ContainsId(ids, "title_save_screen_builder"));
    REQUIRE(ContainsId(ids, "credits_ending_builder"));
    REQUIRE(ContainsId(ids, "tutorial_overlay_builder"));
    REQUIRE(ContainsId(ids, "accessibility_preview_lab"));
    REQUIRE(ContainsId(ids, "economy_balance_simulator"));
    REQUIRE(ContainsId(ids, "enemy_encounter_zone_painter"));
    REQUIRE(ContainsId(ids, "random_dungeon_room_stitcher"));
    REQUIRE(ContainsId(ids, "achievement_visual_builder"));
    REQUIRE(ContainsId(ids, "mod_marketplace_packager"));
    REQUIRE(ContainsId(ids, "proximity_compass_objective_radar"));
    REQUIRE(ContainsId(ids, "resource_manager_unused_asset_cleaner"));
    REQUIRE(ContainsId(ids, "visual_inventory_builder"));
    REQUIRE(ContainsId(ids, "animated_pictures_timeline"));
    REQUIRE(ContainsId(ids, "horror_fx_builder"));
    REQUIRE(ContainsId(ids, "cutscene_skip_fast_forward"));
    REQUIRE(ContainsId(ids, "translation_helper_workspace"));
    REQUIRE(ContainsId(ids, "deployment_cook_dashboard"));
    REQUIRE(ContainsId(ids, "3d_dungeon_world"));
    REQUIRE(ContainsId(ids, "break_shield_system"));
    REQUIRE(ContainsId(ids, "boost_point_system"));
    REQUIRE(ContainsId(ids, "battle_weapon_swap"));
    REQUIRE(ContainsId(ids, "order_turn_battle_system"));
    REQUIRE(ContainsId(ids, "item_concoction_system"));
    REQUIRE(ContainsId(ids, "unison_attack_builder"));
    REQUIRE(ContainsId(ids, "timed_attack_qte_builder"));
    REQUIRE(ContainsId(ids, "action_sequence_impact_builder"));
    REQUIRE(ContainsId(ids, "side_battle_status_ui_builder"));
    REQUIRE(ContainsId(ids, "victory_screen_builder"));
    REQUIRE(ContainsId(ids, "state_tooltip_display"));
    REQUIRE(ContainsId(ids, "animated_loading_screen_builder"));
    REQUIRE(ContainsId(ids, "popup_builder"));
    REQUIRE(ContainsId(ids, "animated_window_builder"));
    REQUIRE(ContainsId(ids, "video_player_surface"));
    REQUIRE(ContainsId(ids, "tactical_battle_system"));
    REQUIRE(ContainsId(ids, "bullet_hell_maker"));
    REQUIRE(ContainsId(ids, "duelist_card_battle_system"));
    REQUIRE(ContainsId(ids, "spin_top_battle_system"));
    REQUIRE(ContainsId(ids, "recruiting_board"));
    REQUIRE(ContainsId(ids, "step_rewards_system"));
    REQUIRE(ContainsId(ids, "event_spawner_solution"));
    REQUIRE(ContainsId(ids, "smart_event_timers_progress"));
    REQUIRE(ContainsId(ids, "actor_construct"));
    REQUIRE(ContainsId(ids, "equip_construct_visible_equipment"));
    REQUIRE(ContainsId(ids, "skill_equip_system"));
    REQUIRE(ContainsId(ids, "front_view_battler_mode"));
    REQUIRE(ContainsId(ids, "quest"));
    REQUIRE(ContainsId(ids, "skill_tree"));
    REQUIRE(ContainsId(ids, "relationship"));
    REQUIRE(ContainsId(ids, "timeline"));
    REQUIRE(ContainsId(ids, "cutscene_timeline"));
    REQUIRE(ContainsId(ids, "balance"));
    REQUIRE(ContainsId(ids, "encounter_designer"));
    REQUIRE(ContainsId(ids, "loot_generator"));
    REQUIRE(ContainsId(ids, "crafting"));
    REQUIRE(ContainsId(ids, "monster_collection"));
    REQUIRE(ContainsId(ids, "npc"));
    REQUIRE(ContainsId(ids, "metroidvania_gates"));
    REQUIRE(ContainsId(ids, "spatial_authoring"));

    const auto* patterns = urpg::editor::findEditorPanelRegistryEntry("patterns");
    REQUIRE(patterns != nullptr);
    REQUIRE(patterns->exposure == urpg::editor::EditorPanelExposure::ReleaseTopLevel);
    REQUIRE(patterns->title == "Pattern Field Editor");
}

TEST_CASE("Editor panel registry documents every hidden compiled panel", "[editor][panel][registry]") {
    REQUIRE(urpg::editor::hiddenEditorPanelEntriesHaveReasons());

    std::set<std::string> seen;
    for (const auto& entry : urpg::editor::editorPanelRegistry()) {
        REQUIRE_FALSE(entry.id.empty());
        REQUIRE_FALSE(entry.title.empty());
        REQUIRE_FALSE(entry.category.empty());
        REQUIRE_FALSE(entry.owner.empty());
        REQUIRE(seen.insert(entry.id).second);

        const auto isKnownExposure =
            entry.exposure == urpg::editor::EditorPanelExposure::ReleaseTopLevel ||
            entry.exposure == urpg::editor::EditorPanelExposure::Nested ||
            entry.exposure == urpg::editor::EditorPanelExposure::DevOnly ||
            entry.exposure == urpg::editor::EditorPanelExposure::Deferred;
        REQUIRE(isKnownExposure);

        if (entry.exposure != urpg::editor::EditorPanelExposure::ReleaseTopLevel) {
            REQUIRE_FALSE(entry.reason.empty());
        }
    }
}

TEST_CASE("Editor panel registry classifies diagnostics and incubating workspaces", "[editor][panel][registry]") {
    const auto* saveInspector = urpg::editor::findEditorPanelRegistryEntry("save_inspector");
    REQUIRE(saveInspector != nullptr);
    REQUIRE(saveInspector->exposure == urpg::editor::EditorPanelExposure::Nested);

    const auto* spatialAuthoring = urpg::editor::findEditorPanelRegistryEntry("spatial_authoring");
    REQUIRE(spatialAuthoring != nullptr);
    REQUIRE(spatialAuthoring->exposure == urpg::editor::EditorPanelExposure::ReleaseTopLevel);

    const auto* modSdk = urpg::editor::findEditorPanelRegistryEntry("mod_sdk");
    REQUIRE(modSdk != nullptr);
    REQUIRE(modSdk->exposure == urpg::editor::EditorPanelExposure::DevOnly);

    const auto topLevel = urpg::editor::topLevelEditorPanels();
    REQUIRE(topLevel.size() == urpg::editor::requiredTopLevelPanelIds().size());
}

TEST_CASE("Editor smoke coverage follows every registered top-level panel", "[editor][panel][registry][smoke]") {
    const auto topLevelIds = urpg::editor::requiredTopLevelPanelIds();
    const auto smokeIds = urpg::editor::smokeRequiredEditorPanelIds();

    REQUIRE(smokeIds == topLevelIds);
    REQUIRE(ContainsId(smokeIds, "patterns"));
    REQUIRE(ContainsId(smokeIds, "status_effect_designer"));
    REQUIRE(ContainsId(smokeIds, "enemy_ai_behavior_tree"));
    REQUIRE(ContainsId(smokeIds, "boss_phase_script"));
    REQUIRE(ContainsId(smokeIds, "equipment_set_bonus"));
    REQUIRE(ContainsId(smokeIds, "dungeon_room_flow"));
    REQUIRE(ContainsId(smokeIds, "companion_banter"));
    REQUIRE(ContainsId(smokeIds, "quest_choice_consequence"));
    REQUIRE(ContainsId(smokeIds, "shop_economy_sim_lab"));
    REQUIRE(ContainsId(smokeIds, "puzzle_mechanic_builder"));
    REQUIRE(ContainsId(smokeIds, "world_state_timeline"));
    REQUIRE(ContainsId(smokeIds, "tactical_terrain_effects"));
    REQUIRE(ContainsId(smokeIds, "procedural_content_rules"));
    REQUIRE(ContainsId(smokeIds, "smart_event_workflow"));
    REQUIRE(ContainsId(smokeIds, "event_template_library"));
    REQUIRE(ContainsId(smokeIds, "interaction_prompt_system"));
    REQUIRE(ContainsId(smokeIds, "message_log_history"));
    REQUIRE(ContainsId(smokeIds, "minimap_fog_of_war"));
    REQUIRE(ContainsId(smokeIds, "picture_hotspot_common_event"));
    REQUIRE(ContainsId(smokeIds, "common_event_menu_builder"));
    REQUIRE(ContainsId(smokeIds, "developer_debug_overlay"));
    REQUIRE(ContainsId(smokeIds, "switch_variable_inspector"));
    REQUIRE(ContainsId(smokeIds, "asset_dlc_library_manager"));
    REQUIRE(ContainsId(smokeIds, "hud_maker"));
    REQUIRE(ContainsId(smokeIds, "plugin_conflict_resolver"));
    REQUIRE(ContainsId(smokeIds, "project_search_everywhere"));
    REQUIRE(ContainsId(smokeIds, "broken_reference_repair"));
    REQUIRE(ContainsId(smokeIds, "mass_edit_batch_operations"));
    REQUIRE(ContainsId(smokeIds, "project_diff_change_review"));
    REQUIRE(ContainsId(smokeIds, "template_instance_sync"));
    REQUIRE(ContainsId(smokeIds, "parallax_mapping_editor"));
    REQUIRE(ContainsId(smokeIds, "collision_passability_visualizer"));
    REQUIRE(ContainsId(smokeIds, "world_map_route_planner"));
    REQUIRE(ContainsId(smokeIds, "secret_hidden_object_authoring"));
    REQUIRE(ContainsId(smokeIds, "biome_rule_system"));
    REQUIRE(ContainsId(smokeIds, "dialogue_relationship_matrix"));
    REQUIRE(ContainsId(smokeIds, "branch_coverage_preview"));
    REQUIRE(ContainsId(smokeIds, "cutscene_blocking_tool"));
    REQUIRE(ContainsId(smokeIds, "localization_context_review"));
    REQUIRE(ContainsId(smokeIds, "damage_formula_visual_lab"));
    REQUIRE(ContainsId(smokeIds, "enemy_troop_timeline_preview"));
    REQUIRE(ContainsId(smokeIds, "skill_combo_chain_builder"));
    REQUIRE(ContainsId(smokeIds, "bestiary_discovery_system"));
    REQUIRE(ContainsId(smokeIds, "formation_positioning_system"));
    REQUIRE(ContainsId(smokeIds, "menu_builder"));
    REQUIRE(ContainsId(smokeIds, "journal_hub"));
    REQUIRE(ContainsId(smokeIds, "notification_toast_system"));
    REQUIRE(ContainsId(smokeIds, "input_prompt_skinner"));
    REQUIRE(ContainsId(smokeIds, "playtest_session_recorder"));
    REQUIRE(ContainsId(smokeIds, "bug_report_packager"));
    REQUIRE(ContainsId(smokeIds, "performance_heatmap"));
    REQUIRE(ContainsId(smokeIds, "release_checklist_dashboard"));
    REQUIRE(ContainsId(smokeIds, "store_page_asset_generator"));
    REQUIRE(ContainsId(smokeIds, "mod_conflict_visualizer"));
    REQUIRE(ContainsId(smokeIds, "mod_packaging_wizard"));
    REQUIRE(ContainsId(smokeIds, "plugin_to_native_migration_advisor"));
    REQUIRE(ContainsId(smokeIds, "fast_travel_system"));
    REQUIRE(ContainsId(smokeIds, "platformer_game_type"));
    REQUIRE(ContainsId(smokeIds, "gacha_system"));
    REQUIRE(ContainsId(smokeIds, "map_zoom_system"));
    REQUIRE(ContainsId(smokeIds, "pictures_ui_creator"));
    REQUIRE(ContainsId(smokeIds, "realtime_event_effects_builder"));
    REQUIRE(ContainsId(smokeIds, "directional_shadow_system"));
    REQUIRE(ContainsId(smokeIds, "dynamic_environment_effects_builder"));
    REQUIRE(ContainsId(smokeIds, "camera_director"));
    REQUIRE(ContainsId(smokeIds, "screen_filter_post_fx_builder"));
    REQUIRE(ContainsId(smokeIds, "weather_composer"));
    REQUIRE(ContainsId(smokeIds, "lighting_painter"));
    REQUIRE(ContainsId(smokeIds, "platformer_physics_lab"));
    REQUIRE(ContainsId(smokeIds, "side_view_action_combat_kit"));
    REQUIRE(ContainsId(smokeIds, "summon_gacha_banner_builder"));
    REQUIRE(ContainsId(smokeIds, "fast_travel_map_builder"));
    REQUIRE(ContainsId(smokeIds, "quest_board_mission_builder"));
    REQUIRE(ContainsId(smokeIds, "relationship_event_scheduler"));
    REQUIRE(ContainsId(smokeIds, "farming_garden_plot_builder"));
    REQUIRE(ContainsId(smokeIds, "fishing_minigame_builder"));
    REQUIRE(ContainsId(smokeIds, "mount_vehicle_system"));
    REQUIRE(ContainsId(smokeIds, "stealth_system"));
    REQUIRE(ContainsId(smokeIds, "puzzle_logic_board"));
    REQUIRE(ContainsId(smokeIds, "in_game_phone_messenger"));
    REQUIRE(ContainsId(smokeIds, "crafting_recipe_graph"));
    REQUIRE(ContainsId(smokeIds, "housing_decoration_editor"));
    REQUIRE(ContainsId(smokeIds, "companion_command_wheel"));
    REQUIRE(ContainsId(smokeIds, "title_save_screen_builder"));
    REQUIRE(ContainsId(smokeIds, "credits_ending_builder"));
    REQUIRE(ContainsId(smokeIds, "tutorial_overlay_builder"));
    REQUIRE(ContainsId(smokeIds, "accessibility_preview_lab"));
    REQUIRE(ContainsId(smokeIds, "economy_balance_simulator"));
    REQUIRE(ContainsId(smokeIds, "enemy_encounter_zone_painter"));
    REQUIRE(ContainsId(smokeIds, "random_dungeon_room_stitcher"));
    REQUIRE(ContainsId(smokeIds, "achievement_visual_builder"));
    REQUIRE(ContainsId(smokeIds, "mod_marketplace_packager"));
    REQUIRE(ContainsId(smokeIds, "proximity_compass_objective_radar"));
    REQUIRE(ContainsId(smokeIds, "resource_manager_unused_asset_cleaner"));
    REQUIRE(ContainsId(smokeIds, "visual_inventory_builder"));
    REQUIRE(ContainsId(smokeIds, "animated_pictures_timeline"));
    REQUIRE(ContainsId(smokeIds, "horror_fx_builder"));
    REQUIRE(ContainsId(smokeIds, "cutscene_skip_fast_forward"));
    REQUIRE(ContainsId(smokeIds, "translation_helper_workspace"));
    REQUIRE(ContainsId(smokeIds, "deployment_cook_dashboard"));
    REQUIRE(ContainsId(smokeIds, "3d_dungeon_world"));
    REQUIRE(ContainsId(smokeIds, "break_shield_system"));
    REQUIRE(ContainsId(smokeIds, "boost_point_system"));
    REQUIRE(ContainsId(smokeIds, "battle_weapon_swap"));
    REQUIRE(ContainsId(smokeIds, "order_turn_battle_system"));
    REQUIRE(ContainsId(smokeIds, "item_concoction_system"));
    REQUIRE(ContainsId(smokeIds, "unison_attack_builder"));
    REQUIRE(ContainsId(smokeIds, "timed_attack_qte_builder"));
    REQUIRE(ContainsId(smokeIds, "action_sequence_impact_builder"));
    REQUIRE(ContainsId(smokeIds, "side_battle_status_ui_builder"));
    REQUIRE(ContainsId(smokeIds, "victory_screen_builder"));
    REQUIRE(ContainsId(smokeIds, "state_tooltip_display"));
    REQUIRE(ContainsId(smokeIds, "animated_loading_screen_builder"));
    REQUIRE(ContainsId(smokeIds, "popup_builder"));
    REQUIRE(ContainsId(smokeIds, "animated_window_builder"));
    REQUIRE(ContainsId(smokeIds, "video_player_surface"));
    REQUIRE(ContainsId(smokeIds, "tactical_battle_system"));
    REQUIRE(ContainsId(smokeIds, "bullet_hell_maker"));
    REQUIRE(ContainsId(smokeIds, "duelist_card_battle_system"));
    REQUIRE(ContainsId(smokeIds, "spin_top_battle_system"));
    REQUIRE(ContainsId(smokeIds, "recruiting_board"));
    REQUIRE(ContainsId(smokeIds, "step_rewards_system"));
    REQUIRE(ContainsId(smokeIds, "event_spawner_solution"));
    REQUIRE(ContainsId(smokeIds, "smart_event_timers_progress"));
    REQUIRE(ContainsId(smokeIds, "actor_construct"));
    REQUIRE(ContainsId(smokeIds, "equip_construct_visible_equipment"));
    REQUIRE(ContainsId(smokeIds, "skill_equip_system"));
    REQUIRE(ContainsId(smokeIds, "front_view_battler_mode"));
    REQUIRE(ContainsId(smokeIds, "quest"));
    REQUIRE(ContainsId(smokeIds, "skill_tree"));
    REQUIRE(ContainsId(smokeIds, "relationship"));
    REQUIRE(ContainsId(smokeIds, "cutscene_timeline"));
    REQUIRE(ContainsId(smokeIds, "encounter_designer"));
    REQUIRE(ContainsId(smokeIds, "loot_generator"));
    REQUIRE(ContainsId(smokeIds, "crafting"));
    REQUIRE(ContainsId(smokeIds, "monster_collection"));
    REQUIRE(ContainsId(smokeIds, "npc"));
    REQUIRE(ContainsId(smokeIds, "metroidvania_gates"));
    REQUIRE(ContainsId(smokeIds, "spatial_authoring"));
}
