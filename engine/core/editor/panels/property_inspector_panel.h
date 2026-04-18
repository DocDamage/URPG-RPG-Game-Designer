#pragma once

#include "editor/editor_shell.h"
#include "editor/property_inspector.h"
#include "editor/workspace_kernel.h"

namespace urpg::editor {

    /**
     * @brief Panel for inspecting and editing properties of selected scene nodes.
     * Part of Wave 6.4: ImGui Property Inspector Panel.
     */
    class PropertyInspectorPanel : public EditorPanel {
    public:
        PropertyInspectorPanel(WorkspaceKernel& workspace) 
            : EditorPanel("Inspector"), m_workspace(workspace) {}

        void draw() override {
            const auto& nodes = m_workspace.getNodes();
            std::string selectedId;

            // Find the selected node
            for (auto const& [id, node] : nodes) {
                if (node.isSelected) {
                    selectedId = id;
                    break;
                }
            }

            if (selectedId.empty()) {
                // ImGui::Text("No object selected.");
                return;
            }

            const auto& node = nodes.at(selectedId);
            // ImGui::Text("ID: %s", node.id.c_str());
            // ImGui::InputText("Name", &node.name);
            // ImGui::Separator();

            // If a node has a PropertyRegistry associated (Wave 5.4), draw it here
            // auto it = m_registries.find(selectedId);
            // if (it != m_registries.end()) {
            //     it->second->drawInspector();
            // }
        }

    private:
        WorkspaceKernel& m_workspace;
        // std::map<std::string, std::shared_ptr<PropertyRegistry>> m_registries;
    };

} // namespace urpg::editor
