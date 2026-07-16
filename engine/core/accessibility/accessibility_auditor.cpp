#include "engine/core/accessibility/accessibility_auditor.h"

#include <algorithm>
#include <unordered_map>

namespace urpg::accessibility {

std::string_view issueCategoryCode(const IssueCategory category) {
    switch (category) {
    case IssueCategory::MissingLabel: return "missing_label";
    case IssueCategory::FocusOrder: return "focus_order";
    case IssueCategory::Contrast: return "contrast";
    case IssueCategory::Navigation: return "navigation";
    case IssueCategory::HitTarget: return "hit_target";
    case IssueCategory::Clipping: return "clipping";
    case IssueCategory::LocalizationOverflow: return "localization_overflow";
    case IssueCategory::UnsafeMotion: return "unsafe_motion";
    }
    return "unknown";
}

std::string_view issueCategoryName(const IssueCategory category) {
    switch (category) {
    case IssueCategory::MissingLabel: return "MissingLabel";
    case IssueCategory::FocusOrder: return "FocusOrder";
    case IssueCategory::Contrast: return "Contrast";
    case IssueCategory::Navigation: return "Navigation";
    case IssueCategory::HitTarget: return "HitTarget";
    case IssueCategory::Clipping: return "Clipping";
    case IssueCategory::LocalizationOverflow: return "LocalizationOverflow";
    case IssueCategory::UnsafeMotion: return "UnsafeMotion";
    }
    return "Unknown";
}

void AccessibilityAuditor::ingestElements(const std::vector<UiElementSnapshot>& elements) {
    m_elements = elements;
}

void AccessibilityAuditor::setAuditOptions(AccessibilityAuditOptions options) {
    options.minimumContrastRatio = std::max(1.0f, options.minimumContrastRatio);
    options.minimumHitTarget = std::max(1, options.minimumHitTarget);
    options.minimumFocusOrder = std::max(0, options.minimumFocusOrder);
    m_options = options;
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
            if (element.focusOrder < m_options.minimumFocusOrder) {
                AccessibilityIssue issue{IssueSeverity::Warning, IssueCategory::FocusOrder, element.id,
                                         "Focusable element has an invalid focus order"};
                stampSource(issue, element.id);
                m_issues.push_back(std::move(issue));
            }
        }

        if (element.hasFocus && element.focusOrder >= m_options.minimumFocusOrder) {
            focusOrderMap[element.focusOrder].push_back(element.id);
        }

        if (element.contrastRatio > 0.0f && element.contrastRatio < m_options.minimumContrastRatio) {
            AccessibilityIssue issue{
                IssueSeverity::Error,
                IssueCategory::Contrast,
                element.id,
                "Contrast ratio is below the configured minimum threshold"
            };
            stampSource(issue, element.id);
            m_issues.push_back(std::move(issue));
        }
        const auto addError = [&](const IssueCategory category, std::string message) {
            AccessibilityIssue issue{IssueSeverity::Error, category, element.id, std::move(message)};
            stampSource(issue, element.id);
            m_issues.push_back(std::move(issue));
        };
        if (m_options.touchDeclared && element.hasFocus &&
            (element.width < m_options.minimumHitTarget || element.height < m_options.minimumHitTarget))
            addError(IssueCategory::HitTarget, "Interactive target is smaller than the configured minimum hit target");
        if (element.clipped) addError(IssueCategory::Clipping, "Element content is clipped");
        if (element.localizationOverflow)
            addError(IssueCategory::LocalizationOverflow, "Localized content overflows its bounds");
        if (m_options.reducedMotion && element.motionDurationMs > 0 && !element.motionEssential)
            addError(IssueCategory::UnsafeMotion, "Non-essential motion remains enabled in reduced-motion mode");
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
