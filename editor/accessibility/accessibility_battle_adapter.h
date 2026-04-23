#pragma once

#include "engine/core/accessibility/accessibility_auditor.h"
#include "editor/battle/battle_inspector_model.h"
#include <vector>

namespace urpg::editor {

/**
 * @brief Adapts a live BattleInspectorModel into UiElementSnapshot elements
 *        for accessibility auditing.
 *
 * Each visible action row becomes one UI element. Rows with issues are
 * represented as focusable elements so the auditor can surface label and
 * navigation gaps in battle UI authoring.
 */
class AccessibilityBattleAdapter {
public:
    static std::vector<urpg::accessibility::UiElementSnapshot> ingest(
        const BattleInspectorModel& model);
};

} // namespace urpg::editor
