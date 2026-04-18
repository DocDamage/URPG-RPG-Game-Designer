#include "engine/core/ui/menu_serializer.h"

#include <algorithm>
#include <cctype>

namespace urpg::ui {

namespace {

std::string NormalizeRoute(std::string_view route_str) {
    std::string normalized(route_str);
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return normalized;
}

MenuRouteTarget ParseRoute(std::string_view route_str) {
    const auto normalized = NormalizeRoute(route_str);

    if (normalized == "item") return MenuRouteTarget::Item;
    if (normalized == "skill") return MenuRouteTarget::Skill;
    if (normalized == "equip") return MenuRouteTarget::Equip;
    if (normalized == "status") return MenuRouteTarget::Status;
    if (normalized == "formation") return MenuRouteTarget::Formation;
    if (normalized == "options") return MenuRouteTarget::Options;
    if (normalized == "save") return MenuRouteTarget::Save;
    if (normalized == "load") return MenuRouteTarget::Load;
    if (normalized == "gameend" || normalized == "game_end") return MenuRouteTarget::GameEnd;
    if (normalized == "codex") return MenuRouteTarget::Codex;
    if (normalized == "questlog" || normalized == "quest_log") return MenuRouteTarget::QuestLog;
    if (normalized == "encyclopedia") return MenuRouteTarget::Encyclopedia;
    if (normalized == "custom") return MenuRouteTarget::Custom;
    return MenuRouteTarget::None;
}

std::string RouteToString(MenuRouteTarget target) {
    switch (target) {
        case MenuRouteTarget::Item: return "Item";
        case MenuRouteTarget::Skill: return "Skill";
        case MenuRouteTarget::Equip: return "Equip";
        case MenuRouteTarget::Status: return "Status";
        case MenuRouteTarget::Formation: return "Formation";
        case MenuRouteTarget::Options: return "Options";
        case MenuRouteTarget::Save: return "Save";
        case MenuRouteTarget::Load: return "Load";
        case MenuRouteTarget::GameEnd: return "GameEnd";
        case MenuRouteTarget::Custom: return "Custom";
        default: return "None";
    }
}

std::vector<urpg::MenuCommandCondition> ParseRules(const nlohmann::json& command_json, const char* field_name) {
    std::vector<urpg::MenuCommandCondition> rules;

    if (!command_json.contains(field_name) || !command_json[field_name].is_array()) {
        return rules;
    }

    for (const auto& rule_json : command_json[field_name]) {
        if (!rule_json.is_object()) {
            continue;
        }

        urpg::MenuCommandCondition rule;
        rule.switch_id = rule_json.value("switch_id", "");
        rule.variable_id = rule_json.value("variable_id", "");
        rule.variable_threshold = rule_json.value("variable_threshold", 0);
        rule.invert = rule_json.value("invert", false);
        rules.push_back(std::move(rule));
    }

    return rules;
}

bool TryImportRichMainMenu(const nlohmann::json& legacy_data, MenuPane& mainPane) {
    if (!legacy_data.contains("mainMenu") || !legacy_data["mainMenu"].is_object()) {
        return false;
    }

    const auto& main_menu = legacy_data["mainMenu"];
    if (!main_menu.contains("commands") || !main_menu["commands"].is_array()) {
        return false;
    }

    for (const auto& command_json : main_menu["commands"]) {
        if (!command_json.is_object()) {
            continue;
        }

        MenuCommandMeta meta;
        meta.id = command_json.value("id", "");
        meta.label = command_json.value("label", meta.id);
        meta.icon_id = command_json.value("icon_id", "");
        meta.route = ParseRoute(command_json.value("route", "none"));
        meta.custom_route_id = command_json.value("custom_route_id", "");
        meta.fallback_route = ParseRoute(command_json.value("fallback_route", "none"));
        meta.fallback_custom_route_id = command_json.value("fallback_custom_route_id", "");
        meta.priority = command_json.value("priority", 0);
        meta.visibility_rules = ParseRules(command_json, "visibility_rules");
        meta.enable_rules = ParseRules(command_json, "enable_rules");
        mainPane.commands.push_back(std::move(meta));
    }

    return !mainPane.commands.empty();
}

} // namespace

bool MenuSceneSerializer::Deserialize(const nlohmann::json& j, MenuSceneGraph& graph) {
    try {
        if (!j.contains("scene_id") || !j.contains("panes") || !j["panes"].is_array()) return false;

        std::string scene_id = j["scene_id"];
        auto scene = std::make_shared<MenuScene>(scene_id);
        
        for (const auto& j_pane : j["panes"]) {
            MenuPane pane;
            pane.id = j_pane.value("id", "");
            pane.displayName = j_pane.value("label", "");
            
            if (j_pane.contains("commands") && j_pane["commands"].is_array()) {
                for (const auto& j_cmd : j_pane["commands"]) {
                    MenuCommandMeta cmd;
                    cmd.id = j_cmd.value("id", "");
                    cmd.label = j_cmd.value("label", "");
                    cmd.icon_id = j_cmd.value("icon_id", "");
                    cmd.route = ParseRoute(j_cmd.value("route", "None"));
                    cmd.custom_route_id = j_cmd.value("custom_route_id", "");
                    cmd.fallback_route = ParseRoute(j_cmd.value("fallback_route", "None"));
                    cmd.fallback_custom_route_id = j_cmd.value("fallback_custom_route_id", "");
                    cmd.priority = j_cmd.value("priority", 0);
                    pane.commands.push_back(std::move(cmd));
                }
            }
            scene->addPane(pane);
        }
        graph.registerScene(scene);
        return true;
    } catch (...) {
        return false;
    }
}

nlohmann::json MenuSceneSerializer::Serialize(const MenuSceneGraph& graph) {
    const auto& scenes = graph.getRegisteredScenes();
    if (scenes.empty()) {
        return nlohmann::json::object();
    }

    const auto& [scene_id, scene] = *scenes.begin();
    if (!scene) {
        return nlohmann::json::object();
    }

    nlohmann::json root;
    root["scene_id"] = scene_id;
    root["panes"] = nlohmann::json::array();

    for (const auto& pane : scene->getPanes()) {
        nlohmann::json pane_json;
        pane_json["id"] = pane.id;
        pane_json["label"] = pane.displayName;
        pane_json["commands"] = nlohmann::json::array();

        for (const auto& command : pane.commands) {
            nlohmann::json command_json;
            command_json["id"] = command.id;
            command_json["label"] = command.label;
            command_json["icon_id"] = command.icon_id;
            command_json["route"] = RouteToString(command.route);
            command_json["custom_route_id"] = command.custom_route_id;
            command_json["fallback_route"] = RouteToString(command.fallback_route);
            command_json["fallback_custom_route_id"] = command.fallback_custom_route_id;
            command_json["priority"] = command.priority;
            pane_json["commands"].push_back(std::move(command_json));
        }

        root["panes"].push_back(std::move(pane_json));
    }

    return root;
}

bool MenuSceneSerializer::ImportLegacy(const nlohmann::json& legacy_data, MenuSceneGraph& out_graph) {
    try {
        // legacy_data is expected to be System.json or a plugin parameters subset
        if (!legacy_data.contains("menuCommands") && !legacy_data.contains("mainMenu")) {
            return false;
        }

        auto scene = std::make_shared<MenuScene>("MainMenu");
        MenuPane mainPane;
        mainPane.id = "p1";
        mainPane.displayName = "Main Menu";

        const bool imported_rich_main_menu = TryImportRichMainMenu(legacy_data, mainPane);

        // Handle standard RPG Maker MV/MZ menu commands from System.json
        if (!imported_rich_main_menu && legacy_data.contains("menuCommands")) {
            const auto& cmds = legacy_data["menuCommands"];
            // MV/MZ System.json uses an array of booleans for [item, skill, equip, status, formation, save]
            // where index 0=item, 1=skill, etc.
            static const std::vector<std::pair<std::string, MenuRouteTarget>> standard_mapping = {
                {"Item", MenuRouteTarget::Item},
                {"Skill", MenuRouteTarget::Skill},
                {"Equip", MenuRouteTarget::Equip},
                {"Status", MenuRouteTarget::Status},
                {"Formation", MenuRouteTarget::Formation},
                {"Save", MenuRouteTarget::Save}
            };

            for (size_t i = 0; i < std::min(cmds.size(), standard_mapping.size()); ++i) {
                if (cmds[i].get<bool>()) {
                    MenuCommandMeta meta;
                    meta.id = "legacy_" + standard_mapping[i].first;
                    meta.label = standard_mapping[i].first;
                    meta.route = standard_mapping[i].second;
                    mainPane.commands.push_back(std::move(meta));
                }
            }
        }

        // Always add Options and Game End as they are typically hardcoded/standard
        if (!imported_rich_main_menu) {
            mainPane.commands.push_back({"legacy_Options", "Options", "", MenuRouteTarget::Options});
            mainPane.commands.push_back({"legacy_GameEnd", "Game End", "", MenuRouteTarget::GameEnd});
        }

        scene->addPane(mainPane);
        out_graph.registerScene(scene);
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace urpg::ui
