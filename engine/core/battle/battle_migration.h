#pragma once

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <cstddef>
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
                nlohmann::json condition_fallbacks = nlohmann::json::array();
                if (page.contains("conditions")) {
                    const auto migrated_condition = migrateConditionTree(page["conditions"], i, progress);
                    condition = migrated_condition.condition;
                    condition_fallbacks = migrated_condition.fallbacks;
                }
                native_phase["condition"] = condition;
                if (!condition_fallbacks.empty()) {
                    native_phase["_compat_condition_fallbacks"] = condition_fallbacks;
                }
                nlohmann::json effects = nlohmann::json::array();
                bool has_non_terminator = false;

                if (page.contains("list") && page["list"].is_array()) {
                    const auto& commands = page["list"];
                    for (size_t command_index = 0; command_index < commands.size(); ++command_index) {
                        const auto& cmd = commands[command_index];
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
                            case 108:
                                appendUnsupportedCommandEffect(
                                    effects, progress, i, code, "comment_command", params);
                                break;
                            case 111: {
                                const auto branch_capture = captureConditionalBranch(commands, command_index);
                                appendUnsupportedCommandEffect(
                                    effects, progress, i, code, "conditional_branch", params, branch_capture.source_commands);
                                command_index = branch_capture.end_index;
                                break;
                            }
                            case 121: {
                                nlohmann::json effect;
                                effect["type"] = "change_switches";
                                effect["start_switch_id"] = "SW_" + std::to_string(param_int(0, 0));
                                effect["end_switch_id"] = "SW_" + std::to_string(param_int(1, param_int(0, 0)));
                                effect["value"] = param_int(2, 0) == 0;
                                effects.push_back(effect);
                                break;
                            }
                            case 122: {
                                const int operand_type = param_int(3, 0);
                                if (operand_type != 0) {
                                    appendUnsupportedCommandEffect(
                                        effects, progress, i, code, "variable_operand_type_not_supported", params);
                                    break;
                                }
                                nlohmann::json effect;
                                effect["type"] = "change_variables";
                                effect["start_variable_id"] = "VAR_" + std::to_string(param_int(0, 0));
                                effect["end_variable_id"] = "VAR_" + std::to_string(param_int(1, param_int(0, 0)));
                                effect["operation"] = mapVariableOperation(param_int(2, 0));
                                effect["constant_value"] = param_int(4, 0);
                                effects.push_back(effect);
                                break;
                            }
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
                            case 408:
                                appendUnsupportedCommandEffect(
                                    effects, progress, i, code, "comment_continuation", params);
                                break;
                            case 412:
                                appendUnsupportedCommandEffect(
                                    effects, progress, i, code, "unexpected_branch_end", params);
                                break;
                            default:
                                appendUnsupportedCommandEffect(
                                    effects, progress, i, code, "unmapped_command", params);
                                break;
                        }
                    }
                }

                if (!has_non_terminator) {
                    nlohmann::json placeholder;
                    placeholder["type"] = "common_event";
                    placeholder["note"] = "unmapped_event_commands";
                    effects.push_back(placeholder);
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
        nlohmann::json effect_fallbacks = nlohmann::json::array();
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
        if (rm_action.contains("effects") && rm_action["effects"].is_array()) {
            for (const auto& effect_record : rm_action["effects"]) {
                appendUnsupportedActionEffectFallback(effect_fallbacks, progress, native["id"].get<std::string>(), effect_record);
            }
        }
        if (!effect_fallbacks.empty()) {
            native["_compat_effect_fallbacks"] = effect_fallbacks;
        }
        native["effects"] = effects;
        native["animation_id"] = "ANI_" + std::to_string(rm_action.value("animationId", 0));

        progress.total_actions++;
        return native;
    }

private:
    struct ConditionMigrationResult {
        nlohmann::json condition = nlohmann::json::object();
        nlohmann::json fallbacks = nlohmann::json::array();
    };

    struct BranchCaptureResult {
        size_t end_index = 0;
        nlohmann::json source_commands = nlohmann::json::array();
    };

    static bool isExplicitConditionGroup(const nlohmann::json& cond) {
        return cond.is_object() &&
               (cond.contains("children") || cond.contains("op") || cond.contains("operator") ||
                cond.contains("allOf") || cond.contains("anyOf"));
    }

    static std::string normalizeGroupOperator(std::string op) {
        std::transform(op.begin(), op.end(), op.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        return op;
    }

    static nlohmann::json makeConditionFallback(const std::string& reason,
                                                size_t page_index,
                                                const nlohmann::json& source,
                                                const std::string& source_operator = "") {
        nlohmann::json fallback = {
            {"type", "unsupported_condition_tree"},
            {"reason", reason},
            {"page_index", page_index},
            {"source", source},
        };
        if (!source_operator.empty()) {
            fallback["source_operator"] = source_operator;
        }
        return fallback;
    }

    static void appendTypedConditionWarning(Progress& progress,
                                            size_t page_index,
                                            const std::string& reason,
                                            const std::string& detail = "") {
        std::string warning = "[battle_condition_tree_unsupported_shape] Troop page " +
                              std::to_string(page_index) +
                              " condition tree uses unsupported shape (" + reason + ")";
        if (!detail.empty()) {
            warning += ": " + detail;
        }
        warning += "; fallback record preserved.";
        progress.warnings.push_back(warning);
    }

    static std::vector<nlohmann::json> extractLegacyConditionLeaves(const nlohmann::json& cond) {
        std::vector<nlohmann::json> leaves;

        if (!cond.is_object()) {
            return leaves;
        }

        if (cond.value("turnValid", false) && cond.value("turnA", 0) > 0) {
            leaves.push_back({{"turn_count", cond.value("turnA", 0)}});
        }
        if (cond.value("enemyValid", false) && cond.value("enemyHp", 0) > 0) {
            nlohmann::json leaf = {{"hp_below_percent", cond.value("enemyHp", 0)}};
            if (cond.value("enemyIndex", -1) >= 0) {
                leaf["enemy_index"] = cond.value("enemyIndex", -1);
            }
            leaves.push_back(std::move(leaf));
        }
        if (cond.value("switchValid", false) && cond.value("switchId", 0) > 0) {
            leaves.push_back({{"switch_id", "SW_" + std::to_string(cond.value("switchId", 0))}});
        }
        if (cond.value("actorValid", false) && cond.value("actorId", 0) > 0) {
            leaves.push_back({{"actor_id", "ACT_" + std::to_string(cond.value("actorId", 0))}});
        }

        return leaves;
    }

    static std::string mapVariableOperation(const int operation) {
        switch (operation) {
            case 0: return "set";
            case 1: return "add";
            case 2: return "subtract";
            case 3: return "multiply";
            case 4: return "divide";
            case 5: return "modulo";
            default: return "set";
        }
    }

    static void appendUnsupportedCommandEffect(nlohmann::json& effects,
                                               Progress& progress,
                                               size_t page_index,
                                               int code,
                                               const std::string& reason,
                                               const nlohmann::json& params,
                                               const nlohmann::json& source_commands = nlohmann::json()) {
        nlohmann::json effect = {
            {"type", "unsupported_command"},
            {"code", code},
            {"reason", reason},
            {"parameters", params},
        };
        if (!source_commands.is_null() && !source_commands.empty()) {
            effect["source_commands"] = source_commands;
        }
        effects.push_back(effect);

        progress.warnings.push_back(
            "[battle_event_command_unsupported] Troop page " + std::to_string(page_index) +
            " contains command " + std::to_string(code) + " with reason '" + reason + "'.");
    }

    static std::string classifyActionEffectReason(const nlohmann::json& effect_record) {
        if (!effect_record.is_object()) {
            return "non_object_effect_record";
        }

        if (!effect_record.contains("code") || !effect_record["code"].is_number_integer()) {
            return "missing_effect_code";
        }

        switch (effect_record["code"].get<int>()) {
            case 11: return "recover_hp_effect_unsupported";
            case 12: return "recover_mp_effect_unsupported";
            case 13: return "gain_tp_effect_unsupported";
            case 21: return "add_state_effect_unsupported";
            case 22: return "remove_state_effect_unsupported";
            case 31: return "add_buff_effect_unsupported";
            case 32: return "add_debuff_effect_unsupported";
            case 33: return "remove_buff_effect_unsupported";
            case 34: return "remove_debuff_effect_unsupported";
            case 41: return "special_effect_unsupported";
            case 42: return "grow_effect_unsupported";
            case 43: return "learn_skill_effect_unsupported";
            case 44: return "common_event_effect_unsupported";
            default: return "unmapped_effect_code";
        }
    }

    static void appendUnsupportedActionEffectFallback(nlohmann::json& fallbacks,
                                                      Progress& progress,
                                                      const std::string& native_action_id,
                                                      const nlohmann::json& effect_record) {
        const std::string reason = classifyActionEffectReason(effect_record);

        nlohmann::json fallback = {
            {"type", "unsupported_action_effect"},
            {"reason", reason},
            {"source", effect_record},
        };
        if (effect_record.is_object() && effect_record.contains("code") && effect_record["code"].is_number_integer()) {
            fallback["code"] = effect_record["code"];
        }
        fallbacks.push_back(fallback);

        std::string warning = "[battle_action_effect_unsupported] Battle action " + native_action_id +
                              " contains unsupported effect";
        if (fallback.contains("code")) {
            warning += " code " + std::to_string(fallback["code"].get<int>());
        }
        warning += " with reason '" + reason + "'; fallback record preserved.";
        progress.warnings.push_back(warning);
    }

    static BranchCaptureResult captureConditionalBranch(const nlohmann::json& commands, size_t start_index) {
        BranchCaptureResult result;
        result.end_index = start_index;

        if (!commands.is_array() || start_index >= commands.size()) {
            return result;
        }

        const int32_t start_indent = commands[start_index].value("indent", 0);
        for (size_t cursor = start_index; cursor < commands.size(); ++cursor) {
            const auto& command = commands[cursor];
            result.source_commands.push_back(command);
            result.end_index = cursor;

            const int code = command.value("code", 0);
            const int32_t indent = command.value("indent", 0);
            if (cursor > start_index && code == 412 && indent == start_indent) {
                break;
            }
        }

        return result;
    }

    static ConditionMigrationResult migrateConditionTree(const nlohmann::json& cond,
                                                         size_t page_index,
                                                         Progress& progress) {
        ConditionMigrationResult result;

        if (!cond.is_object()) {
            appendTypedConditionWarning(progress, page_index, "non_object_node");
            result.fallbacks.push_back(makeConditionFallback("non_object_node", page_index, cond));
            return result;
        }

        if (!isExplicitConditionGroup(cond)) {
            const auto leaves = extractLegacyConditionLeaves(cond);
            if (leaves.empty()) {
                return result;
            }
            if (leaves.size() == 1) {
                result.condition = leaves.front();
                return result;
            }

            result.condition = {
                {"op", "and"},
                {"children", nlohmann::json::array()},
            };
            for (const auto& leaf : leaves) {
                result.condition["children"].push_back(leaf);
            }
            return result;
        }

        bool has_all_of = cond.contains("allOf");
        bool has_any_of = cond.contains("anyOf");
        bool has_children = cond.contains("children");

        std::string op;
        nlohmann::json children = nlohmann::json::array();

        if ((has_all_of && has_any_of) || ((has_all_of || has_any_of) && has_children)) {
            appendTypedConditionWarning(progress, page_index, "ambiguous_group_shape");
            result.fallbacks.push_back(makeConditionFallback("ambiguous_group_shape", page_index, cond));
            return result;
        }

        if (has_all_of) {
            op = "and";
            children = cond["allOf"];
        } else if (has_any_of) {
            op = "or";
            children = cond["anyOf"];
        } else {
            op = normalizeGroupOperator(cond.value("op", cond.value("operator", "")));
            if (op != "and" && op != "or") {
                appendTypedConditionWarning(progress, page_index, "unsupported_operator", op);
                result.fallbacks.push_back(makeConditionFallback("unsupported_operator", page_index, cond, op));
                return result;
            }
            children = cond.value("children", nlohmann::json::array());
        }

        if (!children.is_array() || children.empty()) {
            appendTypedConditionWarning(progress, page_index, "missing_children");
            result.fallbacks.push_back(makeConditionFallback("missing_children", page_index, cond, op));
            return result;
        }

        nlohmann::json migrated_children = nlohmann::json::array();
        for (const auto& child : children) {
            const auto child_result = migrateConditionTree(child, page_index, progress);
            if (!child_result.condition.empty()) {
                migrated_children.push_back(child_result.condition);
            }
            for (const auto& fallback : child_result.fallbacks) {
                result.fallbacks.push_back(fallback);
            }
        }

        if (migrated_children.empty()) {
            appendTypedConditionWarning(progress, page_index, "no_mappable_children", op);
            result.fallbacks.push_back(makeConditionFallback("no_mappable_children", page_index, cond, op));
            return result;
        }

        if (migrated_children.size() == 1) {
            result.condition = migrated_children.front();
            return result;
        }

        result.condition = {
            {"op", op},
            {"children", migrated_children},
        };
        return result;
    }
};

} // namespace urpg::battle
