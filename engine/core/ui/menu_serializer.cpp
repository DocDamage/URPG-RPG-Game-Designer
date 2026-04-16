#include "engine/core/ui/menu_serializer.h"

namespace urpg::ui {

namespace {

MenuRouteTarget ParseRoute(std::string_view route_str) {
    if (route_str == "Item") return MenuRouteTarget::Item;
    if (route_str == "Skill") return MenuRouteTarget::Skill;
    if (route_str == "Equip") return MenuRouteTarget::Equip;
    if (route_str == "Status") return MenuRouteTarget::Status;
    if (route_str == "Formation") return MenuRouteTarget::Formation;
    if (route_str == "Options") return MenuRouteTarget::Options;
    if (route_str == "Save") return MenuRouteTarget::Save;
    if (route_str == "Load") return MenuRouteTarget::Load;
    if (route_str == "GameEnd") return MenuRouteTarget::GameEnd;
    if (route_str == "Custom") return MenuRouteTarget::Custom;
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
    // Current MenuSceneGraph doesn't allow iterating registered scenes easily in its public API.
    // This will be expanded as the graph API matures.
    return nlohmann::json::object();
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

        // Handle standard RPG Maker MV/MZ menu commands from System.json
        if (legacy_data.contains("menuCommands")) {
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
        mainPane.commands.push_back({"legacy_Options", "Options", "", MenuRouteTarget::Options});
        mainPane.commands.push_back({"legacy_GameEnd", "Game End", "", MenuRouteTarget::GameEnd});

        scene->addPane(mainPane);
        out_graph.registerScene(scene);
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace urpg::ui
