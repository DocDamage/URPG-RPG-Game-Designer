#include "engine/core/project/template_certification.h"

#include "engine/core/project/template_runtime_profile.h"

#include <algorithm>
#include <array>

namespace {

std::string severityName(urpg::project::CertificationSeverity severity) {
    return severity == urpg::project::CertificationSeverity::Error ? "error" : "advisory";
}

const std::vector<urpg::project::TemplateCertificationSuite>& suites() {
    using urpg::project::CertificationRequirement;
    using urpg::project::TemplateCertificationSuite;

    static const std::vector<TemplateCertificationSuite> kSuites = {
        {"jrpg", "READY", {{"battle_loop", "Map-to-battle-to-reward loop"}, {"save_loop", "Save/load loop"}}},
        {"visual_novel", "READY", {{"dialogue_loop", "Dialogue choice loop"}, {"save_loop", "Save/load loop"}}},
        {"turn_based_rpg", "READY", {{"turn_based_battle_loop", "Turn-based battle loop"}, {"save_loop", "Save/load loop"}}},
        {"tactics_rpg", "READY", {{"tactical_battle_loop", "Grid tactics loop"}, {"scenario_authoring_loop", "Scenario authoring loop"}, {"save_loop", "Save/load loop"}}},
        {"arpg", "READY", {{"action_combat_loop", "Action combat loop"}, {"growth_loop", "Growth loop"}, {"save_loop", "Save/load loop"}}},
        {"monster_collector_rpg", "READY", {{"capture_loop", "Monster capture loop"}, {"party_assembly_loop", "Party assembly loop"}, {"battle_loop", "Battle loop"}, {"save_loop", "Save/load loop"}}},
        {"cozy_life_rpg", "READY", {{"daily_life_loop", "Daily life schedule loop"}, {"relationship_loop", "Relationship loop"}, {"crafting_loop", "Crafting loop"}, {"economy_loop", "Economy loop"}, {"save_loop", "Save/load loop"}}},
        {"metroidvania_lite", "READY", {{"ability_gate_loop", "Ability-gated exploration loop"}, {"map_unlock_loop", "Map unlock loop"}, {"traversal_loop", "Traversal loop"}, {"save_loop", "Save/load loop"}}},
        {"2_5d_rpg", "READY", {{"spatial_navigation_loop", "2.5D spatial navigation loop"}, {"raycast_authoring_loop", "Raycast authoring loop"}, {"save_loop", "Save/load loop"}}},
        {"roguelite_dungeon", "READY", {{"run_setup_loop", "Run setup loop"}, {"procedural_floor_loop", "Procedural floor loop"}, {"loot_reward_loop", "Loot reward loop"}, {"meta_progression_save_loop", "Meta-progression save loop"}}},
        {"survival_horror_rpg", "READY", {{"tension_exploration_loop", "Tension exploration loop"}, {"scarce_resource_loop", "Scarce resource loop"}, {"puzzle_unlock_loop", "Puzzle unlock loop"}, {"safe_room_save_loop", "Safe-room save loop"}}},
        {"farming_adventure_rpg", "READY", {{"crop_day_loop", "Crop day loop"}, {"forage_adventure_loop", "Forage adventure loop"}, {"crafting_economy_loop", "Crafting economy loop"}, {"relationship_save_loop", "Relationship save loop"}}},
        {"card_battler_rpg", "READY", {{"deck_build_loop", "Deck build loop"}, {"card_battle_loop", "Card battle loop"}, {"reward_draft_loop", "Reward draft loop"}, {"collection_save_loop", "Collection save loop"}}},
        {"platformer_rpg", "READY", {{"platformer_traversal_loop", "Platformer traversal loop"}, {"side_action_combat_loop", "Side-action combat loop"}, {"checkpoint_upgrade_loop", "Checkpoint upgrade loop"}, {"save_loop", "Save/load loop"}}},
        {"gacha_hero_rpg", "READY", {{"summon_banner_loop", "Summon banner loop"}, {"hero_roster_loop", "Hero roster loop"}, {"party_battle_loop", "Party battle loop"}, {"offline_pity_save_loop", "Offline pity save loop"}}},
        {"mystery_detective_rpg", "READY", {{"clue_board_loop", "Clue board loop"}, {"interrogation_dialogue_loop", "Interrogation dialogue loop"}, {"deduction_puzzle_loop", "Deduction puzzle loop"}, {"case_resolution_save_loop", "Case resolution save loop"}}},
        {"world_exploration_rpg", "READY", {{"world_route_loop", "World route loop"}, {"fast_travel_loop", "Fast travel loop"}, {"biome_event_loop", "Biome event loop"}, {"quest_journal_save_loop", "Quest journal save loop"}}},
        {"space_opera_rpg", "READY", {{"crew_management_loop", "Crew management loop"}, {"starship_travel_loop", "Starship travel loop"}, {"faction_mission_loop", "Faction mission loop"}, {"save_loop", "Save/load loop"}}},
        {"post_apocalyptic_rpg", "READY", {{"scavenge_route_loop", "Scavenge route loop"}, {"settlement_upgrade_loop", "Settlement upgrade loop"}, {"survival_resource_loop", "Survival resource loop"}, {"save_loop", "Save/load loop"}}},
        {"tactical_mecha_rpg", "READY", {{"mech_loadout_loop", "Mech loadout loop"}, {"grid_battle_loop", "Grid battle loop"}, {"pilot_bond_loop", "Pilot bond loop"}, {"save_loop", "Save/load loop"}}},
        {"monster_tamer_arena", "READY", {{"team_build_loop", "Team build loop"}, {"arena_battle_loop", "Arena battle loop"}, {"evolution_training_loop", "Evolution training loop"}, {"save_loop", "Save/load loop"}}},
        {"soulslike_lite_rpg", "READY", {{"stamina_combat_loop", "Stamina combat loop"}, {"checkpoint_recovery_loop", "Checkpoint recovery loop"}, {"boss_phase_loop", "Boss phase loop"}, {"save_loop", "Save/load loop"}}},
        {"idle_incremental_rpg", "READY", {{"offline_gain_loop", "Offline gain loop"}, {"auto_battle_loop", "Auto battle loop"}, {"prestige_upgrade_loop", "Prestige upgrade loop"}, {"save_loop", "Save/load loop"}}},
        {"strategy_kingdom_rpg", "READY", {{"kingdom_build_loop", "Kingdom build loop"}, {"recruitment_loop", "Recruitment loop"}, {"army_mission_loop", "Army mission loop"}, {"save_loop", "Save/load loop"}}},
        {"racing_adventure_rpg", "READY", {{"vehicle_upgrade_loop", "Vehicle upgrade loop"}, {"track_event_loop", "Track event loop"}, {"license_progression_loop", "License progression loop"}, {"save_loop", "Save/load loop"}}},
        {"rhythm_rpg", "READY", {{"song_chart_loop", "Song chart loop"}, {"timed_battle_loop", "Timed battle loop"}, {"performance_grade_loop", "Performance grade loop"}, {"save_loop", "Save/load loop"}}},
        {"cooking_restaurant_rpg", "READY", {{"recipe_prep_loop", "Recipe prep loop"}, {"customer_queue_loop", "Customer queue loop"}, {"restaurant_upgrade_loop", "Restaurant upgrade loop"}, {"save_loop", "Save/load loop"}}},
        {"school_life_rpg", "READY", {{"class_calendar_loop", "Class calendar loop"}, {"club_activity_loop", "Club activity loop"}, {"social_link_loop", "Social link loop"}, {"save_loop", "Save/load loop"}}},
        {"pirate_rpg", "READY", {{"crew_ship_loop", "Crew ship loop"}, {"island_treasure_loop", "Island treasure loop"}, {"naval_encounter_loop", "Naval encounter loop"}, {"save_loop", "Save/load loop"}}},
        {"sports_team_rpg", "READY", {{"team_roster_loop", "Team roster loop"}, {"training_week_loop", "Training week loop"}, {"match_day_loop", "Match day loop"}, {"save_loop", "Save/load loop"}}},
        {"pet_shop_creature_care_rpg", "READY", {{"pet_care_loop", "Pet care loop"}, {"breeding_loop", "Breeding loop"}, {"customer_request_loop", "Customer request loop"}, {"save_loop", "Save/load loop"}}},
        {"detective_noir_vn_rpg", "READY", {{"noir_case_loop", "Noir case loop"}, {"evidence_board_loop", "Evidence board loop"}, {"branching_ending_loop", "Branching ending loop"}, {"save_loop", "Save/load loop"}}},
        {"city_builder_rpg", "READY", {{"district_build_loop", "District build loop"}, {"citizen_need_loop", "Citizen need loop"}, {"resource_chain_loop", "Resource chain loop"}, {"save_loop", "Save/load loop"}}},
        {"tower_defense_rpg", "READY", {{"wave_design_loop", "Wave design loop"}, {"tower_upgrade_loop", "Tower upgrade loop"}, {"hero_defense_loop", "Hero defense loop"}, {"save_loop", "Save/load loop"}}},
        {"beat_em_up_rpg", "READY", {{"stage_brawler_loop", "Stage brawler loop"}, {"combo_progression_loop", "Combo progression loop"}, {"pickup_reward_loop", "Pickup reward loop"}, {"save_loop", "Save/load loop"}}},
        {"open_world_survival_rpg", "READY", {{"camp_setup_loop", "Camp setup loop"}, {"survival_needs_loop", "Survival needs loop"}, {"biome_event_loop", "Biome event loop"}, {"save_loop", "Save/load loop"}}},
        {"faction_politics_rpg", "READY", {{"reputation_choice_loop", "Reputation choice loop"}, {"diplomacy_mission_loop", "Diplomacy mission loop"}, {"territory_influence_loop", "Territory influence loop"}, {"save_loop", "Save/load loop"}}},
    };
    return kSuites;
}

} // namespace

namespace urpg::project {

nlohmann::json CertificationReport::toJson() const {
    nlohmann::json issuesJson = nlohmann::json::array();
    for (const auto& issue : issues) {
        issuesJson.push_back({
            {"severity", severityName(issue.severity)},
            {"code", issue.code},
            {"detail", issue.detail},
        });
    }

    return {
        {"schema", "urpg.template_certification.v1"},
        {"templateId", templateId},
        {"passed", passed},
        {"issues", issuesJson},
    };
}

std::vector<TemplateCertificationSuite> TemplateCertification::defaultSuites() const {
    return suites();
}

CertificationReport TemplateCertification::certify(const nlohmann::json& projectDocument,
                                                   const std::string& templateId) const {
    CertificationReport report;
    report.templateId = templateId;
    const auto* suite = findSuite(templateId);
    if (!suite) {
        report.issues.push_back({CertificationSeverity::Error, "unknown_template", "No certification suite exists for template."});
        return report;
    }

    for (const auto& requirement : suite->requirements) {
        if (requirement.optional && projectDisablesOptionalFeature(projectDocument, requirement.id)) {
            continue;
        }

        if (!projectHasLoop(projectDocument, requirement.id)) {
            report.issues.push_back({
                requirement.optional ? CertificationSeverity::Advisory : CertificationSeverity::Error,
                "missing_required_loop",
                "Template '" + templateId + "' is missing loop '" + requirement.id + "'.",
            });
        }
    }

    const auto profile = findTemplateRuntimeProfile(templateId);
    if (!profile.has_value()) {
        report.issues.push_back({CertificationSeverity::Error, "missing_runtime_profile",
                                 "Template runtime profile is missing."});
    } else {
        for (const auto& issue : validateTemplateRuntimeProfile(*profile)) {
            report.issues.push_back({CertificationSeverity::Error, "invalid_runtime_profile", issue});
        }
        if (projectDocument.contains("template_bars") && projectDocument["template_bars"].is_object()) {
            for (const auto& [bar, value] : profile->bars.items()) {
                if (!projectDocument["template_bars"].contains(bar) ||
                    projectDocument["template_bars"][bar].value("status", "") != "READY") {
                    report.issues.push_back({CertificationSeverity::Error, "template_bar_not_ready",
                                             "Template '" + templateId + "' has non-ready bar '" + bar + "'."});
                }
            }
        }
    }

    report.passed = std::none_of(report.issues.begin(), report.issues.end(), [](const CertificationIssue& issue) {
        return issue.severity == CertificationSeverity::Error;
    });
    return report;
}

const TemplateCertificationSuite* TemplateCertification::findSuite(const std::string& templateId) const {
    const auto& allSuites = suites();
    const auto it = std::find_if(allSuites.begin(), allSuites.end(), [&](const TemplateCertificationSuite& suite) {
        return suite.templateId == templateId;
    });
    return it == allSuites.end() ? nullptr : &*it;
}

bool TemplateCertification::projectHasLoop(const nlohmann::json& projectDocument, const std::string& loopId) const {
    if (!projectDocument.contains("loops") || !projectDocument["loops"].is_array()) {
        return false;
    }

    for (const auto& loop : projectDocument["loops"]) {
        if (loop.is_string() && loop.get<std::string>() == loopId) {
            return true;
        }
        if (loop.is_object() && loop.value("id", "") == loopId && loop.value("implemented", true)) {
            return true;
        }
    }
    return false;
}

bool TemplateCertification::projectDisablesOptionalFeature(const nlohmann::json& projectDocument,
                                                           const std::string& featureId) const {
    if (!projectDocument.contains("disabledOptionalFeatures") || !projectDocument["disabledOptionalFeatures"].is_array()) {
        return false;
    }

    for (const auto& feature : projectDocument["disabledOptionalFeatures"]) {
        if (feature.is_string() && feature.get<std::string>() == featureId) {
            return true;
        }
    }
    return false;
}

} // namespace urpg::project
