#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <functional>

namespace urpg {

/**
 * @brief Identifies the functional target of a menu command.
 * 
 * Instead of jumping to a legacy scene name, native menu commands
 * point to abstract RouteTarget IDs.
 */
enum class MenuRouteTarget : uint32_t {
    None = 0,
    Item,
    Skill,
    Equip,
    Status,
    Formation,
    Options,
    Save,
    Load,
    GameEnd,
    Codex,
    QuestLog,
    Encyclopedia,
    Custom = 999
};

/**
 * @brief Rules for determining if a command is visible or enabled.
 */
struct MenuCommandCondition {
    std::string switch_id{};           // Switch must be true
    std::string variable_id{};         // Variable must meet threshold
    int32_t variable_threshold = 0;
    bool invert = false;
};

/**
 * @brief Metadata for a single menu command.
 */
struct MenuCommandMeta {
    std::string id{};                  // Stable ID (e.g. "urpg.menu.item")
    std::string label{};               // Display text or localization key
    std::string icon_id{};             // Optional icon reference
    MenuRouteTarget route = MenuRouteTarget::None;
    std::string custom_route_id{};     // Used if route is Custom
    MenuRouteTarget fallback_route = MenuRouteTarget::None;
    std::string fallback_custom_route_id{}; // Used if fallback_route is Custom
    
    std::vector<MenuCommandCondition> visibility_rules{};
    std::vector<MenuCommandCondition> enable_rules{};
    
    int32_t priority = 0;              // Sort order within a block
};

} // namespace urpg
