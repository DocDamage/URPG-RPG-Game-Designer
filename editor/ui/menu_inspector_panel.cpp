#include "editor/ui/menu_inspector_panel.h"
#include <utility>

/**
 * NOTE: ImGui headers are expected to be provided by the editor environment.
 * If not yet available in the build system, these remain as logical placeholders.
 */
#ifdef URPG_IMGUI_ENABLED
#include <imgui.h>
#endif

namespace urpg::editor {

void MenuInspectorPanel::bindRuntime(const urpg::ui::MenuSceneGraph& scene_graph,
                                     const urpg::ui::MenuCommandRegistry& registry,
                                     const urpg::ui::MenuCommandRegistry::SwitchState& switches,
                                     const urpg::ui::MenuCommandRegistry::VariableState& variables) {
    scene_graph_ = &scene_graph;
    registry_ = &registry;
    switches_ = &switches;
    variables_ = &variables;
}

void MenuInspectorPanel::clearRuntime() {
    scene_graph_ = nullptr;
    registry_ = nullptr;
    switches_ = nullptr;
    variables_ = nullptr;
    model_ = MenuInspectorModel{};
}

MenuInspectorModel& MenuInspectorPanel::getModel() {
    return model_;
}

const MenuInspectorModel& MenuInspectorPanel::getModel() const {
    return model_;
}

void MenuInspectorPanel::setVisible(bool visible) {
    visible_ = visible;
}

bool MenuInspectorPanel::isVisible() const {
    return visible_;
}

void MenuInspectorPanel::setShowIssuesOnly(bool show_issues_only) {
    show_issues_only_ = show_issues_only;
    model_.SetShowIssuesOnly(show_issues_only_);
}

void MenuInspectorPanel::setCommandIdFilter(std::optional<std::string> command_id_filter) {
    command_id_filter_ = std::move(command_id_filter);
    model_.SetCommandIdFilter(command_id_filter_);
}

void MenuInspectorPanel::render() {
    if (!visible_) {
        return;
    }

#ifdef URPG_IMGUI_ENABLED
    if (!ImGui::Begin("Menu Inspector", &visible_)) {
        ImGui::End();
        return;
    }

    const auto& summary = model_.Summary();
    
    // Summary Section
    if (ImGui::CollapsingHeader("Summary", 32)) { // ImGuiTreeNodeFlags_DefaultOpen = 32
        // Simplified usage for placeholder
        ImGui::Text("Active Scene: %s", summary.active_scene_id.empty() ? "(none)" : summary.active_scene_id.c_str());
        ImGui::Text("Stack Depth: %zu", summary.stack_depth);
        ImGui::Text("Total Panes: %zu", summary.total_panes);
        ImGui::Text("Total Commands: %zu", summary.total_commands);
        
        if (summary.issue_count > 0) {
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Total Issues: %zu", summary.issue_count); 
        }
    }

    // Filters
    ImGui::Separator();
    if (ImGui::Checkbox("Show Issues Only", &show_issues_only_)) {
        model_.SetShowIssuesOnly(show_issues_only_);
    }
    
    // Command Table
    if (ImGui::BeginTable("MenuCommandTable", 6, 1 | 2 | 8 | 16)) { // Borders | RowBg | Resizable | ScrollY
        ImGui::TableHeadersRow();

        for (const auto& row : model_.VisibleRows()) {
            ImGui::TableNextRow();
            
            // Pane
            ImGui::TableNextColumn();
            ImGui::Text("%s", row.pane_id.c_str());

            // Command ID
            ImGui::TableNextColumn();
            ImGui::Text("%s", row.command_id.c_str());

            // Label
            ImGui::TableNextColumn();
            ImGui::Text("%s", row.command_label.c_str());

            // Route
            ImGui::TableNextColumn();
            ImGui::Text("%s", row.route_label.c_str());

            // Status
            ImGui::TableNextColumn();
            if (row.command_enabled) {
                ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Enabled");
            } else if (row.command_visible) {
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "Disabled");
            } else {
                ImGui::TextDisabled("Hidden");
            }

            // Issues
            ImGui::TableNextColumn();
            if (row.issue_count > 0) {
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%zu Issues", row.issue_count);
            } else {
                ImGui::TextDisabled("-");
            }
        }
        ImGui::EndTable();
    }

    ImGui::End();
#endif
}

void MenuInspectorPanel::refresh() {
    if (!scene_graph_ || !registry_ || !switches_ || !variables_) {
        return;
    }

    model_.LoadFromRuntime(*scene_graph_, *registry_, *switches_, *variables_);
    model_.SetShowIssuesOnly(show_issues_only_);
    model_.SetCommandIdFilter(command_id_filter_);
}

void MenuInspectorPanel::update() {
    refresh();
}

} // namespace urpg::editor
