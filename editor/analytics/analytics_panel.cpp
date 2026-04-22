#include "analytics_panel.h"

namespace {

std::string issueCategoryToString(urpg::analytics::AnalyticsValidationCategory category) {
    using urpg::analytics::AnalyticsValidationCategory;

    switch (category) {
    case AnalyticsValidationCategory::EmptyEventName:
        return "empty_event_name";
    case AnalyticsValidationCategory::EmptyCategory:
        return "empty_category";
    case AnalyticsValidationCategory::DisallowedCategory:
        return "disallowed_category";
    case AnalyticsValidationCategory::EmptyParameterKey:
        return "empty_parameter_key";
    case AnalyticsValidationCategory::EmptyParameterValue:
        return "empty_parameter_value";
    case AnalyticsValidationCategory::ExcessiveParameterCount:
        return "excessive_parameter_count";
    }

    return "unknown";
}

std::string issueSeverityToString(urpg::analytics::AnalyticsValidationSeverity severity) {
    return severity == urpg::analytics::AnalyticsValidationSeverity::Error ? "error" : "warning";
}

}  // namespace

namespace urpg::editor {

void AnalyticsPanel::bindDispatcher(urpg::analytics::AnalyticsDispatcher* dispatcher) {
    m_dispatcher = dispatcher;
}

void AnalyticsPanel::render() {
    nlohmann::json snapshot;

    if (m_dispatcher == nullptr) {
        snapshot["optIn"] = false;
        snapshot["sessionEventCount"] = 0;
        snapshot["allowedCategories"] = nlohmann::json::array();
        snapshot["validationIssueCount"] = 0;
        snapshot["validationIssues"] = nlohmann::json::array();
        snapshot["errorCount"] = 0;
        snapshot["warningCount"] = 0;
        snapshot["recentEvents"] = nlohmann::json::array();
        m_lastSnapshot = std::move(snapshot);
        return;
    }

    snapshot["optIn"] = m_dispatcher->isOptIn();
    snapshot["sessionEventCount"] = m_dispatcher->getSessionEventCount();
    snapshot["allowedCategories"] = m_dispatcher->getAllowedCategories();

    nlohmann::json allEvents = m_dispatcher->getBufferSnapshot();
    nlohmann::json recentEvents = nlohmann::json::array();
    nlohmann::json validationIssues = nlohmann::json::array();
    std::size_t errorCount = 0;
    std::size_t warningCount = 0;

    const size_t sampleSize = 10;
    size_t start = 0;
    if (allEvents.size() > sampleSize) {
        start = allEvents.size() - sampleSize;
    }

    for (size_t i = start; i < allEvents.size(); ++i) {
        recentEvents.push_back(allEvents[i]);
    }

    for (const auto& issue : m_dispatcher->getValidationIssues()) {
        if (issue.severity == urpg::analytics::AnalyticsValidationSeverity::Error) {
            ++errorCount;
        } else {
            ++warningCount;
        }

        validationIssues.push_back({
            {"event_name", issue.eventName},
            {"category_value", issue.categoryValue},
            {"parameter_key", issue.parameterKey},
            {"category", issueCategoryToString(issue.category)},
            {"severity", issueSeverityToString(issue.severity)},
            {"message", issue.message},
        });
    }

    snapshot["recentEvents"] = std::move(recentEvents);
    snapshot["validationIssueCount"] = validationIssues.size();
    snapshot["validationIssues"] = std::move(validationIssues);
    snapshot["errorCount"] = errorCount;
    snapshot["warningCount"] = warningCount;
    m_lastSnapshot = std::move(snapshot);
}

nlohmann::json AnalyticsPanel::lastRenderSnapshot() const {
    return m_lastSnapshot;
}

}  // namespace urpg::editor
