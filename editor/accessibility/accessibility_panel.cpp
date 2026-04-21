#include "editor/accessibility/accessibility_panel.h"

namespace urpg::editor {

void AccessibilityPanel::bindAuditor(urpg::accessibility::AccessibilityAuditor* auditor) {
    m_auditor = auditor;
}

void AccessibilityPanel::render() {
    if (!m_auditor) {
        m_snapshot = nlohmann::json::object();
        return;
    }

    const auto issues = m_auditor->audit();
    size_t errorCount = 0;
    size_t warningCount = 0;

    for (const auto& issue : issues) {
        if (issue.severity == urpg::accessibility::IssueSeverity::Error) {
            ++errorCount;
        } else {
            ++warningCount;
        }
    }

    nlohmann::json issueRows = nlohmann::json::array();
    for (const auto& issue : issues) {
        std::string severityStr = (issue.severity == urpg::accessibility::IssueSeverity::Error) ? "Error" : "Warning";
        std::string categoryStr;
        switch (issue.category) {
            case urpg::accessibility::IssueCategory::MissingLabel:
                categoryStr = "MissingLabel";
                break;
            case urpg::accessibility::IssueCategory::FocusOrder:
                categoryStr = "FocusOrder";
                break;
            case urpg::accessibility::IssueCategory::Contrast:
                categoryStr = "Contrast";
                break;
            case urpg::accessibility::IssueCategory::Navigation:
                categoryStr = "Navigation";
                break;
        }

        issueRows.push_back(nlohmann::json{
            {"severity", severityStr},
            {"category", categoryStr},
            {"elementId", issue.elementId},
            {"message", issue.message}
        });
    }

    m_snapshot = nlohmann::json{
        {"issueCount", issues.size()},
        {"errorCount", errorCount},
        {"warningCount", warningCount},
        {"issues", issueRows}
    };
}

nlohmann::json AccessibilityPanel::lastRenderSnapshot() const {
    return m_snapshot;
}

} // namespace urpg::editor
