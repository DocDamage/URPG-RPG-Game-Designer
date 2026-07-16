#pragma once

#include <cstdint>
#include <string>
#include <string_view>
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
    Navigation,
    HitTarget,
    Clipping,
    LocalizationOverflow,
    UnsafeMotion
};

std::string_view issueCategoryCode(IssueCategory category);
std::string_view issueCategoryName(IssueCategory category);

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
    int32_t width = 0;
    int32_t height = 0;
    bool clipped = false;
    bool localizationOverflow = false;
    uint32_t motionDurationMs = 0;
    bool motionEssential = false;
};

struct AccessibilityAuditOptions {
    bool touchDeclared = false;
    bool reducedMotion = false;
    float minimumContrastRatio = 3.0f;
    int32_t minimumHitTarget = 44;
    int32_t minimumFocusOrder = 1;
};

class AccessibilityAuditor {
public:
    void ingestElements(const std::vector<UiElementSnapshot>& elements);
    void setAuditOptions(AccessibilityAuditOptions options);
    std::vector<AccessibilityIssue> audit();
    size_t getIssueCount() const;
    void clear();

private:
    std::vector<UiElementSnapshot> m_elements;
    std::vector<AccessibilityIssue> m_issues;
    AccessibilityAuditOptions m_options;
};

} // namespace urpg::accessibility
