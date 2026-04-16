#pragma once

#include "editor/ability/ability_inspector_model.h"
#include <string>

namespace urpg::editor {

class AbilityInspectorPanel {
public:
    AbilityInspectorPanel() = default;

    void update(const AbilitySystemComponent& asc);
    void render();

    bool isVisible() const { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }

private:
    AbilityInspectorModel m_model;
    bool m_visible = true;
};

} // namespace urpg::editor
