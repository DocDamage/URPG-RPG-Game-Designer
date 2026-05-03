#include "apps/editor/editor_app_panels.h"

#include "engine/core/editor/editor_panel_registry.h"

#include <algorithm>

namespace urpg::editor_app {

std::vector<std::string> editorAppRegisteredPanelFactoryIds() {
    return {"diagnostics", "assets", "ability", "patterns", "mod", "analytics", "level_builder"};
}

std::vector<std::string> editorAppRegisteredNestedPanelFactoryIds() {
    std::vector<std::string> ids = {
        "compat_report",
        "event_authority",
        "battle_inspector",
        "menu_inspector",
        "menu_preview",
        "audio_inspector",
        "migration_wizard",
        "project_audit",
        "project_health",
        "elevation_brush",
        "terrain_brush",
        "region_rules",
        "procedural_map",
        "prop_placement",
        "map_ability_binding",
        "spatial_ability_canvas",
        "message_inspector",
        "event_authoring",
        "save_inspector",
        "spatial_authoring",
        "battle_presentation",
        "export_diagnostics",
        "3d_dungeon_world",
        "loot_generator",
        "monster_collection",
        "metroidvania_gates",
        "platformer_physics_lab",
        "side_view_action_combat_kit",
        "summon_gacha_banner_builder",
        "gacha_system",
        "achievement_visual_builder",
        "world_map_route_planner",
        "fast_travel_map_builder",
        "map_zoom_system",
        "farming_garden_plot_builder",
        "puzzle_logic_board",
        "horror_fx_builder",
        "showcase_affection",
        "showcase_aiming",
        "showcase_arena_movement",
        "showcase_arena_rules",
        "showcase_attacks",
        "showcase_auto_battle",
        "showcase_automap",
        "showcase_automation",
        "showcase_ball_physics",
        "showcase_biomes",
        "showcase_board_logic",
        "showcase_board_matching",
        "showcase_boss_phases",
        "showcase_branching_state",
        "showcase_brawler_combat",
        "showcase_bricks",
        "showcase_buildings",
        "showcase_builds",
        "showcase_bullets",
        "showcase_calendar",
        "showcase_camps",
        "showcase_card_battle",
        "showcase_cards",
        "showcase_care_schedule",
        "showcase_cascades",
        "showcase_case_graph",
        "showcase_categories",
        "showcase_charts",
        "showcase_checkpoint_save",
        "showcase_checkpoints",
        "showcase_choices",
        "showcase_citizens",
        "showcase_clubs",
        "showcase_collision",
        "showcase_colonist_stats",
        "showcase_combos",
        "showcase_companions",
        "showcase_crafting",
        "showcase_crafting_economy_loop",
        "showcase_crafting_stations",
        "showcase_crew",
        "showcase_crew_management",
        "showcase_customer_queue",
        "showcase_date_events",
        "showcase_deck_drafts",
        "showcase_dialogue_trees",
        "showcase_diplomacy",
        "showcase_districts",
        "showcase_dungeons",
        "showcase_economy",
        "showcase_encounter_tables",
        "showcase_ending_branches",
        "showcase_enemies",
        "showcase_evidence_dialogue",
        "showcase_faction_mission",
        "showcase_generated_towns",
        "showcase_generators",
        "showcase_goals",
        "showcase_grades",
        "showcase_grid_battle",
        "showcase_grid_movement",
        "showcase_grid_tactics",
        "showcase_guild_quests",
        "showcase_hand_management",
        "showcase_hazards",
        "showcase_hero_abilities",
        "showcase_hints",
        "showcase_hotspots",
        "showcase_inventory_items",
        "showcase_item_use",
        "showcase_jobs",
        "showcase_key_items",
        "showcase_keys",
        "showcase_lanes",
        "showcase_laps",
        "showcase_league_rewards",
        "showcase_license_rewards",
        "showcase_markets",
        "showcase_match_day",
        "showcase_matches",
        "showcase_mech_loadout",
        "showcase_minigames",
        "showcase_missions",
        "showcase_movement",
        "showcase_naval_encounters",
        "showcase_object_lists",
        "showcase_offline_rewards",
        "showcase_optimization",
        "showcase_paddle",
        "showcase_party_recruitment",
        "showcase_pet_roster",
        "showcase_pieces",
        "showcase_pilot_bonds",
        "showcase_placement",
        "showcase_portal_origin",
        "showcase_powerups",
        "showcase_prestige",
        "showcase_pricing",
        "showcase_procedural_dungeon",
        "showcase_procedural_dungeons",
        "showcase_puzzles",
        "showcase_quest_objective_graph",
        "showcase_questions",
        "showcase_quests",
        "showcase_queues",
        "showcase_raids",
        "showcase_recipes",
        "showcase_recruiting",
        "showcase_relationship_affinity",
        "showcase_relationships",
        "showcase_relics",
        "showcase_reputation",
        "showcase_resource_chains",
        "showcase_resources",
        "showcase_retries",
        "showcase_role_abilities",
        "showcase_room_puzzles",
        "showcase_rooms",
        "showcase_rounds",
        "showcase_route_map",
        "showcase_route_planner",
        "showcase_routes",
        "showcase_rows",
        "showcase_scavenge_rules",
        "showcase_scenes",
        "showcase_schedules",
        "showcase_scoring",
        "showcase_settlement_crafting",
        "showcase_shop_economy",
        "showcase_shop_requests",
        "showcase_simple_ai",
        "showcase_social_links",
        "showcase_staff",
        "showcase_stage_rewards",
        "showcase_stamina_combat",
        "showcase_starship_route",
        "showcase_stats",
        "showcase_switches",
        "showcase_team_roster",
        "showcase_territory",
        "showcase_timed_input",
        "showcase_timed_search",
        "showcase_timers",
        "showcase_timing_windows",
        "showcase_tower_grid",
        "showcase_towers",
        "showcase_tracks",
        "showcase_training",
        "showcase_traps",
        "showcase_treasure_routes",
        "showcase_turn_order",
        "showcase_upgrade_tree",
        "showcase_upgrades",
        "showcase_vehicles",
        "showcase_wasteland_biomes",
        "showcase_waves",
        "showcase_world_seed",
        "showcase_zones",
        "showcase_zoning",
    };
    return ids;
}

std::vector<std::string> editorAppRoutablePanelFactoryIds() {
    auto ids = editorAppRegisteredPanelFactoryIds();
    auto nested = editorAppRegisteredNestedPanelFactoryIds();
    ids.insert(ids.end(), nested.begin(), nested.end());
    std::sort(ids.begin(), ids.end());
    ids.erase(std::unique(ids.begin(), ids.end()), ids.end());
    return ids;
}

std::vector<std::string> editorAppMissingReleasePanelFactoryIds() {
    auto factoryIds = editorAppRegisteredPanelFactoryIds();
    std::sort(factoryIds.begin(), factoryIds.end());

    std::vector<std::string> missing;
    for (const auto& panelId : editor::requiredTopLevelPanelIds()) {
        if (!std::binary_search(factoryIds.begin(), factoryIds.end(), panelId)) {
            missing.push_back(panelId);
        }
    }
    return missing;
}

std::vector<std::string> editorAppMissingRoutablePanelFactoryIds() {
    auto factoryIds = editorAppRoutablePanelFactoryIds();

    std::vector<std::string> missing;
    for (const auto& entry : editor::editorPanelRegistry()) {
        if (!editor::isRoutableEditorPanelExposure(entry.exposure)) {
            continue;
        }
        if (!std::binary_search(factoryIds.begin(), factoryIds.end(), entry.id)) {
            missing.push_back(entry.id);
        }
    }
    return missing;
}

} // namespace urpg::editor_app
