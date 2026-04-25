#include "editor/accessibility/accessibility_assistant_panel.h"

namespace urpg::editor::accessibility {

std::string AccessibilityAssistantPanel::snapshotLabel(const std::vector<urpg::accessibility::AccessibilityFix>& fixes) {
    return fixes.empty() ? "accessibility:ready" : "accessibility:fixes";
}

} // namespace urpg::editor::accessibility
