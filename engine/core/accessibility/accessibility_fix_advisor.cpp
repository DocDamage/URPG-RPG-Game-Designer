#include "engine/core/accessibility/accessibility_fix_advisor.h"

namespace urpg::accessibility {

std::vector<AccessibilityFix> AccessibilityFixAdvisor::suggestFixes(const std::vector<AccessibilityIssue>& issues) const {
    std::vector<AccessibilityFix> fixes;
    for (const auto& issue : issues) {
        switch (issue.category) {
        case IssueCategory::MissingLabel:
            fixes.push_back({issue.elementId, "add_accessible_label", issue.message});
            break;
        case IssueCategory::FocusOrder:
            fixes.push_back({issue.elementId, "normalize_focus_order", issue.message});
            break;
        case IssueCategory::Contrast:
            fixes.push_back({issue.elementId, "raise_contrast_ratio", issue.message});
            break;
        case IssueCategory::Navigation:
            fixes.push_back({issue.elementId, "add_input_alternative", issue.message});
            break;
        case IssueCategory::HitTarget:
            fixes.push_back({issue.elementId, "enlarge_hit_target", issue.message});
            break;
        case IssueCategory::Clipping:
            fixes.push_back({issue.elementId, "repair_clipping", issue.message});
            break;
        case IssueCategory::LocalizationOverflow:
            fixes.push_back({issue.elementId, "repair_localization_overflow", issue.message});
            break;
        case IssueCategory::UnsafeMotion:
            fixes.push_back({issue.elementId, "disable_nonessential_motion", issue.message});
            break;
        }
    }
    return fixes;
}

} // namespace urpg::accessibility
