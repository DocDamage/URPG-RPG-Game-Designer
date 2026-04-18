#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdint>

namespace urpg::editor {

    /**
     * @brief Theme and Color settings for the URPG Editor.
     * Part of Wave 6.5: ImGui Editor Style.
     */
    struct EditorTheme {
        uint32_t windowBg = 0x0F0F0FFF;
        uint32_t headerActive = 0x303030FF;
        uint32_t textNormal = 0xF0F0F0FF;
        uint32_t accentColor = 0xFF8C00FF; // URPG Orange
        float rounding = 4.0f;
    };

    /**
     * @brief Orchestrator for Editor Branding and Styling.
     */
    class StyleManager {
    public:
        void applyTheme(const EditorTheme& theme) {
            m_currentTheme = theme;
            // Native ImGui::GetStyle() manipulation goes here
            // ImGuiStyle& style = ImGui::GetStyle();
            // style.WindowRounding = theme.rounding;
            // style.Colors[ImGuiCol_WindowBg] = imColor(theme.windowBg);
        }

        void setAccent(uint32_t color) {
            m_currentTheme.accentColor = color;
            applyTheme(m_currentTheme);
        }

    private:
        EditorTheme m_currentTheme;
    };

} // namespace urpg::editor
