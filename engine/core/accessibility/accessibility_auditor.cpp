#include "engine/core/accessibility/accessibility_auditor.h"

#include <unordered_map>

namespace urpg::accessibility {

void AccessibilityAuditor::ingestElements(const std::vector<UiElementSnapshot>& elements) {
    m_elements = elements;
}

std::vector<AccessibilityIssue> AccessibilityAuditor::audit() {
    m_issues.clear();

    // Build a lookup for source context by element id, for populating issue sourceFile.
    std::unordered_map<std::string, std::string> sourceContextMap;
    for (const auto& element : m_elements) {
        if (!element.sourceContext.empty()) {
            sourceContextMap[element.id] = element.sourceContext;
        }
    }

    // Helper: stamp source context into an issue when available.
    auto stampSource = [&](AccessibilityIssue& issue, const std::string& elementId) {
        auto it = sourceContextMap.find(elementId);
        if (it != sourceContextMap.end()) {
            issue.sourceFile = it->second;
        }
    };

    bool hasFocusableElement = false;
    std::unordered_map<int32_t, std::vector<std::string>> focusOrderMap;

    for (const auto& element : m_elements) {
        if (element.hasFocus) {
            hasFocusableElement = true;
            if (element.label.empty()) {
                AccessibilityIssue issue{
                    IssueSeverity::Error,
                    IssueCategory::MissingLabel,
                    element.id,
                    "Focusable element is missing a label"
                };
                stampSource(issue, element.id);
                m_issues.push_back(std::move(issue));
            }
        }

        if (element.focusOrder > 0) {
            focusOrderMap[element.focusOrder].push_back(element.id);
        }

        if (element.contrastRatio > 0.0f && element.contrastRatio < 3.0f) {
            AccessibilityIssue issue{
                IssueSeverity::Error,
                IssueCategory::Contrast,
                element.id,
                "Contrast ratio below minimum threshold of 3.0"
            };
            stampSource(issue, element.id);
            m_issues.push_back(std::move(issue));
        }
    }

    for (const auto& [order, ids] : focusOrderMap) {
        if (ids.size() > 1) {
            for (const auto& id : ids) {
                AccessibilityIssue issue{
                    IssueSeverity::Warning,
                    IssueCategory::FocusOrder,
                    id,
                    "Duplicate focus order detected"
                };
                stampSource(issue, id);
                m_issues.push_back(std::move(issue));
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
