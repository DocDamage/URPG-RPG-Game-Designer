#pragma once

#include "plugin_manager.h"

#include <nlohmann/json.hpp>
#include <string>

namespace urpg::compat::plugin_manager_detail {

Value jsonToValue(const nlohmann::json& node);
std::string pluginNameFromPath(const std::string& path);
bool isTruthy(const Value& value);
double valueAsDouble(const Value& value);
bool valuesEqual(const Value& lhs, const Value& rhs);
bool isNilValue(const Value& value);
std::string valueToString(const Value& value);
std::string describeValue(const Value& value);
std::string compatSeverityToString(CompatSeverity severity);
std::string currentUtcTimestampIso8601();

} // namespace urpg::compat::plugin_manager_detail
