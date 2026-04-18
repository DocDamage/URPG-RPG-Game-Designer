#pragma once

#include <string>
#include <string_view>

namespace urpg {
    struct FrameContext;
}

namespace urpg::editor {

/**
 * @brief Base class for ImGui-based editor panels.
 * Part of the Wave 1 Editor Productization and Wave 6 ImGui Integration.
 */
class EditorPanel {
public:
    explicit EditorPanel(std::string_view title) : m_title(title) {}
    virtual ~EditorPanel() = default;

    /**
     * @brief Render the panel's content using ImGui.
     * @param context The frame-level state (delta time, etc.).
     */
    virtual void Render(const urpg::FrameContext& context) = 0;

    const std::string& GetTitle() const { return m_title; }

    bool IsVisible() const { return m_visible; }
    void SetVisible(bool visible) { m_visible = visible; }

protected:
    std::string m_title;
    bool m_visible = true;
};

} // namespace urpg::editor
