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
    PatternFieldPanel() = default;

    void update(const PatternFieldModel& model);
    void render();

    bool isVisible() const { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }

private:
    PatternFieldModel m_model;
    bool m_visible = true;
};

} // namespace urpg::editor
