#pragma once

#include "engine/core/accessibility/accessibility_fix_advisor.h"

#include <string>

namespace urpg::editor::accessibility {

class AccessibilityAssistantPanel {
public:
    [[nodiscard]] static std::string snapshotLabel(const std::vector<urpg::accessibility::AccessibilityFix>& fixes);
};

} // namespace urpg::editor::accessibility
