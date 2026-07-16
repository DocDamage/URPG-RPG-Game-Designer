#include "editor/accessibility/accessibility_panel.h"

namespace urpg::editor {

void AccessibilityPanel::bindAuditor(urpg::accessibility::AccessibilityAuditor* auditor) {
    m_auditor = auditor;
}

void AccessibilityPanel::render() {
    if (!m_auditor) {
        m_snapshot = {
            {"panel", "accessibility"},
            {"status", "disabled"},
            {"disabled_reason", "No AccessibilityAuditor is bound."},
            {"owner", "editor/accessibility"},
            {"unlock_condition", "Bind AccessibilityAuditor before rendering accessibility audit results."},
            {"issueCount", 0},
            {"errorCount", 0},
            {"warningCount", 0},
            {"issues", nlohmann::json::array()},
        };
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
        nlohmann::json issueEntry = nlohmann::json{
            {"severity", severityStr},
            {"category", std::string(urpg::accessibility::issueCategoryName(issue.category))},
            {"code", std::string(urpg::accessibility::issueCategoryCode(issue.category))},
            {"elementId", issue.elementId},
            {"objectLinked", !issue.elementId.empty()},
            {"message", issue.message}
        };
        if (!issue.sourceFile.empty()) {
            issueEntry["sourceFile"] = issue.sourceFile;
        }
        if (issue.sourceLine >= 1) {
            issueEntry["sourceLine"] = issue.sourceLine;
        }
        issueRows.push_back(std::move(issueEntry));
    }

    m_snapshot = nlohmann::json{
        {"panel", "accessibility"},
        {"status", "ready"},
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
