#pragma once

#include "engine/core/accessibility/accessibility_auditor.h"
#include "editor/ui/menu_inspector_model.h"
#include <vector>

namespace urpg::editor {

/**
 * @brief Adapts live MenuInspectorModel rows into UiElementSnapshot elements
 *        for accessibility auditing.
 */
class AccessibilityMenuAdapter {
public:
    static std::vector<urpg::accessibility::UiElementSnapshot> ingest(const MenuInspectorModel& model);
};

} // namespace urpg::editor
