#include "analytics_dispatcher_validator.h"

#include <algorithm>

namespace urpg::analytics {

namespace {

bool containsValue(const std::vector<std::string>& values, const std::string& value) {
    return std::find(values.begin(), values.end(), value) != values.end();
}

}  // namespace

std::vector<AnalyticsValidationIssue> AnalyticsDispatcherValidator::validate(
    const std::vector<AnalyticsEvent>& events,
    const AnalyticsValidationRules& rules) const {
    std::vector<AnalyticsValidationIssue> issues;

    for (const auto& event : events) {
        if (event.eventName.empty()) {
            issues.push_back({
                AnalyticsValidationSeverity::Error,
                AnalyticsValidationCategory::EmptyEventName,
                event.eventName,
                event.category,
                "",
                "Analytics event name is required.",
            });
        }

        if (event.category.empty()) {
            issues.push_back({
                AnalyticsValidationSeverity::Error,
                AnalyticsValidationCategory::EmptyCategory,
                event.eventName,
                event.category,
                "",
                "Analytics event category is required.",
            });
        } else if (!rules.allowedCategories.empty() &&
                   !containsValue(rules.allowedCategories, event.category)) {
            issues.push_back({
                AnalyticsValidationSeverity::Error,
                AnalyticsValidationCategory::DisallowedCategory,
                event.eventName,
                event.category,
                "",
                "Analytics event category is not in the allowed category list.",
            });
        }

        if (event.parameters.size() > rules.maxParameterCount) {
            issues.push_back({
                AnalyticsValidationSeverity::Warning,
                AnalyticsValidationCategory::ExcessiveParameterCount,
                event.eventName,
                event.category,
                "",
                "Analytics event parameter count exceeds the bounded runtime limit.",
            });
        }

        for (const auto& [key, value] : event.parameters) {
            if (key.empty()) {
                issues.push_back({
                    AnalyticsValidationSeverity::Error,
                    AnalyticsValidationCategory::EmptyParameterKey,
                    event.eventName,
                    event.category,
                    key,
                    "Analytics event parameter keys must be non-empty.",
                });
            }

            if (value.empty()) {
                issues.push_back({
                    AnalyticsValidationSeverity::Warning,
                    AnalyticsValidationCategory::EmptyParameterValue,
                    event.eventName,
                    event.category,
                    key,
                    "Analytics event parameter values should not be empty.",
                });
            }
        }
    }

    return issues;
}

}  // namespace urpg::analytics
