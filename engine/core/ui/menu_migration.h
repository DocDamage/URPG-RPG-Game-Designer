#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace urpg::ui {

/**
 * @brief Logic for migrating RPG Maker MV/MZ menu structures (Scene_Menu, Scene_Title).
 */
class MenuMigration {
public:
    struct Progress {
        size_t total_commands = 0;
        size_t total_scenes = 0;
        std::vector<std::string> warnings;
        std::vector<std::string> errors;
    };

    /**
     * @brief Maps RM command symbols (e.g., 'item', 'skill', 'equip') to native routes.
     */
    static std::string MapCommandRoute(const std::string& rm_symbol) {
        if (rm_symbol == "item")   return "scene.item";
        if (rm_symbol == "skill")  return "scene.skill";
        if (rm_symbol == "equip")  return "scene.equip";
        if (rm_symbol == "status") return "scene.status";
        if (rm_symbol == "save")   return "scene.save";
        if (rm_symbol == "gameEnd") return "scene.game_end";
        if (rm_symbol == "options") return "scene.options";
        return "scene.custom." + rm_symbol;
    }

    /**
     * @brief Migrates an RPG Maker Window_Command set to a native URPG scene structure.
     */
    static nlohmann::json MigrateCommandPanel(const std::string& scene_id, const nlohmann::json& rm_commands, Progress& progress) {
        nlohmann::json native;
        native["id"] = "PANEL_" + scene_id;
        native["type"] = "command_grid";
        
        nlohmann::json commands = nlohmann::json::array();
        for (const auto& cmd : rm_commands) {
            nlohmann::json native_cmd;
            std::string symbol = cmd.value("symbol", "unknown");
            native_cmd["id"] = "CMD_" + symbol;
            native_cmd["symbol"] = symbol;
            native_cmd["name"] = cmd.value("name", "Unknown Command");
            native_cmd["handler_route"] = MapCommandRoute(symbol);
            
            // Fallback route mapping
            if (cmd.contains("fallback_symbol")) {
                std::string fallback = cmd.value("fallback_symbol", "");
                if (!fallback.empty()) {
                    native_cmd["fallback_route"] = MapCommandRoute(fallback);
                }
            }
            
            // Basic visibility mapping
            if (cmd.contains("enabled")) {
                native_cmd["enabled_by"] = {
                    {"switch_id", ""},
                    {"compare", "=="},
                    {"value", cmd["enabled"].get<bool>()}
                };
            }
            
            // Rich rule mapping from plugin evidence
            if (cmd.contains("visibility_rules") && cmd["visibility_rules"].is_array()) {
                native_cmd["visibility_rules"] = cmd["visibility_rules"];
            }
            if (cmd.contains("enable_rules") && cmd["enable_rules"].is_array()) {
                native_cmd["enable_rules"] = cmd["enable_rules"];
            }
            
            commands.push_back(native_cmd);
            progress.total_commands++;
        }
        native["commands"] = commands;
        progress.total_scenes++;
        return native;
    }
};

} // namespace urpg::ui
