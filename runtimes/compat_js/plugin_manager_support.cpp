#include "plugin_manager_support.h"

#include <chrono>
#include <cmath>
#include <ctime>
#include <filesystem>

namespace urpg::compat::plugin_manager_detail {

Value jsonToValue(const nlohmann::json& node) {
    Value value = Value::Nil();
    if (node.is_null()) {
        return value;
    }
    if (node.is_boolean()) {
        value.v = node.get<bool>();
        return value;
    }
    if (node.is_number_integer() || node.is_number_unsigned()) {
        value.v = node.get<int64_t>();
        return value;
    }
    if (node.is_number_float()) {
        value.v = node.get<double>();
        return value;
    }
    if (node.is_string()) {
        value.v = node.get<std::string>();
        return value;
    }
    if (node.is_array()) {
        Array array;
        array.reserve(node.size());
        for (const auto& item : node) {
            array.push_back(jsonToValue(item));
        }
        value.v = std::move(array);
        return value;
    }

    if (node.is_object()) {
        Object object;
        for (const auto& [key, item] : node.items()) {
            object[key] = jsonToValue(item);
        }
        value.v = std::move(object);
    }
    return value;
}

std::string pluginNameFromPath(const std::string& path) {
    std::filesystem::path fsPath(path);
    if (fsPath.has_stem()) {
        return fsPath.stem().string();
    }
    return path;
}

bool isTruthy(const Value& value) {
    if (std::holds_alternative<std::monostate>(value.v)) {
        return false;
    }
    if (const auto* flag = std::get_if<bool>(&value.v)) {
        return *flag;
    }
    if (const auto* integer = std::get_if<int64_t>(&value.v)) {
        return *integer != 0;
    }
    if (const auto* real = std::get_if<double>(&value.v)) {
        return std::fabs(*real) > 1e-9;
    }
    if (const auto* text = std::get_if<std::string>(&value.v)) {
        return !text->empty();
    }
    if (const auto* array = std::get_if<Array>(&value.v)) {
        return !array->empty();
    }
    if (const auto* object = std::get_if<Object>(&value.v)) {
        return !object->empty();
    }
    return false;
}

double valueAsDouble(const Value& value) {
    if (const auto* integer = std::get_if<int64_t>(&value.v)) {
        return static_cast<double>(*integer);
    }
    if (const auto* real = std::get_if<double>(&value.v)) {
        return *real;
    }
    return 0.0;
}

bool valuesEqual(const Value& lhs, const Value& rhs) {
    const bool lhsNumeric = std::holds_alternative<int64_t>(lhs.v) || std::holds_alternative<double>(lhs.v);
    const bool rhsNumeric = std::holds_alternative<int64_t>(rhs.v) || std::holds_alternative<double>(rhs.v);
    if (lhsNumeric && rhsNumeric) {
        return std::fabs(valueAsDouble(lhs) - valueAsDouble(rhs)) <= 1e-9;
    }

    if (lhs.v.index() != rhs.v.index()) {
        return false;
    }

    if (std::holds_alternative<std::monostate>(lhs.v)) {
        return true;
    }
    if (const auto* lhsBool = std::get_if<bool>(&lhs.v)) {
        return *lhsBool == std::get<bool>(rhs.v);
    }
    if (const auto* lhsString = std::get_if<std::string>(&lhs.v)) {
        return *lhsString == std::get<std::string>(rhs.v);
    }
    if (const auto* lhsArray = std::get_if<Array>(&lhs.v)) {
        const auto& rhsArray = std::get<Array>(rhs.v);
        if (lhsArray->size() != rhsArray.size()) {
            return false;
        }
        for (size_t i = 0; i < lhsArray->size(); ++i) {
            if (!valuesEqual((*lhsArray)[i], rhsArray[i])) {
                return false;
            }
        }
        return true;
    }
    if (const auto* lhsObject = std::get_if<Object>(&lhs.v)) {
        const auto& rhsObject = std::get<Object>(rhs.v);
        if (lhsObject->size() != rhsObject.size()) {
            return false;
        }

        auto lhsIt = lhsObject->begin();
        auto rhsIt = rhsObject.begin();
        while (lhsIt != lhsObject->end() && rhsIt != rhsObject.end()) {
            if (lhsIt->first != rhsIt->first || !valuesEqual(lhsIt->second, rhsIt->second)) {
                return false;
            }
            ++lhsIt;
            ++rhsIt;
        }
        return lhsIt == lhsObject->end() && rhsIt == rhsObject.end();
    }
    return false;
}

bool isNilValue(const Value& value) {
    return std::holds_alternative<std::monostate>(value.v);
}

std::string valueToString(const Value& value) {
    if (std::holds_alternative<std::monostate>(value.v)) {
        return "";
    }
    if (const auto* b = std::get_if<bool>(&value.v)) {
        return *b ? "true" : "false";
    }
    if (const auto* i = std::get_if<int64_t>(&value.v)) {
        return std::to_string(*i);
    }
    if (const auto* d = std::get_if<double>(&value.v)) {
        return std::to_string(*d);
    }
    if (const auto* s = std::get_if<std::string>(&value.v)) {
        return *s;
    }
    if (std::holds_alternative<Array>(value.v)) {
        return "[array]";
    }
    if (std::holds_alternative<Object>(value.v)) {
        return "{object}";
    }
    return "";
}

std::string describeValue(const Value& value) {
    if (std::holds_alternative<std::monostate>(value.v)) {
        return "nil";
    }
    if (const auto* text = std::get_if<std::string>(&value.v)) {
        return "\"" + *text + "\"";
    }
    return valueToString(value);
}

std::string compatSeverityToString(CompatSeverity severity) {
    switch (severity) {
        case CompatSeverity::WARN:
            return "WARN";
        case CompatSeverity::SOFT_FAIL:
            return "SOFT_FAIL";
        case CompatSeverity::HARD_FAIL:
            return "HARD_FAIL";
        case CompatSeverity::CRASH_PREVENTED:
            return "CRASH_PREVENTED";
        default:
            return "HARD_FAIL";
    }
}

std::string currentUtcTimestampIso8601() {
    using clock = std::chrono::system_clock;
    const auto now = clock::now();
    const std::time_t raw = clock::to_time_t(now);

    std::tm utcTm{};
#if defined(_WIN32)
    gmtime_s(&utcTm, &raw);
#else
    gmtime_r(&raw, &utcTm);
#endif

    char buffer[32];
    if (std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &utcTm) == 0) {
        return "1970-01-01T00:00:00Z";
    }
    return buffer;
}

} // namespace urpg::compat::plugin_manager_detail
