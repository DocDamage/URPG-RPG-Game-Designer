#pragma once

#include "engine/core/ui/menu_command_registry.h"
#include "engine/core/ui/menu_scene_graph.h"
#include "engine/core/ui/ui_types.h"

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace urpg::editor {

enum class MenuPaneLayoutTemplate {
    CompactList,
    CenteredDialog,
    BottomOverlay,
    FullCanvas,
};

// Optional template overrides. Zero preferred dimensions retain the selected
// native template's default dimension; positive dimensions are clamped to the
// authored canvas after the requested margin is applied.
struct MenuPaneLayoutTemplateParameters {
    int margin = 32;
    int preferred_width = 0;
    int preferred_height = 0;

    bool isValid() const {
        return margin >= 0 && margin <= 4096 && preferred_width >= 0 && preferred_width <= 8192 &&
               preferred_height >= 0 && preferred_height <= 8192;
    }
};

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
    urpg::ui::MenuPaneLayout pane_layout;
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
    size_t layout_issues = 0;
    urpg::ui::MenuDesignCanvas design_canvas;
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
    std::optional<std::string> CommandIdFilter() const;
    bool ShowIssuesOnly() const;

    bool SelectRow(size_t row_index);
    bool SelectCommandById(std::string_view command_id);
    bool SelectCommandRow(std::string_view pane_id, std::string_view command_id);
    std::optional<std::string> SelectedCommandId() const;
    std::optional<MenuInspectorRow> SelectedRow() const;

    bool UpdateCommandLabel(size_t row_index, std::string label);
    bool UpdateCommandRoute(size_t row_index, urpg::MenuRouteTarget route, std::string custom_route_id);
    bool UpdatePaneLayout(size_t pane_index, urpg::ui::MenuPaneLayout layout);
    // Applies all valid pane rectangles as one native history mutation.
    bool UpdatePaneLayouts(const std::vector<std::pair<size_t, urpg::ui::MenuPaneLayout>>& layouts);
    bool ApplyPaneLayoutTemplate(size_t pane_index, MenuPaneLayoutTemplate layout_template);
    bool ApplyPaneLayoutTemplateWithParameters(size_t pane_index, MenuPaneLayoutTemplate layout_template,
                                               MenuPaneLayoutTemplateParameters parameters);
    bool UpdateDesignCanvas(urpg::ui::MenuDesignCanvas canvas);
    bool CanUndo() const;
    bool CanRedo() const;
    bool Undo();
    bool Redo();
    bool RemoveCommand(size_t row_index);
    bool AddCommand(size_t pane_index, urpg::MenuCommandMeta command);
    bool ApplyToRuntime(urpg::ui::MenuSceneGraph& scene_graph) const;

private:
    struct DocumentState {
        std::vector<urpg::ui::MenuPane> panes;
        urpg::ui::MenuDesignCanvas design_canvas;
    };

    void RecordHistoryBeforeMutation();
    DocumentState CaptureDocumentState() const;
    void RestoreDocumentState(DocumentState state);
    void RebuildFromPanes();
    void RebuildVisibleRows();
    void RestoreSelectionByCommandId(const std::optional<std::string>& command_id);

    std::vector<urpg::ui::MenuPane> panes_;
    std::string scene_id_;
    urpg::ui::MenuDesignCanvas design_canvas_;
    std::vector<DocumentState> undo_history_;
    std::vector<DocumentState> redo_history_;
    const urpg::ui::MenuCommandRegistry* registry_ = nullptr;
    urpg::ui::MenuCommandRegistry::SwitchState switches_;
    urpg::ui::MenuCommandRegistry::VariableState variables_;

    std::vector<MenuInspectorRow> all_rows_;
    std::vector<MenuInspectorRow> visible_rows_;
    std::vector<MenuInspectorIssue> issues_;
    std::optional<size_t> selected_row_index_;
    std::optional<std::string> selected_command_id_;
    std::optional<std::string> command_id_filter_;
    bool show_issues_only_ = false;
    MenuInspectorSummary summary_;
};

} // namespace urpg::editor
