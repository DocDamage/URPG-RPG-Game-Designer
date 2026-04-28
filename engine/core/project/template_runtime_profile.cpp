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

TemplateRuntimeProfile makeRogueliteDungeonProfile() {
    return {
        "roguelite_dungeon",
        "Roguelite Dungeon",
        "READY",
        {"procedural", "dungeon", "loot", "runs"},
        {"presentation_runtime", "battle_core", "save_data_core", "gameplay_ability_framework", "export_validator"},
        {"run_setup_loop", "procedural_floor_loop", "loot_reward_loop", "meta_progression_save_loop"},
        readyBars({"floor_seed_labels", "run_audio_cues", "dungeon_input_map", "run_text_keys", "floor_budget"}),
        {
            {"run", {
                {"seedMode", "deterministic_project_seed"},
                {"floorCount", 4},
                {"roomBudget", {{"min", 8}, {"max", 14}}},
                {"failurePolicy", "return_to_hub_with_meta_rewards"},
            }},
            {"proceduralDungeon", {
                {"roomTypes", {"start", "combat", "event", "treasure", "boss", "exit"}},
                {"stitcher", "random_dungeon_room_stitcher"},
                {"spawnTables", {"floor_1_slimes", "floor_2_constructs", "floor_3_wraiths"}},
                {"bossArena", {{"required", true}, {"roomType", "boss"}}},
            }},
            {"loot", {
                {"rarities", {"common", "rare", "epic"}},
                {"rewardPity", {{"enabled", true}, {"maxDryRooms", 3}}},
                {"exportValidation", {"loot_table_has_weights", "boss_drop_defined", "run_seed_recorded"}},
            }},
            {"accessibility", {{"requiredLabels", {"run_seed", "floor_depth", "loot_choice", "exit_portal"}}}},
            {"audio", {{"cues", {"run_start", "room_clear", "loot_drop", "boss_warning"}}}},
            {"localization", {{"requiredKeys", {"run.start", "loot.choose", "floor.depth", "boss.warning"}}}},
        },
    };
}

TemplateRuntimeProfile makeSurvivalHorrorProfile() {
    return {
        "survival_horror_rpg",
        "Survival Horror RPG",
        "READY",
        {"horror", "resources", "lighting", "puzzles"},
        {"presentation_runtime", "message_text_core", "battle_core", "save_data_core", "accessibility_auditor"},
        {"tension_exploration_loop", "scarce_resource_loop", "puzzle_unlock_loop", "safe_room_save_loop"},
        readyBars({"horror_readability_labels", "tension_audio_cues", "resource_input_map", "puzzle_text_keys", "lighting_budget"}),
        {
            {"tension", {
                {"visibility", {{"flashlightConeDegrees", 55}, {"ambientFloor", 0.18}}},
                {"resourcePressure", {{"ammo", 12}, {"healing", 2}, {"batterySeconds", 180}}},
                {"safeRooms", {"lobby_save_room", "archive_save_room"}},
            }},
            {"puzzles", {
                {"locks", {
                    {{"id", "west_wing_emblem"}, {"requires", {"bronze_emblem"}}},
                    {{"id", "archive_terminal"}, {"requires", {"power_restored", "clerk_code"}}},
                }},
                {"diagnostics", {"missing_clue_link", "unreachable_safe_room", "resource_softlock_risk"}},
            }},
            {"accessibility", {{"requiredLabels", {"flashlight_meter", "inventory_slots", "safe_room_prompt", "threat_warning"}}}},
            {"audio", {{"cues", {"heartbeat_low_health", "distant_threat", "safe_room_theme", "puzzle_unlock"}}}},
            {"localization", {{"requiredKeys", {"item.battery", "prompt.hide", "safe_room.save", "puzzle.locked"}}}},
        },
    };
}

TemplateRuntimeProfile makeFarmingAdventureProfile() {
    return {
        "farming_adventure_rpg",
        "Farming Adventure RPG",
        "READY",
        {"farming", "crafting", "relationships", "adventure"},
        {"ui_menu_core", "message_text_core", "save_data_core", "gameplay_ability_framework", "export_validator"},
        {"crop_day_loop", "forage_adventure_loop", "crafting_economy_loop", "relationship_save_loop"},
        readyBars({"farm_tool_labels", "season_audio_cues", "farm_input_map", "relationship_text_keys", "day_budget"}),
        {
            {"farming", {
                {"seasons", {"spring", "summer", "autumn", "winter"}},
                {"starterCrops", {
                    {{"id", "turnip"}, {"days", 3}, {"season", "spring"}},
                    {{"id", "tomato"}, {"days", 5}, {"season", "summer"}},
                    {{"id", "moonroot"}, {"days", 7}, {"season", "autumn"}},
                }},
                {"weatherEffects", {{"rainWatersCrops", true}, {"stormDamagesChance", 0.08}}},
            }},
            {"adventure", {
                {"forageZones", {"meadow", "old_mine", "spirit_grove"}},
                {"toolGates", {
                    {{"tool", "axe"}, {"unlocks", {"fallen_log_path"}}},
                    {{"tool", "hammer"}, {"unlocks", {"old_mine_floor_2"}}},
                }},
            }},
            {"economy", {{"shippingBin", true}, {"shopRestockDays", {"monday", "thursday"}}}},
            {"relationships", {{"heartLevels", 10}, {"giftMemory", true}}},
            {"accessibility", {{"requiredLabels", {"crop_stage", "tool_slot", "weather_badge", "relationship_heart"}}}},
            {"audio", {{"cues", {"crop_water", "harvest", "shop_open", "heart_up"}}}},
            {"localization", {{"requiredKeys", {"crop.ready", "tool.upgrade", "weather.rain", "friend.gift"}}}},
        },
    };
}

TemplateRuntimeProfile makeCardBattlerProfile() {
    return {
        "card_battler_rpg",
        "Card Battler RPG",
        "READY",
        {"cards", "deckbuilding", "battle", "collection"},
        {"ui_menu_core", "battle_core", "save_data_core", "gameplay_ability_framework", "export_validator"},
        {"deck_build_loop", "card_battle_loop", "reward_draft_loop", "collection_save_loop"},
        readyBars({"card_readability_labels", "card_audio_cues", "deck_input_map", "card_text_keys", "turn_budget"}),
        {
            {"cards", {
                {"deckSize", {{"min", 12}, {"max", 30}}},
                {"handSize", 5},
                {"resources", {"energy", "focus"}},
                {"starterDeck", {"strike", "guard", "spark", "draw"}},
            }},
            {"battle", {
                {"turnStructure", {"draw", "play", "enemy_intent", "resolve"}},
                {"visibleIntent", true},
                {"diagnostics", {"deck_too_small", "unpayable_card", "missing_reward_pool"}},
            }},
            {"collection", {
                {"rarities", {"common", "uncommon", "rare", "legendary"}},
                {"rewardDraftChoices", 3},
                {"exportValidation", {"starter_deck_valid", "reward_pool_weighted", "card_text_localized"}},
            }},
            {"accessibility", {{"requiredLabels", {"card_name", "card_cost", "enemy_intent", "draw_pile_count"}}}},
            {"audio", {{"cues", {"card_draw", "card_play", "block_gain", "enemy_intent"}}}},
            {"localization", {{"requiredKeys", {"card.strike", "card.guard", "battle.intent", "reward.choose"}}}},
        },
    };
}

TemplateRuntimeProfile makePlatformerRpgProfile() {
    return {
        "platformer_rpg",
        "Platformer RPG",
        "READY",
        {"platformer", "traversal", "action", "side-view"},
        {"presentation_runtime", "battle_core", "save_data_core", "gameplay_ability_framework", "export_validator"},
        {"platformer_traversal_loop", "side_action_combat_loop", "checkpoint_upgrade_loop", "save_loop"},
        readyBars({"platformer_labels", "jump_audio_cues", "platformer_input_map", "checkpoint_text_keys", "physics_budget"}),
        {
            {"platformer", {
                {"physics", {{"gravity", 38.0}, {"jumpVelocity", 14.5}, {"coyoteMs", 100}, {"bufferMs", 120}}},
                {"abilities", {"dash", "wall_slide", "double_jump"}},
                {"checkpoints", {"start_cliff", "bridge_midpoint", "tower_gate"}},
                {"diagnostics", {"unreachable_pickup", "unsafe_spawn", "missing_checkpoint_after_gate"}},
            }},
            {"combat", {
                {"mode", "side_action"},
                {"actions", {"jump_attack", "dash_strike", "air_skill"}},
            }},
            {"accessibility", {{"requiredLabels", {"jump_prompt", "checkpoint", "hazard_warning", "ability_gate"}}}},
            {"audio", {{"cues", {"jump", "land", "checkpoint", "hazard_hit"}}}},
            {"localization", {{"requiredKeys", {"platform.jump", "checkpoint.saved", "hazard.warning", "ability.dash"}}}},
        },
    };
}

TemplateRuntimeProfile makeGachaHeroProfile() {
    return {
        "gacha_hero_rpg",
        "Gacha Hero RPG",
        "READY",
        {"collection", "summon", "party", "liveops-local"},
        {"ui_menu_core", "message_text_core", "battle_core", "save_data_core", "export_validator"},
        {"summon_banner_loop", "hero_roster_loop", "party_battle_loop", "offline_pity_save_loop"},
        readyBars({"summon_labels", "summon_audio_cues", "roster_input_map", "banner_text_keys", "simulation_budget"}),
        {
            {"summon", {
                {"currency", "star_gems"},
                {"rarities", {"r", "sr", "ssr"}},
                {"pity", {{"ssrMaxPulls", 90}, {"guaranteedFeaturedAfterMiss", true}}},
                {"bannerValidation", {"rates_sum_to_one", "featured_unit_exists", "pity_state_saved"}},
            }},
            {"heroes", {
                {"starterRoster", {"aura_knight", "cinder_mage", "tide_mender"}},
                {"partySize", 4},
                {"duplicatesConvertTo", "hero_shards"},
            }},
            {"accessibility", {{"requiredLabels", {"summon_button", "rate_details", "pity_counter", "hero_roster_slot"}}}},
            {"audio", {{"cues", {"summon_start", "rarity_reveal", "ssr_pull", "party_ready"}}}},
            {"localization", {{"requiredKeys", {"summon.pull", "rates.details", "hero.roster", "pity.counter"}}}},
        },
    };
}

TemplateRuntimeProfile makeMysteryDetectiveProfile() {
    return {
        "mystery_detective_rpg",
        "Mystery Detective RPG",
        "READY",
        {"mystery", "dialogue", "clues", "quests"},
        {"ui_menu_core", "message_text_core", "save_data_core", "gameplay_ability_framework", "export_validator"},
        {"clue_board_loop", "interrogation_dialogue_loop", "deduction_puzzle_loop", "case_resolution_save_loop"},
        readyBars({"clue_labels", "investigation_audio_cues", "case_input_map", "deduction_text_keys", "case_graph_budget"}),
        {
            {"casework", {
                {"clueTypes", {"evidence", "testimony", "location", "contradiction"}},
                {"deductions", {
                    {{"id", "clock_tower_alibi"}, {"requires", {"broken_watch", "guard_testimony"}}},
                    {{"id", "library_secret"}, {"requires", {"ash_trace", "hidden_key"}}},
                }},
                {"diagnostics", {"unreachable_clue", "dead_end_case", "missing_resolution_path"}},
            }},
            {"dialogue", {
                {"interrogationModes", {"friendly", "press", "present_evidence"}},
                {"variablePreview", true},
            }},
            {"accessibility", {{"requiredLabels", {"clue_card", "case_thread", "evidence_prompt", "deduction_result"}}}},
            {"audio", {{"cues", {"clue_found", "contradiction", "case_update", "case_solved"}}}},
            {"localization", {{"requiredKeys", {"clue.found", "case.thread", "prompt.present", "case.solved"}}}},
        },
    };
}

TemplateRuntimeProfile makeWorldExplorationProfile() {
    return {
        "world_exploration_rpg",
        "World Exploration RPG",
        "READY",
        {"world-map", "travel", "quests", "biomes"},
        {"presentation_runtime", "message_text_core", "save_data_core", "gameplay_ability_framework", "export_validator"},
        {"world_route_loop", "fast_travel_loop", "biome_event_loop", "quest_journal_save_loop"},
        readyBars({"world_map_labels", "travel_audio_cues", "route_input_map", "quest_text_keys", "streaming_budget"}),
        {
            {"world", {
                {"biomes", {"grassland", "snowfield", "desert", "coast"}},
                {"routes", {"capital_to_forest", "forest_to_mine", "mine_to_coast"}},
                {"fastTravel", {{"requiresDiscovery", true}, {"costMode", "none"}}},
                {"diagnostics", {"route_disconnected", "undiscoverable_landmark", "quest_marker_missing"}},
            }},
            {"quests", {
                {"journalCategories", {"main", "side", "rumor"}},
                {"objectiveRadar", true},
            }},
            {"accessibility", {{"requiredLabels", {"world_marker", "route_node", "biome_badge", "quest_objective"}}}},
            {"audio", {{"cues", {"route_select", "fast_travel", "biome_enter", "quest_update"}}}},
            {"localization", {{"requiredKeys", {"world.marker", "travel.fast", "biome.name", "quest.objective"}}}},
        },
    };
}

TemplateRuntimeProfile makeGenreProfile(std::string id,
                                        std::string displayName,
                                        std::vector<std::string> tags,
                                        std::vector<std::string> loops,
                                        std::string primarySystem,
                                        std::vector<std::string> featureList) {
    const std::string prefix = id;
    return {
        std::move(id),
        std::move(displayName),
        "READY",
        std::move(tags),
        {"ui_menu_core", "message_text_core", "battle_core", "save_data_core", "export_validator"},
        std::move(loops),
        readyBars({prefix + "_labels", prefix + "_audio_cues", prefix + "_input_map", prefix + "_text_keys",
                   prefix + "_budget"}),
        {
            {primarySystem, {
                {"features", featureList},
                {"diagnostics", {"missing_required_loop", "unreachable_starter_state", "export_validation_failed"}},
                {"exportValidation", {"starter_data_complete", "required_text_keys_bound", "save_state_declared"}},
            }},
            {"accessibility", {{"requiredLabels", {prefix + "_primary", prefix + "_status", prefix + "_action", prefix + "_warning"}}}},
            {"audio", {{"cues", {prefix + "_open", prefix + "_confirm", prefix + "_success", prefix + "_warning"}}}},
            {"localization", {{"requiredKeys", {prefix + ".title", prefix + ".action", prefix + ".status", prefix + ".warning"}}}},
        },
    };
}

TemplateRuntimeProfile makeSpaceOperaProfile() {
    return makeGenreProfile("space_opera_rpg", "Space Opera RPG", {"space", "crew", "factions", "travel"},
                            {"crew_management_loop", "starship_travel_loop", "faction_mission_loop", "save_loop"},
                            "spaceOpera", {"crew_roster", "ship_travel", "planet_landings", "faction_reputation"});
}

TemplateRuntimeProfile makePostApocalypticProfile() {
    return makeGenreProfile("post_apocalyptic_rpg", "Post-Apocalyptic RPG", {"survival", "scavenging", "settlements"},
                            {"scavenge_route_loop", "settlement_upgrade_loop", "survival_resource_loop", "save_loop"},
                            "wasteland", {"scavenging", "settlements", "survival_resources", "faction_zones"});
}

TemplateRuntimeProfile makeTacticalMechaProfile() {
    return makeGenreProfile("tactical_mecha_rpg", "Tactical Mecha RPG", {"mecha", "tactics", "loadouts"},
                            {"mech_loadout_loop", "grid_battle_loop", "pilot_bond_loop", "save_loop"},
                            "mecha", {"parts_loadout", "heat_ammo", "pilot_bonds", "grid_deployment"});
}

TemplateRuntimeProfile makeMonsterTamerArenaProfile() {
    return makeGenreProfile("monster_tamer_arena", "Monster Tamer Arena", {"monster", "arena", "tournament"},
                            {"team_build_loop", "arena_battle_loop", "evolution_training_loop", "save_loop"},
                            "arena", {"team_rules", "league_ladder", "breeding", "evolution"});
}

TemplateRuntimeProfile makeSoulslikeLiteProfile() {
    return makeGenreProfile("soulslike_lite_rpg", "Soulslike Lite RPG", {"bosses", "stamina", "checkpoints"},
                            {"stamina_combat_loop", "checkpoint_recovery_loop", "boss_phase_loop", "save_loop"},
                            "soulslike", {"stamina_combat", "currency_recovery", "boss_phases", "checkpoint_respawn"});
}

TemplateRuntimeProfile makeIdleIncrementalProfile() {
    return makeGenreProfile("idle_incremental_rpg", "Idle Incremental RPG", {"idle", "prestige", "automation"},
                            {"offline_gain_loop", "auto_battle_loop", "prestige_upgrade_loop", "save_loop"},
                            "idle", {"offline_gains", "prestige", "auto_battle", "upgrade_tree"});
}

TemplateRuntimeProfile makeStrategyKingdomProfile() {
    return makeGenreProfile("strategy_kingdom_rpg", "Strategy Kingdom RPG", {"kingdom", "resources", "armies"},
                            {"kingdom_build_loop", "recruitment_loop", "army_mission_loop", "save_loop"},
                            "kingdom", {"buildings", "recruiting", "resources", "army_missions"});
}

TemplateRuntimeProfile makeRacingAdventureProfile() {
    return makeGenreProfile("racing_adventure_rpg", "Racing Adventure RPG", {"racing", "vehicles", "licenses"},
                            {"vehicle_upgrade_loop", "track_event_loop", "license_progression_loop", "save_loop"},
                            "racing", {"vehicle_stats", "track_events", "overworld_racing", "licenses"});
}

TemplateRuntimeProfile makeRhythmRpgProfile() {
    return makeGenreProfile("rhythm_rpg", "Rhythm RPG", {"rhythm", "combat", "timing"},
                            {"song_chart_loop", "timed_battle_loop", "performance_grade_loop", "save_loop"},
                            "rhythm", {"beat_timed_combat", "song_charts", "timing_windows", "grades"});
}

TemplateRuntimeProfile makeCookingRestaurantProfile() {
    return makeGenreProfile("cooking_restaurant_rpg", "Cooking Restaurant RPG", {"cooking", "restaurant", "relationships"},
                            {"recipe_prep_loop", "customer_queue_loop", "restaurant_upgrade_loop", "save_loop"},
                            "restaurant", {"recipes", "customer_queue", "restaurant_upgrades", "food_buffs"});
}

TemplateRuntimeProfile makeSchoolLifeProfile() {
    return makeGenreProfile("school_life_rpg", "School Life RPG", {"school", "calendar", "social-links"},
                            {"class_calendar_loop", "club_activity_loop", "social_link_loop", "save_loop"},
                            "schoolLife", {"classes", "clubs", "social_links", "exams"});
}

TemplateRuntimeProfile makePirateProfile() {
    return makeGenreProfile("pirate_rpg", "Pirate RPG", {"pirates", "crew", "islands"},
                            {"crew_ship_loop", "island_treasure_loop", "naval_encounter_loop", "save_loop"},
                            "pirate", {"ship_upgrades", "crew", "treasure_maps", "naval_combat"});
}

TemplateRuntimeProfile makeSportsTeamProfile() {
    return makeGenreProfile("sports_team_rpg", "Sports Team RPG", {"sports", "team", "season"},
                            {"team_roster_loop", "training_week_loop", "match_day_loop", "save_loop"},
                            "sportsTeam", {"team_roster", "training", "season_matches", "relationships"});
}

TemplateRuntimeProfile makePetShopProfile() {
    return makeGenreProfile("pet_shop_creature_care_rpg", "Pet Shop Creature Care RPG", {"pets", "care", "shop"},
                            {"pet_care_loop", "breeding_loop", "customer_request_loop", "save_loop"},
                            "petCare", {"pet_care", "breeding", "shop_economy", "customer_requests"});
}

TemplateRuntimeProfile makeDetectiveNoirProfile() {
    return makeGenreProfile("detective_noir_vn_rpg", "Detective Noir VN RPG", {"noir", "visual-novel", "evidence"},
                            {"noir_case_loop", "evidence_board_loop", "branching_ending_loop", "save_loop"},
                            "detectiveNoir", {"evidence_boards", "branching_endings", "noir_dialogue", "case_variables"});
}

TemplateRuntimeProfile makeCityBuilderProfile() {
    return makeGenreProfile("city_builder_rpg", "City Builder RPG", {"city", "citizens", "resources"},
                            {"district_build_loop", "citizen_need_loop", "resource_chain_loop", "save_loop"},
                            "cityBuilder", {"town_placement", "citizen_needs", "resident_quests", "resource_chains"});
}

TemplateRuntimeProfile makeTowerDefenseProfile() {
    return makeGenreProfile("tower_defense_rpg", "Tower Defense RPG", {"tower-defense", "waves", "traps"},
                            {"wave_design_loop", "tower_upgrade_loop", "hero_defense_loop", "save_loop"},
                            "towerDefense", {"wave_design", "towers", "traps", "hero_abilities"});
}

TemplateRuntimeProfile makeBeatEmUpProfile() {
    return makeGenreProfile("beat_em_up_rpg", "Beat Em Up RPG", {"brawler", "stages", "combos"},
                            {"stage_brawler_loop", "combo_progression_loop", "pickup_reward_loop", "save_loop"},
                            "beatEmUp", {"brawler_combat", "combos", "pickups", "stage_progression"});
}

TemplateRuntimeProfile makeOpenWorldSurvivalProfile() {
    return makeGenreProfile("open_world_survival_rpg", "Open World Survival RPG", {"open-world", "survival", "crafting"},
                            {"camp_setup_loop", "survival_needs_loop", "biome_event_loop", "save_loop"},
                            "openWorldSurvival", {"hunger_thirst", "camps", "crafting", "dynamic_events"});
}

TemplateRuntimeProfile makeFactionPoliticsProfile() {
    return makeGenreProfile("faction_politics_rpg", "Faction Politics RPG", {"factions", "diplomacy", "territory"},
                            {"reputation_choice_loop", "diplomacy_mission_loop", "territory_influence_loop", "save_loop"},
                            "factionPolitics", {"reputation", "diplomacy", "faction_quests", "territory_influence"});
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
        makeRogueliteDungeonProfile(),
        makeSurvivalHorrorProfile(),
        makeFarmingAdventureProfile(),
        makeCardBattlerProfile(),
        makePlatformerRpgProfile(),
        makeGachaHeroProfile(),
        makeMysteryDetectiveProfile(),
        makeWorldExplorationProfile(),
        makeSpaceOperaProfile(),
        makePostApocalypticProfile(),
        makeTacticalMechaProfile(),
        makeMonsterTamerArenaProfile(),
        makeSoulslikeLiteProfile(),
        makeIdleIncrementalProfile(),
        makeStrategyKingdomProfile(),
        makeRacingAdventureProfile(),
        makeRhythmRpgProfile(),
        makeCookingRestaurantProfile(),
        makeSchoolLifeProfile(),
        makePirateProfile(),
        makeSportsTeamProfile(),
        makePetShopProfile(),
        makeDetectiveNoirProfile(),
        makeCityBuilderProfile(),
        makeTowerDefenseProfile(),
        makeBeatEmUpProfile(),
        makeOpenWorldSurvivalProfile(),
        makeFactionPoliticsProfile(),
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
