#pragma once

#include "engine/core/accessibility/accessibility_auditor.h"

#include <string>
#include <vector>

namespace urpg::accessibility {

struct AccessibilityFix {
    std::string elementId;
    std::string action;
    std::string detail;
};

class AccessibilityFixAdvisor {
public:
    [[nodiscard]] std::vector<AccessibilityFix> suggestFixes(const std::vector<AccessibilityIssue>& issues) const;
};

} // namespace urpg::accessibility
