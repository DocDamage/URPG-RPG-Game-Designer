#pragma once

#include "analytics_event.h"

#include <cstddef>
#include <string>
#include <vector>

namespace urpg::analytics {

enum class AnalyticsValidationSeverity {
    Warning,
    Error,
};

enum class AnalyticsValidationCategory {
    EmptyEventName,
    EmptyCategory,
    DisallowedCategory,
    EmptyParameterKey,
    EmptyParameterValue,
    ExcessiveParameterCount,
};

struct AnalyticsValidationRules {
    std::vector<std::string> allowedCategories;
    std::size_t maxParameterCount = 8;
};

struct AnalyticsValidationIssue {
    AnalyticsValidationSeverity severity = AnalyticsValidationSeverity::Error;
    AnalyticsValidationCategory category = AnalyticsValidationCategory::EmptyEventName;
    std::string eventName;
    std::string categoryValue;
    std::string parameterKey;
    std::string message;
};

class AnalyticsDispatcherValidator {
public:
    std::vector<AnalyticsValidationIssue> validate(const std::vector<AnalyticsEvent>& events,
                                                   const AnalyticsValidationRules& rules) const;
};

}  // namespace urpg::analytics
