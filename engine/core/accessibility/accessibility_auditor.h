#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace urpg::accessibility {

enum class IssueSeverity {
    Warning,
    Error
};

enum class IssueCategory {
    MissingLabel,
    FocusOrder,
    Contrast,
    Navigation
};

struct AccessibilityIssue {
    IssueSeverity severity;
    IssueCategory category;
    std::string elementId;
    std::string message;
};

struct UiElementSnapshot {
    std::string id;
    std::string label;
    bool hasFocus = false;
    int32_t focusOrder = 0;
    float contrastRatio = 0.0f;
};

class AccessibilityAuditor {
public:
    void ingestElements(const std::vector<UiElementSnapshot>& elements);
    std::vector<AccessibilityIssue> audit();
    size_t getIssueCount() const;
    void clear();

private:
    std::vector<UiElementSnapshot> m_elements;
    std::vector<AccessibilityIssue> m_issues;
};

} // namespace urpg::accessibility
