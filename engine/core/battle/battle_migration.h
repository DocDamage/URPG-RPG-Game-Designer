#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace urpg::battle {

/**
 * @brief Logic for migrating RPG Maker MV/MZ battle data (Troops, Skills, Items).
 */
class BattleMigration {
public:
    struct Progress {
        size_t total_enemies = 0;
        size_t total_troops = 0;
        size_t total_actions = 0;
        std::vector<std::string> warnings;
        std::vector<std::string> errors;
    };

    /**
     * @brief Migrates an RPG Maker Troop to URPG Native format.
     */
    static nlohmann::json migrateTroop(const nlohmann::json& rm_troop, Progress& progress) {
        nlohmann::json native;
        native["id"] = "TRP_" + std::to_string(rm_troop.value("id", 0));
        native["name"] = rm_troop.value("name", "Unknown Troop");
        
        nlohmann::json members = nlohmann::json::array();
        for (const auto& mem : rm_troop.value("members", nlohmann::json::array())) {
            nlohmann::json native_mem;
            native_mem["enemy_id"] = "ENM_" + std::to_string(mem.value("enemyId", 0));
            native_mem["x"] = mem.value("x", 0);
            native_mem["y"] = mem.value("y", 0);
            native_mem["hidden"] = mem.value("hidden", false);
            members.push_back(native_mem);
            progress.total_enemies++;
        }
        native["members"] = members;

        native["phases"] = nlohmann::json::array();
        if (rm_troop.contains("phases") && rm_troop["phases"].is_array()) {
            native["phases"] = rm_troop["phases"];
        } else if (rm_troop.contains("pages") && rm_troop["pages"].is_array()) {
            const auto& pages = rm_troop["pages"];
            for (size_t i = 0; i < pages.size(); ++i) {
                const auto& page = pages[i];
                nlohmann::json native_phase;
                nlohmann::json condition = nlohmann::json::object();
                if (page.contains("conditions")) {
                    const auto& cond = page["conditions"];
                    if (cond.value("turnValid", false) && cond.value("turnA", 0) > 0) {
                        condition["turn_count"] = cond.value("turnA", 0);
                    }
                    if (cond.value("enemyValid", false) && cond.value("enemyHp", 0) > 0) {
                        condition["hp_below_percent"] = cond.value("enemyHp", 0);
                        if (cond.value("enemyIndex", -1) >= 0) {
                            condition["enemy_index"] = cond.value("enemyIndex", -1);
                        }
                    }
                    if (cond.value("switchValid", false) && cond.value("switchId", 0) > 0) {
                        condition["switch_id"] = "SW_" + std::to_string(cond.value("switchId", 0));
                    }
                    if (cond.value("actorValid", false) && cond.value("actorId", 0) > 0) {
                        condition["actor_id"] = "ACT_" + std::to_string(cond.value("actorId", 0));
                    }
                }
                native_phase["condition"] = condition;
                nlohmann::json effects = nlohmann::json::array();
                std::vector<int> unmapped_codes;
                bool has_non_terminator = false;

                if (page.contains("list") && page["list"].is_array()) {
                    for (const auto& cmd : page["list"]) {
                        int code = cmd.value("code", 0);
                        if (code == 0) continue;
                        has_non_terminator = true;

                        const auto& params = cmd.value("parameters", nlohmann::json::array());
                        auto param_str = [&](size_t idx, const std::string& def) -> std::string {
                            return params.size() > idx && params[idx].is_string() ? params[idx].get<std::string>() : def;
                        };
                        auto param_int = [&](size_t idx, int def) -> int {
                            return params.size() > idx && params[idx].is_number_integer() ? params[idx].get<int>() : def;
                        };

                        switch (code) {
                            case 101: {
                                nlohmann::json effect;
                                effect["type"] = "message";
                                effect["text"] = param_str(0, "");
                                effects.push_back(effect);
                                break;
                            }
                            case 117: {
                                nlohmann::json effect;
                                effect["type"] = "common_event";
                                effect["event_id"] = param_int(0, 0);
                                effects.push_back(effect);
                                break;
                            }
                            case 313: {
                                nlohmann::json effect;
                                effect["type"] = "state_change";
                                effect["target"] = "enemy";
                                effect["state_id"] = param_int(1, 0);
                                effect["add"] = param_int(0, 0) == 0;
                                effects.push_back(effect);
                                break;
                            }
                            case 332: {
                                nlohmann::json effect;
                                effect["type"] = "force_action";
                                effect["subject"] = param_int(0, 0) == 0 ? "enemy" : "actor";
                                effect["subject_index"] = param_int(1, 0);
                                effect["skill_id"] = param_int(2, 0);
                                effect["target_index"] = param_int(3, 0);
                                effects.push_back(effect);
                                break;
                            }
                            case 311: {
                                nlohmann::json effect;
                                effect["type"] = "state_change";
                                effect["target"] = "enemy";
                                effect["param"] = "hp";
                                effect["value"] = param_int(2, 0);
                                effect["operation"] = param_int(0, 0) == 0 ? "add" : "set";
                                effects.push_back(effect);
                                break;
                            }
                            case 312: {
                                nlohmann::json effect;
                                effect["type"] = "state_change";
                                effect["target"] = "enemy";
                                effect["param"] = "mp";
                                effect["value"] = param_int(2, 0);
                                effect["operation"] = param_int(0, 0) == 0 ? "add" : "set";
                                effects.push_back(effect);
                                break;
                            }
                            case 315: {
                                nlohmann::json effect;
                                effect["type"] = "state_change";
                                effect["target"] = "enemy";
                                effect["state_id"] = param_int(1, 0);
                                effect["add"] = param_int(0, 0) == 0;
                                effects.push_back(effect);
                                break;
                            }
                            case 334: {
                                nlohmann::json effect;
                                effect["type"] = "common_event";
                                effect["note"] = "abort_battle";
                                effects.push_back(effect);
                                break;
                            }
                            case 125: {
                                nlohmann::json effect;
                                effect["type"] = "change_gold";
                                effect["operation"] = param_int(0, 0) == 0 ? "increase" : "decrease";
                                effect["value"] = param_int(2, 0);
                                effects.push_back(effect);
                                break;
                            }
                            case 126: {
                                nlohmann::json effect;
                                effect["type"] = "change_items";
                                effect["item_id"] = "ITM_" + std::to_string(param_int(0, 0));
                                effect["operation"] = param_int(1, 0) == 0 ? "increase" : "decrease";
                                effect["value"] = param_int(3, 0);
                                effects.push_back(effect);
                                break;
                            }
                            case 127: {
                                nlohmann::json effect;
                                effect["type"] = "change_weapons";
                                effect["weapon_id"] = "WPN_" + std::to_string(param_int(0, 0));
                                effect["operation"] = param_int(1, 0) == 0 ? "increase" : "decrease";
                                effect["value"] = param_int(3, 0);
                                effects.push_back(effect);
                                break;
                            }
                            case 128: {
                                nlohmann::json effect;
                                effect["type"] = "change_armors";
                                effect["armor_id"] = "ARM_" + std::to_string(param_int(0, 0));
                                effect["operation"] = param_int(1, 0) == 0 ? "increase" : "decrease";
                                effect["value"] = param_int(3, 0);
                                effects.push_back(effect);
                                break;
                            }
                            case 201: {
                                nlohmann::json effect;
                                effect["type"] = "transfer_player";
                                effect["map_id"] = "MAP_" + std::to_string(param_int(0, 0));
                                effect["x"] = param_int(1, 0);
                                effect["y"] = param_int(2, 0);
                                effect["direction"] = param_int(3, 0);
                                effects.push_back(effect);
                                break;
                            }
                            case 353: {
                                nlohmann::json effect;
                                effect["type"] = "game_over";
                                effects.push_back(effect);
                                break;
                            }
                            default:
                                unmapped_codes.push_back(code);
                                break;
                        }
                    }
                }

                if (!has_non_terminator) {
                    nlohmann::json placeholder;
                    placeholder["type"] = "common_event";
                    placeholder["note"] = "unmapped_event_commands";
                    effects.push_back(placeholder);
                } else if (!unmapped_codes.empty()) {
                    nlohmann::json fallback;
                    fallback["type"] = "common_event";
                    fallback["note"] = "unmapped_commands";
                    fallback["codes"] = unmapped_codes;
                    effects.push_back(fallback);

                    std::string codes_str;
                    for (size_t j = 0; j < unmapped_codes.size(); ++j) {
                        if (j > 0) codes_str += ", ";
                        codes_str += std::to_string(unmapped_codes[j]);
                    }
                    progress.warnings.push_back(
                        "Troop page " + std::to_string(i) + " contains unmapped event commands (codes: " + codes_str + "); mapped effects are partial.");
                }

                native_phase["effects"] = effects;
                native["phases"].push_back(native_phase);
            }
        }
        
        progress.total_troops++;
        return native;
    }

    /**
     * @brief Migrates an RPG Maker Skill or Item to URPG Native battle action.
     */
    static nlohmann::json migrateAction(const nlohmann::json& rm_action, bool is_item, Progress& progress) {
        nlohmann::json native;
        std::string prefix = is_item ? "ITM_" : "SKL_";
        native["id"] = prefix + std::to_string(rm_action.value("id", 0));
        native["name"] = rm_action.value("name", "");
        native["description"] = rm_action.value("description", "");

        // Map Scope (simplified)
        int scope = rm_action.value("scope", 1);
        switch (scope) {
            case 1: native["scope"] = "single_enemy"; break;
            case 2: native["scope"] = "all_enemies"; break;
            case 3: native["scope"] = "random_enemy"; break;
            case 4: native["scope"] = "random_ally"; break;
            case 5: native["scope"] = "ally_dead"; break;
            case 6: native["scope"] = "all_allies_dead"; break;
            case 7: native["scope"] = "single_ally"; break;
            case 8: native["scope"] = "all_allies"; break;
            case 9: native["scope"] = "enemy_dead"; break;
            case 10: native["scope"] = "all_enemies_dead"; break;
            case 11: native["scope"] = "user"; break;
            case 12: native["scope"] = "ally_except_user"; break;
            default:
                native["scope"] = "none";
                progress.warnings.push_back(
                    "Battle action scope " + std::to_string(scope) + " is unsupported; falling back to 'none'.");
                break;
        }

        // Map Cost
        nlohmann::json cost;
        cost["hp"] = 0;
        cost["mp"] = rm_action.value("mpCost", 0);
        cost["tp"] = rm_action.value("tpCost", 0);
        native["cost"] = cost;

        // Map Effects
        nlohmann::json effects = nlohmann::json::array();
        // damage mapping
        auto damage = rm_action.value("damage", nlohmann::json::object());
        if (damage.contains("formula") && !damage["formula"].get<std::string>().empty()) {
            nlohmann::json dmg_effect;
            dmg_effect["type"] = "damage";
            dmg_effect["formula"] = damage["formula"];
            dmg_effect["variance"] = damage.value("variance", 20);
            dmg_effect["critical"] = damage.value("critical", false);
            effects.push_back(dmg_effect);
        }
        if (rm_action.contains("effects") && rm_action["effects"].is_array() && !rm_action["effects"].empty()) {
            progress.warnings.push_back(
                "Battle action effects payload contains unsupported non-damage effect records; preserving only mapped native effects.");
        }
        native["effects"] = effects;
        native["animation_id"] = "ANI_" + std::to_string(rm_action.value("animationId", 0));

        progress.total_actions++;
        return native;
    }
};

} // namespace urpg::battle
