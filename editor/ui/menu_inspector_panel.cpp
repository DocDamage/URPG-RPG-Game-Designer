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

void MenuInspectorPanel::Render(const urpg::FrameContext& context) {
    if (!m_visible) return;

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

        for (const auto& row : model_->VisibleRows()) {
            ImGui::TableNextRow();
            
            ImGui::TableNextColumn(); ImGui::Text("%s", row.pane_id.c_str());
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

void MenuInspectorPanel::RenderCommandRegistry() {
#ifdef URPG_IMGUI_ENABLED
    ImGui::TextDisabled("Placeholder for Registry state...");
#endif
}

void MenuInspectorPanel::RenderSceneGraphState() {
#ifdef URPG_IMGUI_ENABLED
    ImGui::TextDisabled("Placeholder for Scene Graph tree...");
#endif
}

void MenuInspectorPanel::RenderSelectedCommandDetails() {
#ifdef URPG_IMGUI_ENABLED
    ImGui::TextDisabled("Placeholder for Selection details...");
#endif
}

} // namespace urpg::editor
