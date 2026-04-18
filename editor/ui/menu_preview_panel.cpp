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

void MenuPreviewPanel::bindRuntime(const urpg::ui::MenuSceneGraph& scene_graph) {
    scene_graph_ = &scene_graph;
}

void MenuPreviewPanel::clearRuntime() {
    scene_graph_ = nullptr;
}

void MenuPreviewPanel::Render(const urpg::FrameContext& context) {
    if (!m_visible) return;

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
                
                if (isSelected) {
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.4f, 1.0f), "> %s", cmd.id.c_str());
                } else {
                    ImGui::Text("  %s", cmd.id.c_str());
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
}

void MenuPreviewPanel::update() {
    refresh();
}

} // namespace urpg::editor
