#include "editor/ui/menu_preview_panel.h"
#include "engine/core/engine_context.h"
#include "engine/core/ui/menu_scene_graph.h"

/**
 * NOTE: ImGui headers are expected to be provided by the editor environment.
 */
#ifdef URPG_IMGUI_ENABLED
#include <imgui.h>
#endif

namespace urpg::editor {

MenuPreviewPanel::MenuPreviewPanel()
    : EditorPanel("Menu Preview") {}

void MenuPreviewPanel::bindRuntime(urpg::ui::MenuSceneGraph& scene_graph) {
    scene_graph_ = &scene_graph;
}

void MenuPreviewPanel::clearRuntime() {
    scene_graph_ = nullptr;
    has_rendered_frame_ = false;
    last_render_snapshot_ = {};
}

void MenuPreviewPanel::Render(const urpg::FrameContext& context) {
    if (!m_visible) return;

    captureRenderSnapshot();

#ifdef URPG_IMGUI_ENABLED
    if (!ImGui::Begin(m_title.c_str(), &m_visible)) {
        ImGui::End();
        return;
    }

    if (!scene_graph_) {
        ImGui::TextDisabled("No scene graph bound.");
        ImGui::End();
        return;
    }

    auto activeScene = scene_graph_->getActiveScene();
    if (!activeScene) {
        ImGui::TextDisabled("No active scene in stack.");
        ImGui::End();
        return;
    }

    ImGui::Text("Active Scene: %s", activeScene->getId().c_str());
    ImGui::Separator();

    // Render each pane in the active scene
    const auto& panes = activeScene->getPanes();
    for (const auto& pane : panes) {
        if (!pane.isVisible) continue;

        ImGui::PushID(pane.id.c_str());
        
        ImGui::Text("Pane: %s", pane.id.c_str());
        if (pane.isActive) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "[Focused]");
        }

        // Render commands within the pane
        if (ImGui::BeginChild(pane.id.c_str(), ImVec2(0, 150), true)) {
            for (size_t i = 0; i < pane.commands.size(); ++i) {
                const auto& cmd = pane.commands[i];
                bool isSelected = (i == pane.selectedCommandIndex);
                const std::string label = cmd.label.empty() ? cmd.id : cmd.label;

                if (isSelected) {
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.4f, 1.0f), "> %s", label.c_str());
                } else {
                    ImGui::Text("  %s", label.c_str());
                }
            }
            ImGui::EndChild();
        }

        ImGui::PopID();
        ImGui::Spacing();
    }

    ImGui::End();
#endif
}

void MenuPreviewPanel::refresh() {
    if (!m_visible || !scene_graph_) {
        return;
    }

    captureRenderSnapshot();
}

void MenuPreviewPanel::update() {
    refresh();
}

void MenuPreviewPanel::captureRenderSnapshot() {
    last_render_snapshot_ = {};
    if (!scene_graph_) {
        has_rendered_frame_ = false;
        return;
    }

    const auto activeScene = scene_graph_->getActiveScene();
    if (!activeScene) {
        has_rendered_frame_ = false;
        return;
    }

    last_render_snapshot_.active_scene_id = activeScene->getId();
    last_render_snapshot_.last_blocked_command_id = scene_graph_->getLastBlockedCommandId();
    last_render_snapshot_.last_blocked_reason = scene_graph_->getLastBlockedReason();

    const auto& panes = activeScene->getPanes();
    for (const auto& pane : panes) {
        if (!pane.isVisible) {
            continue;
        }

        PaneSnapshot snapshot;
        snapshot.pane_id = pane.id;
        snapshot.pane_label = pane.displayName;
        snapshot.pane_active = pane.isActive;

        for (const auto& cmd : pane.commands) {
            snapshot.command_ids.push_back(cmd.id);
            snapshot.command_labels.push_back(cmd.label.empty() ? cmd.id : cmd.label);
            snapshot.command_enabled.push_back(true);
        }

        if (pane.selectedCommandIndex < pane.commands.size()) {
            snapshot.selected_command_id = pane.commands[pane.selectedCommandIndex].id;
        }

        last_render_snapshot_.visible_panes.push_back(std::move(snapshot));
    }

    last_render_snapshot_.has_data =
        !last_render_snapshot_.active_scene_id.empty() || !last_render_snapshot_.visible_panes.empty();
    has_rendered_frame_ = true;
}

} // namespace urpg::editor
