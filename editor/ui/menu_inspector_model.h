#pragma once

#include "engine/core/ui/menu_command_registry.h"
#include "engine/core/ui/menu_scene_graph.h"
#include "engine/core/ui/ui_types.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace urpg::editor {

enum class MenuInspectorIssueSeverity {
    Info,
    Warning,
    Error
};

struct MenuInspectorIssue {
    MenuInspectorIssueSeverity severity;
    std::string code;
    std::string message;
    std::optional<size_t> pane_index;
    std::optional<size_t> command_index;
    std::string scene_id;
    std::string pane_id;
    std::string command_id;
};

struct MenuInspectorRow {
    std::string scene_id;
    size_t pane_index;
    std::string pane_id;
    std::string pane_label;
    size_t command_index;
    std::string command_id;
    std::string command_label;
    std::string icon_id;
    urpg::MenuRouteTarget route;
    std::string route_label;
    std::string custom_route_id;
    urpg::MenuRouteTarget fallback_route;
    std::string fallback_route_label;
    std::string fallback_custom_route_id;
    int32_t priority;
    bool pane_visible;
    bool pane_active;
    bool command_registered;
    bool command_visible;
    bool command_enabled;
    bool row_navigable;
    size_t issue_count;
    std::string summary;
};

struct MenuInspectorSummary {
    std::string active_scene_id;
    size_t stack_depth = 0;
    size_t total_panes = 0;
    size_t visible_panes = 0;
    size_t active_panes = 0;
    size_t navigable_panes = 0;
    size_t total_commands = 0;
    size_t visible_commands = 0;
    size_t enabled_commands = 0;
    size_t blocked_commands = 0;
    size_t issue_count = 0;
    size_t missing_registry_entries = 0;
    size_t route_binding_issues = 0;
    size_t rule_validation_issues = 0;
    size_t duplicate_command_ids = 0;
};

class MenuInspectorModel {
public:
    MenuInspectorModel() = default;

    void LoadFromRuntime(
        const urpg::ui::MenuSceneGraph& scene_graph,
        const urpg::ui::MenuCommandRegistry& registry,
        const urpg::ui::MenuCommandRegistry::SwitchState& switches,
        const urpg::ui::MenuCommandRegistry::VariableState& variables);
    void Clear();

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
    std::optional<size_t> selected_row_index_;
    std::optional<std::string> command_id_filter_;
    bool show_issues_only_ = false;
    MenuInspectorSummary summary_;
};

} // namespace urpg::editor
