#pragma once

#include "editor/editor_shell.h"
#include "editor/workspace_kernel.h"

namespace urpg::editor {

    /**
     * @brief Panel for visualizing and managing the Scene Hierarchy.
     * Part of Wave 6.2: ImGui Hierarchy Panel.
     */
    class HierarchyPanel : public EditorPanel {
    public:
        HierarchyPanel(WorkspaceKernel& workspace) 
            : EditorPanel("Hierarchy"), m_workspace(workspace) {}

        void draw() override {
            for (const auto& rootId : m_workspace.getRootNodes()) {
                drawNodeRecursive(rootId);
            }

            // Logic for right-click context menu (Add Entity, Delete, etc.)
        }

    private:
        void drawNodeRecursive(const std::string& nodeId) {
            const auto& node = m_workspace.getNodes().at(nodeId);
            
            // Representing ImGui::TreeNodeEx behavior:
            // bool nodeOpen = ImGui::TreeNodeEx(node.name.c_str(), ...);
            // if (ImGui::IsItemClicked()) m_workspace.selectNode(nodeId);

            // if (nodeOpen) {
            //     for (const auto& childId : node.childrenIds) {
            //         drawNodeRecursive(childId);
            //     }
            //     ImGui::TreePop();
            // }
        }

        WorkspaceKernel& m_workspace;
    };

} // namespace urpg::editor
