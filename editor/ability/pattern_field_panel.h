#pragma once

#include "editor/ability/pattern_field_model.h"
#include <iostream>

namespace urpg::editor {

/**
 * @brief Terminal-based UI for the Pattern Field editor.
 * Allows painting patterns using coordinate inputs.
 */
class PatternFieldPanel {
public:
    struct RenderSnapshot {
        bool visible = true;
        std::string name;
        bool is_valid = true;
        std::vector<std::string> issues;
        std::vector<std::string> grid_rows;
    };

    PatternFieldPanel() = default;

    void update(const PatternFieldModel& model);
    void render();
    const RenderSnapshot& getRenderSnapshot() const { return m_snapshot; }

    bool isVisible() const { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }

private:
    PatternFieldModel m_model;
    RenderSnapshot m_snapshot;
    bool m_visible = true;
};

} // namespace urpg::editor
