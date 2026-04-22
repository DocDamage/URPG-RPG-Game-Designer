#include "editor/ui/menu_inspector_panel.h"
#include "engine/core/engine_context.h"
#include <utility>

/**
 * NOTE: ImGui headers are expected to be provided by the editor environment.
 */
#ifdef URPG_IMGUI_ENABLED
#include <imgui.h>
#endif

namespace urpg::editor {

MenuInspectorPanel::MenuInspectorPanel(std::shared_ptr<MenuInspectorModel> model)
    : EditorPanel("Menu Inspector"), model_(std::move(model)) {}

void MenuInspectorPanel::refresh() {
    if (!m_visible || !model_) {
        return;
    }

    CaptureRenderSnapshot();
}

void MenuInspectorPanel::update() {
    refresh();
}

void MenuInspectorPanel::Render(const urpg::FrameContext& context) {
    (void)context;
    if (!m_visible) return;

    if (!model_) {
        last_render_snapshot_ = {};
        has_rendered_frame_ = false;
        return;
    }

    CaptureRenderSnapshot();

#ifdef URPG_IMGUI_ENABLED
    if (!ImGui::Begin(m_title.c_str(), &m_visible)) {
        ImGui::End();
        return;
    }

    if (!model_) {
        ImGui::TextDisabled("No model bound.");
        ImGui::End();
        return;
    }

    const auto& summary = model_->Summary();
    
    // Header Section - Summary
    if (ImGui::CollapsingHeader("Summary", 32)) { // ImGuiTreeNodeFlags_DefaultOpen
        ImGui::Columns(2);
        ImGui::Text("Active Scene:"); ImGui::NextColumn(); ImGui::Text("%s", summary.active_scene_id.empty() ? "(none)" : summary.active_scene_id.c_str()); ImGui::NextColumn();
        ImGui::Text("Stack Depth:"); ImGui::NextColumn(); ImGui::Text("%zu", summary.stack_depth); ImGui::NextColumn();
        ImGui::Text("Total Panes:"); ImGui::NextColumn(); ImGui::Text("%zu", summary.total_panes); ImGui::NextColumn();
        ImGui::Text("Total Commands:"); ImGui::NextColumn(); ImGui::Text("%zu", summary.total_commands); ImGui::NextColumn();
        
        if (summary.issue_count > 0) {
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Total Issues:"); ImGui::NextColumn();
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%zu", summary.issue_count); ImGui::NextColumn();
        }
        ImGui::Columns(1);
    }

    ImGui::Separator();

    // Secondary Sections
    if (ImGui::TreeNodeEx("Registry & State", 32)) {
        RenderCommandRegistry();
        ImGui::TreePop();
    }

    if (ImGui::TreeNodeEx("Scene Graph", 32)) {
        RenderSceneGraphState();
        ImGui::TreePop();
    }

    if (ImGui::TreeNodeEx("Selected Command", 32)) {
        RenderSelectedCommandDetails();
        ImGui::TreePop();
    }

    // Command List Table
    ImGui::Text("Command Audit:");
    if (ImGui::BeginTable("MenuAuditTable", 6, 1 | 2 | 8 | 16)) { // Borders | RowBg | Resizable | ScrollY
        ImGui::TableSetupColumn("Pane", 0, 0.15f);
        ImGui::TableSetupColumn("Command ID", 0, 0.25f);
        ImGui::TableSetupColumn("Label", 0, 0.20f);
        ImGui::TableSetupColumn("Route", 0, 0.15f);
        ImGui::TableSetupColumn("Status", 0, 0.10f);
        ImGui::TableSetupColumn("Issues", 0, 0.15f);
        ImGui::TableHeadersRow();

        const auto selected_command_id = model_->SelectedCommandId();
        const auto& rows = model_->VisibleRows();
        for (size_t index = 0; index < rows.size(); ++index) {
            const auto& row = rows[index];
            ImGui::TableNextRow();
            
            ImGui::TableNextColumn();
            const bool is_selected = selected_command_id.has_value() && row.command_id == *selected_command_id;
            if (ImGui::Selectable((row.pane_id + "##menu-pane-" + std::to_string(index)).c_str(),
                                  is_selected,
                                  1 | 2)) { // SpanAllColumns | AllowItemOverlap
                model_->SelectRow(index);
                CaptureRenderSnapshot();
            }

            ImGui::TableNextColumn(); ImGui::Text("%s", row.command_id.c_str());
            ImGui::TableNextColumn(); ImGui::Text("%s", row.command_label.c_str());
            ImGui::TableNextColumn(); ImGui::Text("%s", row.route_label.c_str());

            ImGui::TableNextColumn();
            if (row.command_enabled) {
                ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Enabled");
            } else if (row.command_visible) {
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "Disabled");
            } else {
                ImGui::TextDisabled("Hidden");
            }

            ImGui::TableNextColumn();
            if (row.issue_count > 0) {
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%zu", row.issue_count);
            } else {
                ImGui::TextDisabled("-");
            }
        }
        ImGui::EndTable();
    }

    ImGui::End();
#endif
}

void MenuInspectorPanel::CaptureRenderSnapshot() {
    last_render_snapshot_ = {};
    last_render_snapshot_.summary = model_->Summary();
    last_render_snapshot_.visible_rows = model_->VisibleRows();
    last_render_snapshot_.issues = model_->Issues();
    last_render_snapshot_.selected_command_id = model_->SelectedCommandId();
    last_render_snapshot_.selected_row = model_->SelectedRow();
    last_render_snapshot_.command_id_filter =
        model_->CommandIdFilter().has_value() ? *model_->CommandIdFilter() : std::string{};
    last_render_snapshot_.show_issues_only = model_->ShowIssuesOnly();
    last_render_snapshot_.has_data =
        !last_render_snapshot_.summary.active_scene_id.empty() ||
        last_render_snapshot_.summary.total_panes > 0 ||
        !last_render_snapshot_.visible_rows.empty() ||
        !last_render_snapshot_.issues.empty();

    has_rendered_frame_ = true;
}

void MenuInspectorPanel::RenderCommandRegistry() {
#ifdef URPG_IMGUI_ENABLED
    const auto& summary = last_render_snapshot_.summary;
    ImGui::Text("Missing registry entries: %zu", summary.missing_registry_entries);
    ImGui::Text("Duplicate command ids: %zu", summary.duplicate_command_ids);
    ImGui::Text("Route binding issues: %zu", summary.route_binding_issues);
    ImGui::Text("Rule validation issues: %zu", summary.rule_validation_issues);
    ImGui::Text("Total inspector issues: %zu", last_render_snapshot_.issues.size());
#endif
}

void MenuInspectorPanel::RenderSceneGraphState() {
#ifdef URPG_IMGUI_ENABLED
    const auto& summary = last_render_snapshot_.summary;
    ImGui::Text("Visible panes: %zu / %zu", summary.visible_panes, summary.total_panes);
    ImGui::Text("Active panes: %zu", summary.active_panes);
    ImGui::Text("Navigable panes: %zu", summary.navigable_panes);
    ImGui::Text("Visible commands: %zu / %zu", summary.visible_commands, summary.total_commands);
    ImGui::Text("Enabled commands: %zu", summary.enabled_commands);
    ImGui::Text("Blocked commands: %zu", summary.blocked_commands);
    ImGui::Text("Command id filter: %s",
                last_render_snapshot_.command_id_filter.empty() ? "(none)" : last_render_snapshot_.command_id_filter.c_str());
    ImGui::Text("Issues only: %s", last_render_snapshot_.show_issues_only ? "yes" : "no");
#endif
}

void MenuInspectorPanel::RenderSelectedCommandDetails() {
#ifdef URPG_IMGUI_ENABLED
    if (!last_render_snapshot_.selected_row.has_value()) {
        ImGui::TextDisabled("No command selected.");
        return;
    }

    const auto& row = *last_render_snapshot_.selected_row;
    ImGui::Text("Command: %s", row.command_id.c_str());
    ImGui::Text("Label: %s", row.command_label.c_str());
    ImGui::Text("Pane: %s", row.pane_label.c_str());
    ImGui::Text("Route: %s", row.route_label.c_str());
    ImGui::Text("Summary: %s", row.summary.c_str());
    ImGui::Text("Visible / Enabled / Navigable: %s / %s / %s",
                row.command_visible ? "yes" : "no",
                row.command_enabled ? "yes" : "no",
                row.row_navigable ? "yes" : "no");
    ImGui::Text("Issues on row: %zu", row.issue_count);
#endif
}

} // namespace urpg::editor
