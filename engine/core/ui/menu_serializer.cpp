#include "engine/core/ui/menu_serializer.h"

#include <algorithm>
#include <cctype>

namespace urpg::ui {

namespace {

std::string NormalizeRoute(std::string_view route_str) {
    std::string normalized(route_str);
    std::transform(normalized.begin(), normalized.end(), normalized.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return normalized;
}

MenuRouteTarget ParseRoute(std::string_view route_str) {
    const auto normalized = NormalizeRoute(route_str);

    if (normalized == "item")
        return MenuRouteTarget::Item;
    if (normalized == "skill")
        return MenuRouteTarget::Skill;
    if (normalized == "equip")
        return MenuRouteTarget::Equip;
    if (normalized == "status")
        return MenuRouteTarget::Status;
    if (normalized == "formation")
        return MenuRouteTarget::Formation;
    if (normalized == "options")
        return MenuRouteTarget::Options;
    if (normalized == "save")
        return MenuRouteTarget::Save;
    if (normalized == "load")
        return MenuRouteTarget::Load;
    if (normalized == "gameend" || normalized == "game_end")
        return MenuRouteTarget::GameEnd;
    if (normalized == "codex")
        return MenuRouteTarget::Codex;
    if (normalized == "questlog" || normalized == "quest_log")
        return MenuRouteTarget::QuestLog;
    if (normalized == "encyclopedia")
        return MenuRouteTarget::Encyclopedia;
    if (normalized == "custom")
        return MenuRouteTarget::Custom;
    return MenuRouteTarget::None;
}

std::string RouteToString(MenuRouteTarget target) {
    switch (target) {
    case MenuRouteTarget::Item:
        return "Item";
    case MenuRouteTarget::Skill:
        return "Skill";
    case MenuRouteTarget::Equip:
        return "Equip";
    case MenuRouteTarget::Status:
        return "Status";
    case MenuRouteTarget::Formation:
        return "Formation";
    case MenuRouteTarget::Options:
        return "Options";
    case MenuRouteTarget::Save:
        return "Save";
    case MenuRouteTarget::Load:
        return "Load";
    case MenuRouteTarget::GameEnd:
        return "GameEnd";
    case MenuRouteTarget::Codex:
        return "Codex";
    case MenuRouteTarget::QuestLog:
        return "QuestLog";
    case MenuRouteTarget::Encyclopedia:
        return "Encyclopedia";
    case MenuRouteTarget::Custom:
        return "Custom";
    default:
        return "None";
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

bool ParseDesignCanvas(const nlohmann::json& scene_json, MenuDesignCanvas& canvas) {
    if (!scene_json.contains("canvas")) {
        return true;
    }
    if (!scene_json["canvas"].is_object()) {
        return false;
    }

    const auto& canvas_json = scene_json["canvas"];
    canvas.width = canvas_json.value("width", canvas.width);
    canvas.height = canvas_json.value("height", canvas.height);
    return canvas.isValid();
}

bool ParsePaneLayout(const nlohmann::json& pane_json, MenuPaneLayout& layout) {
    if (!pane_json.contains("layout")) {
        return true;
    }
    if (!pane_json["layout"].is_object()) {
        return false;
    }

    const auto& layout_json = pane_json["layout"];
    layout.x = layout_json.value("x", layout.x);
    layout.y = layout_json.value("y", layout.y);
    layout.width = layout_json.value("width", layout.width);
    layout.height = layout_json.value("height", layout.height);
    layout.z_order = layout_json.value("z_order", layout.z_order);
    layout.focus_order = layout_json.value("focus_order", layout.focus_order);
    layout.anchor_left = layout_json.value("anchor_left", layout.anchor_left);
    layout.anchor_top = layout_json.value("anchor_top", layout.anchor_top);
    layout.anchor_right = layout_json.value("anchor_right", layout.anchor_right);
    layout.anchor_bottom = layout_json.value("anchor_bottom", layout.anchor_bottom);
    layout.min_width = layout_json.value("min_width", layout.min_width);
    layout.min_height = layout_json.value("min_height", layout.min_height);
    return layout.isValid();
}

nlohmann::json SerializeDesignCanvas(const MenuDesignCanvas& canvas) {
    return {
        {"width", canvas.width},
        {"height", canvas.height},
    };
}

nlohmann::json SerializePaneLayout(const MenuPaneLayout& layout) {
    return {
        {"x", layout.x},
        {"y", layout.y},
        {"width", layout.width},
        {"height", layout.height},
        {"z_order", layout.z_order},
        {"focus_order", layout.focus_order},
        {"anchor_left", layout.anchor_left},
        {"anchor_top", layout.anchor_top},
        {"anchor_right", layout.anchor_right},
        {"anchor_bottom", layout.anchor_bottom},
        {"min_width", layout.min_width},
        {"min_height", layout.min_height},
    };
}

MenuPaneLayout LegacyPaneLayout(size_t pane_index, const MenuDesignCanvas& canvas) {
    MenuPaneLayout layout;
    const int pane_width = std::min(320, canvas.width);
    const int pane_height = std::min(180, canvas.height);
    const int column_width = pane_width + 16;
    const int row_height = pane_height + 16;
    const size_t columns = std::max<size_t>(1, static_cast<size_t>(canvas.width / column_width));
    layout.x = static_cast<int>(pane_index % columns) * column_width;
    layout.y = static_cast<int>(pane_index / columns) * row_height;
    layout.width = pane_width;
    layout.height = pane_height;
    return layout;
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
        if (!j.contains("scene_id") || !j.contains("panes") || !j["panes"].is_array())
            return false;
        if (j.contains("layout_version") &&
            (!j["layout_version"].is_number_integer() || j["layout_version"].get<int>() != 1)) {
            return false;
        }

        std::string scene_id = j["scene_id"];
        auto scene = std::make_shared<MenuScene>(scene_id);
        MenuDesignCanvas canvas = scene->getDesignCanvas();
        if (!ParseDesignCanvas(j, canvas) || !scene->setDesignCanvas(canvas)) {
            return false;
        }

        for (size_t pane_index = 0; pane_index < j["panes"].size(); ++pane_index) {
            const auto& j_pane = j["panes"][pane_index];
            if (!j_pane.is_object()) {
                return false;
            }
            MenuPane pane;
            pane.id = j_pane.value("id", "");
            pane.displayName = j_pane.value("label", "");
            if (!ParsePaneLayout(j_pane, pane.layout)) {
                return false;
            }
            if (!j_pane.contains("layout")) {
                pane.layout = LegacyPaneLayout(pane_index, canvas);
            }

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
                    cmd.visibility_rules = ParseRules(j_cmd, "visibility_rules");
                    cmd.enable_rules = ParseRules(j_cmd, "enable_rules");
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

namespace {

nlohmann::json SerializeScene(const std::shared_ptr<MenuScene>& scene) {
    nlohmann::json root;
    root["layout_version"] = 1;
    root["scene_id"] = scene->getId();
    root["canvas"] = SerializeDesignCanvas(scene->getDesignCanvas());
    root["panes"] = nlohmann::json::array();

    for (const auto& pane : scene->getPanes()) {
        nlohmann::json pane_json;
        pane_json["id"] = pane.id;
        pane_json["label"] = pane.displayName;
        pane_json["layout"] = SerializePaneLayout(pane.layout);
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

            auto serializeRules = [](const std::vector<urpg::MenuCommandCondition>& rules) {
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
            };

            if (!command.visibility_rules.empty()) {
                command_json["visibility_rules"] = serializeRules(command.visibility_rules);
            }
            if (!command.enable_rules.empty()) {
                command_json["enable_rules"] = serializeRules(command.enable_rules);
            }

            pane_json["commands"].push_back(std::move(command_json));
        }

        root["panes"].push_back(std::move(pane_json));
    }

    return root;
}

} // namespace

nlohmann::json MenuSceneSerializer::Serialize(const MenuSceneGraph& graph) {
    const auto& scenes = graph.getRegisteredScenes();
    if (scenes.empty()) {
        return nlohmann::json::object();
    }

    const auto& [scene_id, scene] = *scenes.begin();
    if (!scene) {
        return nlohmann::json::object();
    }

    return SerializeScene(scene);
}

nlohmann::json MenuSceneSerializer::SerializeGraph(const MenuSceneGraph& graph) {
    nlohmann::json root;
    root["schema"] = "urpg.menu_graph.v1";
    root["scenes"] = nlohmann::json::array();
    for (const auto& [id, scene] : graph.getRegisteredScenes()) {
        (void)id;
        if (scene) {
            root["scenes"].push_back(SerializeScene(scene));
        }
    }
    if (const auto active_scene = graph.getActiveScene(); active_scene) {
        root["active_scene_id"] = active_scene->getId();
    }
    return root;
}

bool MenuSceneSerializer::DeserializeGraph(const nlohmann::json& j, MenuSceneGraph& graph) {
    if (!j.is_object() || !j.contains("scenes") || !j["scenes"].is_array()) {
        return false;
    }
    if (j.contains("schema") && (!j["schema"].is_string() || j["schema"].get<std::string>() != "urpg.menu_graph.v1")) {
        return false;
    }

    std::optional<std::string> active_scene_id;
    if (j.contains("active_scene_id")) {
        if (!j["active_scene_id"].is_string() || j["active_scene_id"].get<std::string>().empty()) {
            return false;
        }
        active_scene_id = j["active_scene_id"].get<std::string>();
    }

    MenuSceneGraph staged_graph;
    for (const auto& scene_json : j["scenes"]) {
        if (!scene_json.is_object() || !scene_json.contains("scene_id") || !scene_json["scene_id"].is_string()) {
            return false;
        }
        const auto scene_id = scene_json["scene_id"].get<std::string>();
        if (scene_id.empty() || staged_graph.getRegisteredScenes().contains(scene_id) ||
            !Deserialize(scene_json, staged_graph)) {
            return false;
        }
    }
    if (staged_graph.getRegisteredScenes().empty()) {
        return false;
    }
    if (!active_scene_id.has_value()) {
        if (const auto current_active_scene = graph.getActiveScene(); current_active_scene &&
            staged_graph.getRegisteredScenes().contains(current_active_scene->getId())) {
            active_scene_id = current_active_scene->getId();
        }
    }
    if (active_scene_id.has_value() && !staged_graph.restoreActiveScene(*active_scene_id)) {
        return false;
    }

    graph.clearRegisteredScenes();
    for (const auto& [scene_id, scene] : staged_graph.getRegisteredScenes()) {
        (void)scene_id;
        graph.registerScene(scene);
    }
    if (active_scene_id.has_value()) {
        return graph.restoreActiveScene(*active_scene_id);
    }
    return true;
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
                {"Item", MenuRouteTarget::Item},           {"Skill", MenuRouteTarget::Skill},
                {"Equip", MenuRouteTarget::Equip},         {"Status", MenuRouteTarget::Status},
                {"Formation", MenuRouteTarget::Formation}, {"Save", MenuRouteTarget::Save}};

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
