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
    struct RenderSnapshot {
        bool visible = true;
        std::vector<std::string> diagnostic_lines;
        size_t diagnostic_count = 0;
        std::string latest_ability_id;
        std::string latest_outcome;
    };

    AbilityInspectorPanel() = default;

    void update(const AbilitySystemComponent& asc);
    void clear();
    void render();

    const AbilityInspectorModel& getModel() const { return m_model; }
    const RenderSnapshot& getRenderSnapshot() const { return m_snapshot; }

    bool isVisible() const { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }

private:
    void rebuildSnapshot(const AbilitySystemComponent& asc);

    AbilityInspectorModel m_model;
    RenderSnapshot m_snapshot;
    bool m_visible = true;
};

} // namespace urpg::editor
