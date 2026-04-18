#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "editor/workspace_kernel.h"
#include "editor/undo_history.h"
#include "editor/property_inspector.h"

namespace urpg::editor {

    /**
     * @brief High-level abstraction for an ImGui-based editor panel.
     * Part of Wave 6: ImGui Editor Integration.
     */
    class EditorPanel {
    public:
        EditorPanel(const std::string& title) : m_title(title) {}
        virtual ~EditorPanel() = default;

        virtual void draw() = 0;
        
        const std::string& getTitle() const { return m_title; }
        bool isOpen() const { return m_isOpen; }
        void setOpen(bool open) { m_isOpen = open; }

    protected:
        std::string m_title;
        bool m_isOpen = true;
    };

    /**
     * @brief The main Editor Shell that hosts and manages panel lifecycle.
     */
    class EditorShell {
    public:
        void addPanel(std::unique_ptr<EditorPanel> panel) {
            m_panels.push_back(std::move(panel));
        }

        void renderUI() {
            // This would normally be called between ImGui::NewFrame() and ImGui::Render()
            // for (auto& panel : m_panels) {
            //     if (panel->isOpen()) {
            //         if (ImGui::Begin(panel->getTitle().c_str(), &panel->m_isOpen)) {
            //             panel->draw();
            //         }
            //         ImGui::End();
            //     }
            // }
        }

    private:
        std::vector<std::unique_ptr<EditorPanel>> m_panels;
    };

} // namespace urpg::editor
