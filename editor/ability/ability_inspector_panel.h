#pragma once

#include "editor/ability/ability_inspector_model.h"
#include <string>

namespace urpg::ability {
    class AbilitySystemComponent;
}

namespace urpg::editor {

using namespace urpg::ability;

class AbilityInspectorPanel {
public:
    AbilityInspectorPanel() = default;

    void update(const AbilitySystemComponent& asc);
    void clear();
    void render();

    const AbilityInspectorModel& getModel() const { return m_model; }

    bool isVisible() const { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }

private:
    AbilityInspectorModel m_model;
    bool m_visible = true;
};

} // namespace urpg::editor
