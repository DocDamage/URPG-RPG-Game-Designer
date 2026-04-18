#pragma once

#include "editor/editor_shell.h"
#include <vector>
#include <string>

namespace urpg::editor {

    /**
     * @brief Panel for browsing and searching project assets.
     * Part of Wave 6.3: ImGui Asset Browser.
     */
    class AssetBrowserPanel : public EditorPanel {
    public:
        AssetBrowserPanel() : EditorPanel("Asset Browser") {}

        void draw() override {
            // ImGui::InputText("Search", m_searchQuery, ...);
            // ImGui::SameLine();
            // if (ImGui::Button("Refresh")) refresh();

            // ImGui::BeginChild("AssetGrid");
            // for (const auto& asset : m_assets) {
            //     // Render grid icon/label based on asset.type
            //     // ImGui::Selectable(asset.filename.c_str(), ...);
            //     // Drag-and-drop support: if (ImGui::BeginDragDropSource()) ...
            // }
            // ImGui::EndChild();
        }

    private:
        void refresh() {
            // Scan filesystem via std::filesystem
        }

        char m_searchQuery[128] = "";
        // std::vector<AssetMetadata> m_assets;
    };

} // namespace urpg::editor
