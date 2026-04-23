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
    IssueSeverity severity = IssueSeverity::Warning;
    IssueCategory category = IssueCategory::Navigation;
    std::string elementId{};
    std::string message{};
    /** Optional file path reference for actionable navigation to the source of the issue. */
    std::string sourceFile{};
    /** Optional 1-based line number within sourceFile, or -1 if not available. */
    int32_t sourceLine = -1;
};

struct UiElementSnapshot {
    std::string id{};
    std::string label{};
    bool hasFocus = false;
    int32_t focusOrder = 0;
    float contrastRatio = 0.0f;
    /** Optional source context propagated to issues generated for this element. */
    std::string sourceContext{};
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
