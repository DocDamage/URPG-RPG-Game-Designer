#pragma once

#include "ui_types.h"
#include <map>
#include <string>
#include <vector>
#include <algorithm>
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

            registerCommand(cmd);
        }
        return true;
    }

private:
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

    std::map<std::string, MenuCommandMeta> _commands;
};

} // namespace urpg
