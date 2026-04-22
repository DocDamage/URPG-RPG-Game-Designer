#include "engine/core/accessibility/accessibility_auditor.h"

#include <unordered_map>

namespace urpg::accessibility {

void AccessibilityAuditor::ingestElements(const std::vector<UiElementSnapshot>& elements) {
    m_elements = elements;
}

std::vector<AccessibilityIssue> AccessibilityAuditor::audit() {
    m_issues.clear();

    bool hasFocusableElement = false;
    std::unordered_map<int32_t, std::vector<std::string>> focusOrderMap;

    for (const auto& element : m_elements) {
        if (element.hasFocus) {
            hasFocusableElement = true;
            if (element.label.empty()) {
                m_issues.push_back(AccessibilityIssue{
                    IssueSeverity::Error,
                    IssueCategory::MissingLabel,
                    element.id,
                    "Focusable element is missing a label"
                });
            }
        }

        if (element.focusOrder > 0) {
            focusOrderMap[element.focusOrder].push_back(element.id);
        }

        if (element.contrastRatio > 0.0f && element.contrastRatio < 3.0f) {
            m_issues.push_back(AccessibilityIssue{
                IssueSeverity::Error,
                IssueCategory::Contrast,
                element.id,
                "Contrast ratio below minimum threshold of 3.0"
            });
        }
    }

    for (const auto& [order, ids] : focusOrderMap) {
        if (ids.size() > 1) {
            for (const auto& id : ids) {
                m_issues.push_back(AccessibilityIssue{
                    IssueSeverity::Warning,
                    IssueCategory::FocusOrder,
                    id,
                    "Duplicate focus order detected"
                });
            }
        }
    }

    if (!m_elements.empty() && !hasFocusableElement) {
        m_issues.push_back(AccessibilityIssue{
            IssueSeverity::Warning,
            IssueCategory::Navigation,
            "",
            "No focusable elements detected"
        });
    }

    return m_issues;
}

size_t AccessibilityAuditor::getIssueCount() const {
    return m_issues.size();
}

void AccessibilityAuditor::clear() {
    m_elements.clear();
    m_issues.clear();
}

} // namespace urpg::accessibility
