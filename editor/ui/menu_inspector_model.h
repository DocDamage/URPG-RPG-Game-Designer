#pragma once

#include "engine/core/ui/menu_command_registry.h"
#include "engine/core/ui/menu_scene_graph.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace urpg::editor {

enum class MenuInspectorIssueSeverity : uint8_t {
    Info = 0,
    Warning = 1,
    Error = 2,
};

struct MenuInspectorIssue {
    MenuInspectorIssueSeverity severity = MenuInspectorIssueSeverity::Info;
    std::string code;
    std::optional<size_t> pane_index;
    std::optional<size_t> command_index;
    std::string scene_id;
    std::string pane_id;
    std::string command_id;
    std::string message;
};

struct MenuInspectorRow {
    std::string scene_id;
    size_t pane_index = 0;
    std::string pane_id;
    std::string pane_label;
    size_t command_index = 0;
    std::string command_id;
    std::string command_label;
    std::string icon_id;
    urpg::MenuRouteTarget route = urpg::MenuRouteTarget::None;
    std::string route_label;
    std::string custom_route_id;
    urpg::MenuRouteTarget fallback_route = urpg::MenuRouteTarget::None;
    std::string fallback_route_label;
    std::string fallback_custom_route_id;
    int32_t priority = 0;
    bool pane_visible = false;
    bool pane_active = false;
    bool command_registered = false;
    bool command_visible = false;
    bool command_enabled = false;
    bool row_navigable = false;
    size_t issue_count = 0;
    std::string summary;
};

struct MenuInspectorSummary {
    size_t stack_depth = 0;
    std::string active_scene_id;
    size_t total_panes = 0;
    size_t visible_panes = 0;
    size_t active_panes = 0;
    size_t navigable_panes = 0;
    size_t total_commands = 0;
    size_t visible_commands = 0;
    size_t enabled_commands = 0;
    size_t blocked_commands = 0;
    size_t duplicate_command_ids = 0;
    size_t missing_registry_entries = 0;
    size_t route_binding_issues = 0;
    size_t rule_validation_issues = 0;
    size_t issue_count = 0;
};

class MenuInspectorModel {
public:
    void LoadFromRuntime(
        const urpg::ui::MenuSceneGraph& scene_graph,
        const urpg::ui::MenuCommandRegistry& registry,
        const urpg::ui::MenuCommandRegistry::SwitchState& switches,
        const urpg::ui::MenuCommandRegistry::VariableState& variables);
    void SetCommandIdFilter(std::optional<std::string> command_id_filter);
    void SetShowIssuesOnly(bool show_issues_only);

    const MenuInspectorSummary& Summary() const;
    const std::vector<MenuInspectorRow>& VisibleRows() const;
    const std::vector<MenuInspectorIssue>& Issues() const;

    bool SelectRow(size_t row_index);
    std::optional<std::string> SelectedCommandId() const;

private:
    void RebuildVisibleRows();

    std::vector<MenuInspectorRow> all_rows_;
    std::vector<MenuInspectorRow> visible_rows_;
    std::vector<MenuInspectorIssue> issues_;
    MenuInspectorSummary summary_;
    std::optional<std::string> command_id_filter_;
    bool show_issues_only_ = false;
    std::optional<size_t> selected_row_index_;
};

} // namespace urpg::editor