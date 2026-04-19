#pragma once

#include "ui_types.h"
#include "../global_state_hub.h"
#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace urpg::ui {

/**
 * @brief Authoritative store of all available UI commands in the engine.
 * 
 * The registry holds both core and custom commands, providing deterministic
 * lookup and sorting for menu composition.
 */
class MenuCommandRegistry {
public:
    using SwitchState = std::unordered_map<std::string, bool>;
    using VariableState = std::unordered_map<std::string, int32_t>;

    /**
     * @brief Helper to capture current global state into SwitchState/VariableState maps.
     * 
     * Wave 2: This bridges the GlobalStateHub to the Menu evaluators.
     */
    static void captureGlobalState(SwitchState& outSwitches, VariableState& outVariables) {
        const auto& hub = GlobalStateHub::getInstance();
        for (const auto& [id, value] : hub.getAllSwitches()) {
            outSwitches[id] = value;
        }
        for (const auto& [id, value] : hub.getAllVariables()) {
            if (std::holds_alternative<int32_t>(value)) {
                outVariables[id] = std::get<int32_t>(value);
            }
        }
    }

    void registerCommand(const MenuCommandMeta& command) {
        _commands[command.id] = command;
    }

    const MenuCommandMeta* getCommand(const std::string& id) const {
        auto it = _commands.find(id);
        return it != _commands.end() ? &it->second : nullptr;
    }

    /**
     * @brief Returns all commands sorted by priority.
     */
    std::vector<MenuCommandMeta> listCommands() const {
        std::vector<MenuCommandMeta> list;
        list.reserve(_commands.size());
        for (const auto& [id, cmd] : _commands) {
            (void)id;
            list.push_back(cmd);
        }
        std::sort(list.begin(), list.end(), [](const auto& a, const auto& b) {
            if (a.priority != b.priority) return a.priority < b.priority;
            return a.id < b.id;
        });
        return list;
    }

    /**
     * @brief Evaluates whether a command is visible for the provided runtime state.
     */
    bool isVisible(const MenuCommandMeta& command,
                   const SwitchState& switches,
                   const VariableState& variables) const {
        return evaluateRuleSet(command.visibility_rules, switches, variables);
    }

    /**
     * @brief Evaluates whether a command is enabled for the provided runtime state.
     */
    bool isEnabled(const MenuCommandMeta& command,
                   const SwitchState& switches,
                   const VariableState& variables) const {
        return evaluateRuleSet(command.enable_rules, switches, variables);
    }

    /**
     * @brief Returns visible commands sorted by priority for the provided runtime state.
     */
    std::vector<MenuCommandMeta> listCommandsForState(const SwitchState& switches,
                                                      const VariableState& variables) const {
        std::vector<MenuCommandMeta> list;
        list.reserve(_commands.size());
        for (const auto& [id, cmd] : _commands) {
            (void)id;
            if (isVisible(cmd, switches, variables)) {
                list.push_back(cmd);
            }
        }

        std::sort(list.begin(), list.end(), [](const auto& a, const auto& b) {
            if (a.priority != b.priority) return a.priority < b.priority;
            return a.id < b.id;
        });
        return list;
    }

    /**
     * @brief Loads commands from a JSON schema (from menu_commands.json).
     */
    bool loadFromSchema(const nlohmann::json& schema) {
        if (!schema.is_object() || !schema.contains("commands")) {
            return false;
        }

        const auto& cmds = schema["commands"];
        if (!cmds.is_array()) {
            return false;
        }

        for (const auto& item : cmds) {
            if (!item.contains("id")) continue;
            
            MenuCommandMeta cmd;
            cmd.id = item["id"].get<std::string>();
            cmd.label = item.value("label", cmd.id);
            cmd.icon_id = item.value("icon", "");
            cmd.priority = item.value("priority", 0);
            
            // Route Target
            std::string route_str = item.value("route", "none");
            cmd.route = parseRoute(route_str);
            if (cmd.route == MenuRouteTarget::Custom) {
                cmd.custom_route_id = item.value("custom_route_id", "");
            }
            std::string fallback_route_str = item.value("fallback_route", "none");
            cmd.fallback_route = parseRoute(fallback_route_str);
            if (cmd.fallback_route == MenuRouteTarget::Custom) {
                cmd.fallback_custom_route_id = item.value("fallback_custom_route_id", "");
            }

            cmd.visibility_rules = parseRules(item, "visibility_rules");
            cmd.enable_rules = parseRules(item, "enable_rules");

            registerCommand(cmd);
        }
        return true;
    }

    /**
     * @brief Saves commands to a JSON schema (for menu_commands.json).
     */
    nlohmann::json saveToSchema() const {
        nlohmann::json root;
        root["commands"] = nlohmann::json::array();
        for (const auto& cmd : listCommands()) {
            nlohmann::json item;
            item["id"] = cmd.id;
            item["label"] = cmd.label;
            item["icon"] = cmd.icon_id;
            item["priority"] = cmd.priority;
            item["route"] = routeToString(cmd.route);
            if (cmd.route == MenuRouteTarget::Custom) {
                item["custom_route_id"] = cmd.custom_route_id;
            }
            item["fallback_route"] = routeToString(cmd.fallback_route);
            if (cmd.fallback_route == MenuRouteTarget::Custom) {
                item["fallback_custom_route_id"] = cmd.fallback_custom_route_id;
            }
            if (!cmd.visibility_rules.empty()) {
                item["visibility_rules"] = serializeRules(cmd.visibility_rules);
            }
            if (!cmd.enable_rules.empty()) {
                item["enable_rules"] = serializeRules(cmd.enable_rules);
            }
            root["commands"].push_back(std::move(item));
        }
        return root;
    }

private:
    static bool evaluateRule(const MenuCommandCondition& rule,
                             const SwitchState& switches,
                             const VariableState& variables) {
        bool pass = true;

        if (!rule.switch_id.empty()) {
            const auto switchIt = switches.find(rule.switch_id);
            const bool switchValue = switchIt != switches.end() ? switchIt->second : false;
            pass = pass && switchValue;
        }

        if (!rule.variable_id.empty()) {
            const auto variableIt = variables.find(rule.variable_id);
            const int32_t variableValue = variableIt != variables.end() ? variableIt->second : 0;
            pass = pass && (variableValue >= rule.variable_threshold);
        }

        if (rule.invert) {
            pass = !pass;
        }
        return pass;
    }

    static bool evaluateRuleSet(const std::vector<MenuCommandCondition>& rules,
                                const SwitchState& switches,
                                const VariableState& variables) {
        for (const auto& rule : rules) {
            if (!evaluateRule(rule, switches, variables)) {
                return false;
            }
        }
        return true;
    }

    static MenuRouteTarget parseRoute(const std::string& str) {
        if (str == "item")         return MenuRouteTarget::Item;
        if (str == "skill")        return MenuRouteTarget::Skill;
        if (str == "equip")        return MenuRouteTarget::Equip;
        if (str == "status")       return MenuRouteTarget::Status;
        if (str == "formation")    return MenuRouteTarget::Formation;
        if (str == "save")         return MenuRouteTarget::Save;
        if (str == "load")         return MenuRouteTarget::Load;
        if (str == "options")      return MenuRouteTarget::Options;
        if (str == "game_end")     return MenuRouteTarget::GameEnd;
        if (str == "codex")        return MenuRouteTarget::Codex;
        if (str == "quest_log")    return MenuRouteTarget::QuestLog;
        if (str == "encyclopedia") return MenuRouteTarget::Encyclopedia;
        if (str == "custom")       return MenuRouteTarget::Custom;
        return MenuRouteTarget::None;
    }

    static std::string routeToString(MenuRouteTarget target) {
        switch (target) {
        case MenuRouteTarget::Item:        return "item";
        case MenuRouteTarget::Skill:       return "skill";
        case MenuRouteTarget::Equip:       return "equip";
        case MenuRouteTarget::Status:      return "status";
        case MenuRouteTarget::Formation:   return "formation";
        case MenuRouteTarget::Save:        return "save";
        case MenuRouteTarget::Load:        return "load";
        case MenuRouteTarget::Options:     return "options";
        case MenuRouteTarget::GameEnd:     return "game_end";
        case MenuRouteTarget::Codex:       return "codex";
        case MenuRouteTarget::QuestLog:    return "quest_log";
        case MenuRouteTarget::Encyclopedia:return "encyclopedia";
        case MenuRouteTarget::Custom:      return "custom";
        default:                           return "none";
        }
    }

    static std::vector<urpg::MenuCommandCondition> parseRules(const nlohmann::json& parent, const char* field) {
        std::vector<urpg::MenuCommandCondition> rules;
        if (!parent.contains(field) || !parent[field].is_array()) return rules;
        for (const auto& r : parent[field]) {
            if (!r.is_object()) continue;
            urpg::MenuCommandCondition cond;
            cond.switch_id = r.value("switch_id", "");
            cond.variable_id = r.value("variable_id", "");
            cond.variable_threshold = r.value("variable_threshold", 0);
            cond.invert = r.value("invert", false);
            rules.push_back(std::move(cond));
        }
        return rules;
    }

    static nlohmann::json serializeRules(const std::vector<urpg::MenuCommandCondition>& rules) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& rule : rules) {
            nlohmann::json r;
            r["switch_id"] = rule.switch_id;
            r["variable_id"] = rule.variable_id;
            r["variable_threshold"] = rule.variable_threshold;
            r["invert"] = rule.invert;
            arr.push_back(std::move(r));
        }
        return arr;
    }

    std::map<std::string, MenuCommandMeta> _commands;
};

} // namespace urpg
