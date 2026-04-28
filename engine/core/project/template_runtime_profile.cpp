#include "engine/core/project/template_runtime_profile.h"

#include <algorithm>
#include <array>
#include <utility>

namespace urpg::project {

namespace {

nlohmann::json readyBars(std::vector<std::string> evidence) {
    return {
        {"accessibility", {{"status", "READY"}, {"evidence", evidence}}},
        {"audio", {{"status", "READY"}, {"evidence", evidence}}},
        {"input", {{"status", "READY"}, {"evidence", evidence}}},
        {"localization", {{"status", "READY"}, {"evidence", evidence}}},
        {"performance", {{"status", "READY"}, {"evidence", evidence}}},
    };
}

TemplateRuntimeProfile makeTacticsProfile() {
    return {
        "tactics_rpg",
        "Tactics RPG",
        "READY",
        {"grid", "scenario", "turn-order", "battle"},
        {"battle_core", "presentation_runtime", "save_data_core", "accessibility_auditor", "audio_mix_presets"},
        {"tactical_battle_loop", "scenario_authoring_loop", "save_loop"},
        readyBars({"tactics_grid_labels", "tactical_audio_cues", "scenario_localization_keys", "turn_budget_16ms"}),
        {
            {"scenarioAuthoring", {
                {"grid", {{"width", 12}, {"height", 10}, {"layers", {"terrain", "height", "deployment"}}}},
                {"deploymentZones", {
                    {{"id", "player_start"}, {"team", "player"}, {"cells", {{{"x", 1}, {"y", 5}}, {{"x", 1}, {"y", 6}}}}},
                    {{"id", "enemy_start"}, {"team", "enemy"}, {"cells", {{{"x", 10}, {"y", 4}}, {{"x", 10}, {"y", 5}}}}},
                }},
                {"turnOrder", "speed_then_team_priority"},
                {"winConditions", {"defeat_all_enemies", "survive_turns"}},
            }},
            {"accessibility", {{"focusLabels", {"unit_roster", "grid_cursor", "action_menu"}}, {"contrastMode", "tactics_grid"}}},
            {"audio", {{"cues", {"unit_select", "move_confirm", "attack_preview", "turn_start"}}}},
            {"localization", {{"requiredKeys", {"scenario.title", "objective.primary", "action.move", "action.attack"}}}},
        },
    };
}

TemplateRuntimeProfile makeArpgProfile() {
    return {
        "arpg",
        "Action RPG",
        "READY",
        {"action", "combat", "growth", "real-time"},
        {"presentation_runtime", "save_data_core", "gameplay_ability_framework", "input_runtime"},
        {"action_combat_loop", "growth_loop", "save_loop"},
        readyBars({"arpg_combat_labels", "realtime_audio_cues", "action_input_map", "combat_text_keys", "frame_budget_16ms"}),
        {
            {"combat", {
                {"states", {"idle", "attacking", "dodging", "recoiling"}},
                {"actions", {"light_attack", "heavy_attack", "dodge", "interact", "quick_item"}},
                {"stamina", {{"max", 100}, {"dodgeCost", 20}, {"recoveryPerSecond", 10}}},
                {"closureVisibility", {{"hitFlash", true}, {"dodgeIFrames", true}, {"damageNumbers", true}}},
            }},
            {"accessibility", {{"requiredLabels", {"health_bar", "stamina_bar", "quick_slot", "target_lock"}}}},
            {"audio", {{"cues", {"swing_light", "swing_heavy", "dodge", "hit_confirm", "low_health"}}}},
            {"localization", {{"requiredKeys", {"combat.dodge", "combat.quick_item", "prompt.interact"}}}},
        },
    };
}

TemplateRuntimeProfile makeMonsterCollectorProfile() {
    return {
        "monster_collector_rpg",
        "Monster Collector RPG",
        "READY",
        {"collection", "party", "capture", "battle"},
        {"ui_menu_core", "message_text_core", "battle_core", "save_data_core", "gameplay_ability_framework"},
        {"capture_loop", "party_assembly_loop", "battle_loop", "save_loop"},
        readyBars({"collection_roster_labels", "capture_audio_cues", "party_input_map", "creature_text_keys", "encounter_budget"}),
        {
            {"collection", {
                {"species", {
                    {{"id", "sproutling"}, {"element", "nature"}, {"captureRate", 0.72}},
                    {{"id", "cinderpup"}, {"element", "fire"}, {"captureRate", 0.55}},
                    {{"id", "tidalisk"}, {"element", "water"}, {"captureRate", 0.60}},
                }},
                {"party", {{"maxActive", 4}, {"reserveLimit", 64}}},
                {"capture", {{"formula", "base_rate + status_bonus - rarity_penalty"}, {"items", {"basic_charm", "strong_charm"}}}},
            }},
            {"accessibility", {{"requiredLabels", {"creature_roster", "capture_button", "party_slot", "ability_grid"}}}},
            {"audio", {{"cues", {"encounter_start", "capture_throw", "capture_success", "capture_breakout"}}}},
            {"localization", {{"requiredKeys", {"creature.name", "capture.success", "capture.failed", "party.swap"}}}},
        },
    };
}

TemplateRuntimeProfile makeCozyLifeProfile() {
    return {
        "cozy_life_rpg",
        "Cozy Life RPG",
        "READY",
        {"life-sim", "schedule", "social", "crafting", "economy"},
        {"ui_menu_core", "message_text_core", "save_data_core", "crafting", "economy", "shop"},
        {"daily_life_loop", "relationship_loop", "crafting_loop", "economy_loop", "save_loop"},
        readyBars({"schedule_labels", "ambient_audio_cues", "life_sim_input_map", "social_text_keys", "day_tick_budget"}),
        {
            {"schedule", {
                {"dayPhases", {"morning", "afternoon", "evening", "night"}},
                {"activities", {"forage", "craft", "shop", "gift", "rest"}},
                {"npcRoutines", {{{"npc", "Mira"}, {"morning", "market"}, {"evening", "home"}}}},
            }},
            {"relationships", {{"levels", {"stranger", "friend", "trusted", "bonded"}}, {"giftCooldownDays", 1}}},
            {"crafting", {{"starterRecipes", {"tea_blend", "flower_bundle", "wooden_charm"}}}},
            {"economy", {{"startingGold", 120}, {"vendors", {"general_store", "seed_cart"}}}},
            {"accessibility", {{"requiredLabels", {"calendar", "relationship_log", "recipe_list", "vendor_stock"}}}},
            {"audio", {{"cues", {"day_start", "craft_complete", "friendship_up", "shop_open"}}}},
            {"localization", {{"requiredKeys", {"day.morning", "activity.craft", "relationship.gift", "shop.buy"}}}},
        },
    };
}

TemplateRuntimeProfile makeMetroidvaniaProfile() {
    return {
        "metroidvania_lite",
        "Metroidvania Lite",
        "READY",
        {"exploration", "traversal", "ability-gates", "map"},
        {"presentation_runtime", "save_data_core", "gameplay_ability_framework", "map_runtime"},
        {"ability_gate_loop", "map_unlock_loop", "traversal_loop", "save_loop"},
        readyBars({"traversal_labels", "ability_audio_cues", "platforming_input_map", "region_text_keys", "room_budget"}),
        {
            {"traversal", {
                {"abilities", {
                    {{"id", "dash"}, {"unlocks", {"gap_small", "timed_gate"}}},
                    {{"id", "wall_jump"}, {"unlocks", {"vertical_shaft"}}},
                    {{"id", "double_jump"}, {"unlocks", {"high_ledge"}}},
                }},
                {"regions", {
                    {{"id", "old_well"}, {"requires", nlohmann::json::array()}},
                    {{"id", "clock_tower"}, {"requires", {"wall_jump"}}},
                    {{"id", "sky_bridge"}, {"requires", {"dash", "double_jump"}}},
                }},
            }},
            {"accessibility", {{"requiredLabels", {"map_region", "ability_gate", "checkpoint", "upgrade_pickup"}}}},
            {"audio", {{"cues", {"ability_unlock", "gate_open", "checkpoint", "low_health"}}}},
            {"localization", {{"requiredKeys", {"ability.dash", "ability.wall_jump", "region.locked", "map.checkpoint"}}}},
        },
    };
}

TemplateRuntimeProfile makeTwoPointFiveDProfile() {
    return {
        "2_5d_rpg",
        "2.5D RPG",
        "READY",
        {"spatial", "raycast", "exploration", "rpg"},
        {"presentation_runtime", "save_data_core", "raycast_renderer", "spatial_projection"},
        {"spatial_navigation_loop", "raycast_authoring_loop", "save_loop"},
        readyBars({"spatial_labels", "spatial_audio_cues", "raycast_input_map", "area_text_keys", "raycast_budget"}),
        {
            {"raycast", {
                {"screen", {{"width", 640}, {"height", 480}, {"fov", 0.66}}},
                {"authoringAdapter", {{"blockingFromElevation", true}, {"spawnValidation", true}}},
                {"exportValidation", {"map_has_blocking_cells", "camera_spawn_not_blocked", "raycast_budget_defined"}},
            }},
            {"accessibility", {{"requiredLabels", {"depth_cue", "navigation_prompt", "interact_target", "minimap"}}}},
            {"audio", {{"cues", {"footstep_near", "door_open", "spatial_ambience", "interact_prompt"}}}},
            {"localization", {{"requiredKeys", {"area.name", "prompt.forward", "prompt.turn", "prompt.interact"}}}},
        },
    };
}

TemplateRuntimeProfile makeBasicProfile(std::string id,
                                        std::string displayName,
                                        std::vector<std::string> tags,
                                        std::vector<std::string> subsystems,
                                        std::vector<std::string> loops) {
    return {std::move(id),
            std::move(displayName),
            "READY",
            std::move(tags),
            std::move(subsystems),
            std::move(loops),
            readyBars({"baseline_accessibility", "baseline_audio", "baseline_input", "baseline_localization", "baseline_perf"}),
            nlohmann::json::object()};
}

} // namespace

std::vector<TemplateRuntimeProfile> allTemplateRuntimeProfiles() {
    return {
        makeBasicProfile("jrpg",
                         "Classic JRPG",
                         {"party", "battle", "exploration"},
                         {"ui_menu_core", "message_text_core", "battle_core", "save_data_core"},
                         {"battle_loop", "save_loop"}),
        makeBasicProfile("visual_novel",
                         "Visual Novel",
                         {"dialogue", "choices", "save"},
                         {"message_text_core", "save_data_core"},
                         {"dialogue_loop", "save_loop"}),
        makeBasicProfile("turn_based_rpg",
                         "Turn-Based RPG",
                         {"tactics", "battle", "progression"},
                         {"message_text_core", "battle_core", "save_data_core"},
                         {"turn_based_battle_loop", "save_loop"}),
        makeTacticsProfile(),
        makeArpgProfile(),
        makeMonsterCollectorProfile(),
        makeCozyLifeProfile(),
        makeMetroidvaniaProfile(),
        makeTwoPointFiveDProfile(),
    };
}

std::optional<TemplateRuntimeProfile> findTemplateRuntimeProfile(const std::string& templateId) {
    const auto profiles = allTemplateRuntimeProfiles();
    const auto it = std::find_if(profiles.begin(), profiles.end(), [&](const TemplateRuntimeProfile& profile) {
        return profile.id == templateId;
    });
    if (it == profiles.end()) {
        return std::nullopt;
    }
    return *it;
}

nlohmann::json templateRuntimeProfileToJson(const TemplateRuntimeProfile& profile) {
    return {
        {"id", profile.id},
        {"displayName", profile.displayName},
        {"status", profile.status},
        {"tags", profile.tags},
        {"requiredSubsystems", profile.requiredSubsystems},
        {"loops", profile.loops},
        {"bars", profile.bars},
        {"systems", profile.systems},
    };
}

std::vector<std::string> validateTemplateRuntimeProfile(const TemplateRuntimeProfile& profile) {
    std::vector<std::string> issues;
    if (profile.id.empty()) {
        issues.push_back("missing_template_id");
    }
    if (profile.displayName.empty()) {
        issues.push_back("missing_display_name");
    }
    if (profile.requiredSubsystems.empty()) {
        issues.push_back("missing_required_subsystems");
    }
    if (profile.loops.empty()) {
        issues.push_back("missing_template_loops");
    }
    static const std::array<const char*, 5> kBars = {"accessibility", "audio", "input", "localization", "performance"};
    for (const auto* bar : kBars) {
        if (!profile.bars.contains(bar)) {
            issues.push_back(std::string("missing_bar:") + bar);
            continue;
        }
        if (profile.bars[bar].value("status", "") != "READY") {
            issues.push_back(std::string("bar_not_ready:") + bar);
        }
    }
    return issues;
}

} // namespace urpg::project
