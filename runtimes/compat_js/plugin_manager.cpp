// PluginManager - MZ Plugin Command Registry + Execution - Implementation
// Phase 2 - Compat Layer

#include "plugin_manager.h"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cmath>
#include <ctime>
#include <deque>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <mutex>
#include <nlohmann/json.hpp>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <thread>

namespace urpg {
namespace compat {

namespace {

constexpr std::string_view kFailDirectoryScanMarker = "__urpg_fail_directory_scan__";
constexpr std::string_view kFailDirectoryEntryStatusMarker =
    "__urpg_fail_directory_entry_status__";

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

struct FixtureScriptContext {
    std::string pluginName;
    std::string commandName;
    const std::vector<Value>& args;
    const std::unordered_map<std::string, Value>& parameters;
    const Object& locals;
};

struct FixtureScriptResult {
    bool returned = false;
    Value value = Value::Nil();
};

using FixtureCommandInvoker =
    std::function<Value(const std::string& pluginName,
                        const std::string& commandName,
                        const std::vector<Value>& args)>;
using FixtureCommandByNameInvoker =
    std::function<Value(const std::string& fullName, const std::vector<Value>& args)>;

Value resolveFixtureScriptValue(const nlohmann::json& node, const FixtureScriptContext& ctx);
FixtureScriptResult runFixtureScriptSteps(const nlohmann::json& script,
                                          const std::string& pluginName,
                                          const std::string& commandName,
                                          const std::vector<Value>& args,
                                          const std::unordered_map<std::string, Value>& parameters,
                                          Object& locals,
                                          const FixtureCommandInvoker& commandInvoker,
                                          const FixtureCommandByNameInvoker& commandByNameInvoker);

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

double asDouble(const Value& value) {
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
        return std::fabs(asDouble(lhs) - asDouble(rhs)) <= 1e-9;
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

[[noreturn]] void throwFixtureResolverError(const std::string& message) {
    throw std::runtime_error("Fixture script resolver " + message);
}

void validateResolverAllowedFields(const nlohmann::json& node,
                                   const std::string& source,
                                   std::initializer_list<std::string_view> allowedFields) {
    for (const auto& [key, value] : node.items()) {
        (void)value;
        if (key == "from") {
            continue;
        }

        const auto allowedIt = std::find_if(
            allowedFields.begin(),
            allowedFields.end(),
            [&](const std::string_view field) {
                return key == field;
            }
        );
        if (allowedIt == allowedFields.end()) {
            throwFixtureResolverError(source + " does not accept field '" + key + "'");
        }
    }
}

template <typename Predicate>
const nlohmann::json& requireResolverTypedField(const nlohmann::json& node,
                                                const std::string& source,
                                                const char* fieldName,
                                                const char* typeName,
                                                Predicate&& predicate) {
    const auto it = node.find(fieldName);
    if (it == node.end() || !predicate(*it)) {
        throwFixtureResolverError(source + " requires " + typeName + " " + fieldName);
    }
    return *it;
}

const nlohmann::json& requireResolverArrayField(const nlohmann::json& node,
                                                const std::string& source,
                                                const char* fieldName) {
    return requireResolverTypedField(
        node,
        source,
        fieldName,
        "array",
        [](const nlohmann::json& field) {
            return field.is_array();
        }
    );
}

int64_t requireResolverIntegerField(const nlohmann::json& node,
                                    const std::string& source,
                                    const char* fieldName) {
    return requireResolverTypedField(
               node,
               source,
               fieldName,
               "integer",
               [](const nlohmann::json& field) {
                   return field.is_number_integer() || field.is_number_unsigned();
               }
           )
        .get<int64_t>();
}

std::string requireResolverStringField(const nlohmann::json& node,
                                       const std::string& source,
                                       const char* fieldName) {
    return requireResolverTypedField(
               node,
               source,
               fieldName,
               "string",
               [](const nlohmann::json& field) {
                   return field.is_string() && !field.get<std::string>().empty();
               }
           )
        .get<std::string>();
}

void enforceFixtureInvokeExpectation(const nlohmann::json& expectNode,
                                     const Value& invokeResult,
                                     const FixtureScriptContext& ctx,
                                     const std::string& op,
                                     const std::string& target,
                                     size_t stepIndex) {
    const auto fail = [&](const std::string& message) {
        throw std::runtime_error(
            "Fixture script " + op + " op " + message + " for " + target +
            " at index " + std::to_string(stepIndex)
        );
    };

    if (expectNode.is_string()) {
        const std::string expect = expectNode.get<std::string>();
        if (expect == "non_nil") {
            if (isNilValue(invokeResult)) {
                fail("expected non-nil result");
            }
            return;
        }
        if (expect == "nil") {
            if (!isNilValue(invokeResult)) {
                fail("expected nil result but got " + describeValue(invokeResult));
            }
            return;
        }
        if (expect == "truthy") {
            if (!isTruthy(invokeResult)) {
                fail("expected truthy result but got " + describeValue(invokeResult));
            }
            return;
        }
        if (expect == "falsey" || expect == "falsy") {
            if (isTruthy(invokeResult)) {
                fail("expected falsey result but got " + describeValue(invokeResult));
            }
            return;
        }

        throw std::runtime_error(
            "Fixture script " + op + " op unsupported expect value '" + expect +
            "' at index " + std::to_string(stepIndex)
        );
    }

    if (expectNode.is_object()) {
        const auto equalsIt = expectNode.find("equals");
        if (equalsIt == expectNode.end()) {
            throw std::runtime_error(
                "Fixture script " + op + " op requires supported expect object at index " +
                std::to_string(stepIndex)
            );
        }

        const Value expected = resolveFixtureScriptValue(*equalsIt, ctx);
        if (!valuesEqual(invokeResult, expected)) {
            fail(
                "expected result equal to " + describeValue(expected) +
                " but got " + describeValue(invokeResult)
            );
        }
        return;
    }

    throw std::runtime_error(
        "Fixture script " + op + " op requires string or object expect at index " +
        std::to_string(stepIndex)
    );
}

Value resolveValueWithDefault(const nlohmann::json& fromNode,
                              const nlohmann::json& containerNode,
                              const FixtureScriptContext& ctx) {
    if (!fromNode.is_null()) {
        return resolveFixtureScriptValue(fromNode, ctx);
    }
    if (containerNode.is_object() && containerNode.contains("default")) {
        return resolveFixtureScriptValue(containerNode["default"], ctx);
    }
    return Value::Nil();
}

Value resolveFixtureScriptValue(const nlohmann::json& node, const FixtureScriptContext& ctx) {
    if (node.is_array()) {
        Array array;
        array.reserve(node.size());
        for (const auto& item : node) {
            array.push_back(resolveFixtureScriptValue(item, ctx));
        }
        return Value::Arr(std::move(array));
    }

    if (!node.is_object()) {
        return jsonToValue(node);
    }

    const auto fromIt = node.find("from");
    if (fromIt != node.end() && fromIt->is_string()) {
        const std::string source = fromIt->get<std::string>();

        if (source == "pluginName") {
            validateResolverAllowedFields(node, source, {});
            Value value;
            value.v = ctx.pluginName;
            return value;
        }
        if (source == "commandName") {
            validateResolverAllowedFields(node, source, {});
            Value value;
            value.v = ctx.commandName;
            return value;
        }
        if (source == "argCount") {
            validateResolverAllowedFields(node, source, {});
            return Value::Int(static_cast<int64_t>(ctx.args.size()));
        }
        if (source == "args") {
            validateResolverAllowedFields(node, source, {});
            Array array;
            array.reserve(ctx.args.size());
            for (const auto& arg : ctx.args) {
                array.push_back(arg);
            }
            return Value::Arr(std::move(array));
        }
        if (source == "arg") {
            validateResolverAllowedFields(node, source, {"index", "default"});
            const int64_t index = requireResolverIntegerField(node, source, "index");
            if (index >= 0 && static_cast<size_t>(index) < ctx.args.size()) {
                return ctx.args[static_cast<size_t>(index)];
            }
            return resolveValueWithDefault(nlohmann::json{}, node, ctx);
        }
        if (source == "hasArg") {
            validateResolverAllowedFields(node, source, {"index"});
            const int64_t index = requireResolverIntegerField(node, source, "index");
            Value value;
            value.v = index >= 0 && static_cast<size_t>(index) < ctx.args.size();
            return value;
        }
        if (source == "param") {
            validateResolverAllowedFields(node, source, {"name", "default"});
            const std::string name = requireResolverStringField(node, source, "name");
            const auto it = ctx.parameters.find(name);
            if (it != ctx.parameters.end()) {
                return it->second;
            }
            return resolveValueWithDefault(nlohmann::json{}, node, ctx);
        }
        if (source == "local") {
            validateResolverAllowedFields(node, source, {"name", "default"});
            const std::string name = requireResolverStringField(node, source, "name");
            const auto it = ctx.locals.find(name);
            if (it != ctx.locals.end()) {
                return it->second;
            }
            return resolveValueWithDefault(nlohmann::json{}, node, ctx);
        }
        if (source == "hasParam") {
            validateResolverAllowedFields(node, source, {"name"});
            const std::string name = requireResolverStringField(node, source, "name");
            Value value;
            value.v = ctx.parameters.find(name) != ctx.parameters.end();
            return value;
        }
        if (source == "paramKeys") {
            validateResolverAllowedFields(node, source, {});
            std::vector<std::string> keys;
            keys.reserve(ctx.parameters.size());
            for (const auto& [key, paramValue] : ctx.parameters) {
                (void)paramValue;
                keys.push_back(key);
            }
            std::sort(keys.begin(), keys.end());
            Array array;
            array.reserve(keys.size());
            for (const auto& key : keys) {
                Value value;
                value.v = key;
                array.push_back(std::move(value));
            }
            return Value::Arr(std::move(array));
        }
        if (source == "equals") {
            validateResolverAllowedFields(node, source, {"left", "right"});
            const auto leftIt = node.find("left");
            const auto rightIt = node.find("right");
            if (leftIt == node.end() || rightIt == node.end()) {
                throwFixtureResolverError("equals requires left and right");
            }
            const Value lhs =
                resolveFixtureScriptValue(*leftIt, ctx);
            const Value rhs =
                resolveFixtureScriptValue(*rightIt, ctx);
            Value value;
            value.v = valuesEqual(lhs, rhs);
            return value;
        }
        if (source == "coalesce") {
            validateResolverAllowedFields(node, source, {"values", "default"});
            const auto& values = requireResolverArrayField(node, source, "values");
            for (const auto& valueNode : values) {
                const Value candidate = resolveFixtureScriptValue(valueNode, ctx);
                if (!std::holds_alternative<std::monostate>(candidate.v)) {
                    return candidate;
                }
            }
            return resolveValueWithDefault(nlohmann::json{}, node, ctx);
        }
        if (source == "concat") {
            validateResolverAllowedFields(node, source, {"parts"});
            std::string out;
            const auto& parts = requireResolverArrayField(node, source, "parts");
            for (const auto& part : parts) {
                out += valueToString(resolveFixtureScriptValue(part, ctx));
            }
            Value value;
            value.v = std::move(out);
            return value;
        }
        if (source == "length") {
            const auto valueIt = node.find("value");
            const Value target =
                valueIt != node.end() ? resolveFixtureScriptValue(*valueIt, ctx) : Value::Nil();

            int64_t length = -1;
            if (std::holds_alternative<std::monostate>(target.v)) {
                length = 0;
            } else if (const auto* text = std::get_if<std::string>(&target.v)) {
                length = static_cast<int64_t>(text->size());
            } else if (const auto* array = std::get_if<Array>(&target.v)) {
                length = static_cast<int64_t>(array->size());
            } else if (const auto* object = std::get_if<Object>(&target.v)) {
                length = static_cast<int64_t>(object->size());
            }

            if (length >= 0) {
                return Value::Int(length);
            }
            return resolveValueWithDefault(nlohmann::json{}, node, ctx);
        }
        if (source == "contains") {
            const auto containerIt = node.find("container");
            const auto valueIt = node.find("value");
            const Value container =
                containerIt != node.end() ? resolveFixtureScriptValue(*containerIt, ctx) : Value::Nil();
            const Value needle =
                valueIt != node.end() ? resolveFixtureScriptValue(*valueIt, ctx) : Value::Nil();

            bool contains = false;
            if (const auto* array = std::get_if<Array>(&container.v)) {
                for (const auto& entry : *array) {
                    if (valuesEqual(entry, needle)) {
                        contains = true;
                        break;
                    }
                }
            } else if (const auto* text = std::get_if<std::string>(&container.v)) {
                contains = text->find(valueToString(needle)) != std::string::npos;
            } else if (const auto* object = std::get_if<Object>(&container.v)) {
                if (const auto* key = std::get_if<std::string>(&needle.v)) {
                    contains = object->find(*key) != object->end();
                }
            }

            Value value;
            value.v = contains;
            return value;
        }
        if (source == "greaterThan") {
            const auto leftIt = node.find("left");
            const auto rightIt = node.find("right");
            const Value lhs =
                leftIt != node.end() ? resolveFixtureScriptValue(*leftIt, ctx) : Value::Nil();
            const Value rhs =
                rightIt != node.end() ? resolveFixtureScriptValue(*rightIt, ctx) : Value::Nil();

            const bool lhsNumeric =
                std::holds_alternative<int64_t>(lhs.v) || std::holds_alternative<double>(lhs.v);
            const bool rhsNumeric =
                std::holds_alternative<int64_t>(rhs.v) || std::holds_alternative<double>(rhs.v);

            Value value;
            value.v = lhsNumeric && rhsNumeric && (asDouble(lhs) > asDouble(rhs));
            return value;
        }
        if (source == "lessThan") {
            const auto leftIt = node.find("left");
            const auto rightIt = node.find("right");
            const Value lhs =
                leftIt != node.end() ? resolveFixtureScriptValue(*leftIt, ctx) : Value::Nil();
            const Value rhs =
                rightIt != node.end() ? resolveFixtureScriptValue(*rightIt, ctx) : Value::Nil();

            const bool lhsNumeric =
                std::holds_alternative<int64_t>(lhs.v) || std::holds_alternative<double>(lhs.v);
            const bool rhsNumeric =
                std::holds_alternative<int64_t>(rhs.v) || std::holds_alternative<double>(rhs.v);

            Value value;
            value.v = lhsNumeric && rhsNumeric && (asDouble(lhs) < asDouble(rhs));
            return value;
        }
        if (source == "not") {
            const auto valueIt = node.find("value");
            const Value target =
                valueIt != node.end() ? resolveFixtureScriptValue(*valueIt, ctx) : Value::Nil();
            Value value;
            value.v = !isTruthy(target);
            return value;
        }
        if (source == "all") {
            if (const auto valuesIt = node.find("values");
                valuesIt != node.end() && valuesIt->is_array()) {
                bool allTrue = true;
                for (const auto& valueNode : *valuesIt) {
                    if (!isTruthy(resolveFixtureScriptValue(valueNode, ctx))) {
                        allTrue = false;
                        break;
                    }
                }
                Value value;
                value.v = allTrue;
                return value;
            }
            return resolveValueWithDefault(nlohmann::json{}, node, ctx);
        }
        if (source == "any") {
            if (const auto valuesIt = node.find("values");
                valuesIt != node.end() && valuesIt->is_array()) {
                bool anyTrue = false;
                for (const auto& valueNode : *valuesIt) {
                    if (isTruthy(resolveFixtureScriptValue(valueNode, ctx))) {
                        anyTrue = true;
                        break;
                    }
                }
                Value value;
                value.v = anyTrue;
                return value;
            }
            return resolveValueWithDefault(nlohmann::json{}, node, ctx);
        }

        throwFixtureResolverError("unknown source '" + source + "'");
    }

    Object object;
    for (const auto& [key, valueNode] : node.items()) {
        object[key] = resolveFixtureScriptValue(valueNode, ctx);
    }
    return Value::Obj(std::move(object));
}

FixtureScriptResult runFixtureScriptSteps(const nlohmann::json& script,
                                          const std::string& pluginName,
                                          const std::string& commandName,
                                          const std::vector<Value>& args,
                                          const std::unordered_map<std::string, Value>& parameters,
                                          Object& locals,
                                          const FixtureCommandInvoker& commandInvoker,
                                          const FixtureCommandByNameInvoker& commandByNameInvoker) {
    FixtureScriptResult out;
    if (!script.is_array()) {
        return out;
    }

    for (size_t stepIndex = 0; stepIndex < script.size(); ++stepIndex) {
        const auto& step = script[stepIndex];
        if (!step.is_object()) {
            throw std::runtime_error(
                "Fixture script step must be an object at index " + std::to_string(stepIndex)
            );
        }

        const std::string op = step.value("op", "");
        if (op.empty()) {
            throw std::runtime_error(
                "Fixture script step missing op at index " + std::to_string(stepIndex)
            );
        }
        FixtureScriptContext ctx{pluginName, commandName, args, parameters, locals};

        if (op == "set") {
            const std::string key = step.value("key", "");
            if (key.empty()) {
                throw std::runtime_error(
                    "Fixture script set op requires key at index " + std::to_string(stepIndex)
                );
            }
            const auto valueIt = step.find("value");
            locals[key] = valueIt != step.end()
                              ? resolveFixtureScriptValue(*valueIt, ctx)
                              : Value::Nil();
            continue;
        }

        if (op == "append") {
            const std::string key = step.value("key", "");
            if (key.empty()) {
                throw std::runtime_error(
                    "Fixture script append op requires key at index " + std::to_string(stepIndex)
                );
            }
            Array* outArray = nullptr;
            auto localIt = locals.find(key);
            if (localIt == locals.end()) {
                locals[key] = Value::Arr(Array{});
                outArray = &std::get<Array>(locals[key].v);
            } else if (auto* existing = std::get_if<Array>(&localIt->second.v)) {
                outArray = existing;
            } else {
                Array array;
                array.push_back(localIt->second);
                localIt->second = Value::Arr(std::move(array));
                outArray = &std::get<Array>(localIt->second.v);
            }

            const auto valueIt = step.find("value");
            outArray->push_back(valueIt != step.end()
                                    ? resolveFixtureScriptValue(*valueIt, ctx)
                                    : Value::Nil());
            continue;
        }

        if (op == "if") {
            const auto conditionIt = step.find("condition");
            const Value condition =
                conditionIt != step.end() ? resolveFixtureScriptValue(*conditionIt, ctx) : Value::Nil();
            const bool takeThen = isTruthy(condition);
            const auto branchIt = step.find(takeThen ? "then" : "else");
            if (branchIt != step.end() && !branchIt->is_array()) {
                throw std::runtime_error(
                    "Fixture script if branch must be an array at index " +
                    std::to_string(stepIndex)
                );
            }
            if (branchIt != step.end()) {
                const auto branchResult = runFixtureScriptSteps(
                    *branchIt,
                    pluginName,
                    commandName,
                    args,
                    parameters,
                    locals,
                    commandInvoker,
                    commandByNameInvoker
                );
                if (branchResult.returned) {
                    return branchResult;
                }
            }
            continue;
        }

        if (op == "invoke" || op == "invokeByName") {
            std::vector<Value> invokeArgs;
            if (const auto argsIt = step.find("args");
                argsIt != step.end()) {
                if (!argsIt->is_array()) {
                    throw std::runtime_error(
                        "Fixture script " + op + " op requires array args at index " +
                        std::to_string(stepIndex)
                    );
                }
                invokeArgs.reserve(argsIt->size());
                for (const auto& argNode : *argsIt) {
                    invokeArgs.push_back(resolveFixtureScriptValue(argNode, ctx));
                }
            }

            Value invokeResult = Value::Nil();
            std::string target;
            if (op == "invoke") {
                if (!commandInvoker) {
                    throw std::runtime_error(
                        "Fixture script invoke op unavailable at index " +
                        std::to_string(stepIndex)
                    );
                }

                const auto commandIt = step.find("command");
                if (commandIt == step.end()) {
                    throw std::runtime_error(
                        "Fixture script invoke op requires command at index " +
                        std::to_string(stepIndex)
                    );
                }
                const Value commandValue = resolveFixtureScriptValue(*commandIt, ctx);
                if (!std::holds_alternative<std::string>(commandValue.v) ||
                    std::get<std::string>(commandValue.v).empty()) {
                    throw std::runtime_error(
                        "Fixture script invoke op requires string command at index " +
                        std::to_string(stepIndex)
                    );
                }
                const std::string targetCommand = std::get<std::string>(commandValue.v);

                std::string targetPlugin = pluginName;
                if (const auto pluginIt = step.find("plugin");
                    pluginIt != step.end()) {
                    const Value pluginValue = resolveFixtureScriptValue(*pluginIt, ctx);
                    if (!std::holds_alternative<std::string>(pluginValue.v) ||
                        std::get<std::string>(pluginValue.v).empty()) {
                        throw std::runtime_error(
                            "Fixture script invoke op requires string plugin at index " +
                            std::to_string(stepIndex)
                        );
                    }
                    targetPlugin = std::get<std::string>(pluginValue.v);
                }

                target = targetPlugin + "_" + targetCommand;
                invokeResult = commandInvoker(targetPlugin, targetCommand, invokeArgs);
            } else {
                if (!commandByNameInvoker) {
                    throw std::runtime_error(
                        "Fixture script invokeByName op unavailable at index " +
                        std::to_string(stepIndex)
                    );
                }

                const auto nameIt = step.find("name");
                if (nameIt == step.end()) {
                    throw std::runtime_error(
                        "Fixture script invokeByName op requires name at index " +
                        std::to_string(stepIndex)
                    );
                }
                const Value nameValue = resolveFixtureScriptValue(*nameIt, ctx);
                if (!std::holds_alternative<std::string>(nameValue.v) ||
                    std::get<std::string>(nameValue.v).empty()) {
                    throw std::runtime_error(
                        "Fixture script invokeByName op requires string name at index " +
                        std::to_string(stepIndex)
                    );
                }
                target = std::get<std::string>(nameValue.v);
                invokeResult = commandByNameInvoker(target, invokeArgs);
            }

            if (const auto storeIt = step.find("store");
                storeIt != step.end()) {
                if (!storeIt->is_string() || storeIt->get<std::string>().empty()) {
                    throw std::runtime_error(
                        "Fixture script " + op + " op requires string store at index " +
                        std::to_string(stepIndex)
                    );
                }
                locals[storeIt->get<std::string>()] = invokeResult;
            }

            if (const auto expectIt = step.find("expect");
                expectIt != step.end()) {
                enforceFixtureInvokeExpectation(
                    *expectIt,
                    invokeResult,
                    ctx,
                    op,
                    target,
                    stepIndex
                );
            }

            continue;
        }

        if (op == "return") {
            const auto valueIt = step.find("value");
            out.returned = true;
            out.value = valueIt != step.end()
                            ? resolveFixtureScriptValue(*valueIt, ctx)
                            : Value::Nil();
            return out;
        }

        if (op == "returnObject") {
            out.returned = true;
            out.value = Value::Obj(locals);
            return out;
        }

        if (op == "error") {
            const auto messageIt = step.find("message");
            const Value messageValue =
                messageIt != step.end() ? resolveFixtureScriptValue(*messageIt, ctx) : Value::Nil();
            std::string message = valueToString(messageValue);
            if (message.empty()) {
                message = "Fixture script error op triggered at index " +
                          std::to_string(stepIndex);
            }
            throw std::runtime_error(message);
        }

        throw std::runtime_error(
            "Unsupported fixture script op '" + op + "' at index " +
            std::to_string(stepIndex)
        );
    }

    return out;
}

Value executeFixtureScript(const nlohmann::json& script,
                           const std::string& pluginName,
                           const std::string& commandName,
                           const std::vector<Value>& args,
                           const std::unordered_map<std::string, Value>& parameters,
                           const FixtureCommandInvoker& commandInvoker,
                           const FixtureCommandByNameInvoker& commandByNameInvoker) {
    Object locals;
    const auto result =
        runFixtureScriptSteps(
            script,
            pluginName,
            commandName,
            args,
            parameters,
            locals,
            commandInvoker,
            commandByNameInvoker
        );
    if (result.returned) {
        return result.value;
    }
    if (!locals.empty()) {
        return Value::Obj(locals);
    }
    return Value::Nil();
}

} // namespace

// Static member definitions
std::unordered_map<std::string, CompatStatus> PluginManager::methodStatus_;
std::unordered_map<std::string, std::string> PluginManager::methodDeviations_;

// ============================================================================
// PluginManagerImpl
// ============================================================================

class PluginManagerImpl {
public:
    struct AsyncCommandTask {
        std::string pluginName;
        std::string commandName;
        std::vector<Value> args;
        std::function<void(const Value&)> callback;
        uint64_t sequence = 0;
    };

    // Plugin registry
    std::unordered_map<std::string, PluginInfo> plugins_;
    
    // Command registry: "pluginName_commandName" -> CommandInfo
    std::unordered_map<std::string, CommandInfo> commands_;
    
    // Parameter storage: pluginName -> (paramName -> value)
    std::unordered_map<std::string, std::unordered_map<std::string, Value>> parameters_;
    
    // Event handlers: event -> (handlerId -> handler)
    std::unordered_map<PluginManager::PluginEvent, std::unordered_map<int32_t, PluginManager::PluginEventHandler>> eventHandlers_;
    int32_t nextHandlerId_ = 1;
    
    // Execution context
    std::vector<PluginContext> contextStack_;
    
    // Error handling
    std::string lastError_;
    std::function<void(const std::string&, const std::string&, const std::string&)> errorHandler_;

    // Structured diagnostics for failure-path artifact export.
    std::vector<std::string> failureDiagnosticsJsonl_;
    uint64_t nextFailureDiagnosticSequence_ = 1;

    // Plugin source path tracking (used by reload)
    std::unordered_map<std::string, std::string> pluginSourcePaths_;

    // QuickJS runtime bridge for fixture-backed command execution
    QuickJSRuntime runtime_;
    bool runtimeInitialized_ = false;
    mutable std::recursive_mutex stateMutex_;

    // Async command queue (FIFO)
    std::mutex asyncMutex_;
    std::condition_variable asyncCv_;
    std::deque<AsyncCommandTask> asyncQueue_;
    std::thread asyncWorker_;
    bool stopAsyncWorker_ = false;
    uint64_t nextAsyncSequence_ = 1;

    PluginManagerImpl()
        : runtimeInitialized_(runtime_.initialize()) {}

    void enqueueAsyncTask(AsyncCommandTask task) {
        {
            std::lock_guard<std::mutex> lock(asyncMutex_);
            task.sequence = nextAsyncSequence_++;
            asyncQueue_.push_back(std::move(task));
        }
        asyncCv_.notify_one();
    }

    void appendFailureDiagnostic(const std::string& pluginName,
                                 const std::string& commandName,
                                 const std::string& operation,
                                 const std::string& message,
                                 const std::string& severity) {
        nlohmann::json diagnostic;
        diagnostic["seq"] = nextFailureDiagnosticSequence_++;
        diagnostic["ts"] = currentUtcTimestampIso8601();
        diagnostic["subsystem"] = "plugin_manager";
        diagnostic["event"] = "compat_failure";
        diagnostic["plugin"] = pluginName;
        diagnostic["command"] = commandName;
        diagnostic["operation"] = operation;
        diagnostic["message"] = message;
        diagnostic["severity"] = severity;
        failureDiagnosticsJsonl_.push_back(diagnostic.dump());

        constexpr size_t kMaxFailureDiagnostics = 2048;
        if (failureDiagnosticsJsonl_.size() > kMaxFailureDiagnostics) {
            failureDiagnosticsJsonl_.erase(failureDiagnosticsJsonl_.begin());
        }
    }

    void reportFailure(const std::string& pluginName,
                       const std::string& commandName,
                       const std::string& operation,
                       const std::string& message,
                       const std::string& severity = "HARD_FAIL") {
        lastError_ = message;
        appendFailureDiagnostic(pluginName, commandName, operation, message, severity);
        if (errorHandler_) {
            errorHandler_(pluginName, commandName, message);
        }
    }

    QuickJSContext* ensurePluginContext(const std::string& pluginName) {
        if (!runtimeInitialized_) {
            return nullptr;
        }
        if (const auto contextId = runtime_.getContextId(pluginName)) {
            return runtime_.getContext(*contextId);
        }
        const auto created = runtime_.createContext(pluginName, QuickJSConfig{});
        if (!created) {
            return nullptr;
        }
        return runtime_.getContext(*created);
    }

    QuickJSContext* getPluginContext(const std::string& pluginName) {
        if (!runtimeInitialized_) {
            return nullptr;
        }
        if (const auto contextId = runtime_.getContextId(pluginName)) {
            return runtime_.getContext(*contextId);
        }
        return nullptr;
    }

    void destroyPluginContext(const std::string& pluginName) {
        if (!runtimeInitialized_) {
            return;
        }
        if (const auto contextId = runtime_.getContextId(pluginName)) {
            runtime_.destroyContext(*contextId);
        }
    }
};

// ============================================================================
// PluginManager Implementation
// ============================================================================

PluginManager::PluginManager()
    : impl_(std::make_unique<PluginManagerImpl>())
{
    initializeMethodStatus();

    impl_->asyncWorker_ = std::thread([this]() {
        while (true) {
            PluginManagerImpl::AsyncCommandTask task;
            {
                std::unique_lock<std::mutex> lock(impl_->asyncMutex_);
                impl_->asyncCv_.wait(lock, [this]() {
                    return impl_->stopAsyncWorker_ || !impl_->asyncQueue_.empty();
                });

                if (impl_->stopAsyncWorker_ && impl_->asyncQueue_.empty()) {
                    return;
                }

                task = std::move(impl_->asyncQueue_.front());
                impl_->asyncQueue_.pop_front();
            }

            const Value result = executeCommand(task.pluginName, task.commandName, task.args);
            if (task.callback) {
                task.callback(result);
            }
        }
    });
}

PluginManager::~PluginManager() {
    {
        std::lock_guard<std::mutex> lock(impl_->asyncMutex_);
        impl_->stopAsyncWorker_ = true;
    }
    impl_->asyncCv_.notify_all();
    if (impl_->asyncWorker_.joinable()) {
        impl_->asyncWorker_.join();
    }
}

PluginManager& PluginManager::instance() {
    static PluginManager instance;
    return instance;
}

void PluginManager::initializeMethodStatus() {
    if (methodStatus_.empty()) {
        // Plugin lifecycle
        methodStatus_["loadPlugin"] = CompatStatus::FULL;
        methodStatus_["unloadPlugin"] = CompatStatus::FULL;
        methodStatus_["reloadPlugin"] = CompatStatus::FULL;
        methodStatus_["loadPluginsFromDirectory"] = CompatStatus::FULL;
        methodStatus_["unloadAllPlugins"] = CompatStatus::FULL;
        
        // Plugin registration
        methodStatus_["registerPlugin"] = CompatStatus::FULL;
        methodStatus_["unregisterPlugin"] = CompatStatus::FULL;
        methodStatus_["isPluginLoaded"] = CompatStatus::FULL;
        methodStatus_["getPluginInfo"] = CompatStatus::FULL;
        methodStatus_["getLoadedPlugins"] = CompatStatus::FULL;
        
        // Command registration
        methodStatus_["registerCommand"] = CompatStatus::FULL;
        methodStatus_["unregisterCommand"] = CompatStatus::FULL;
        methodStatus_["unregisterAllCommands"] = CompatStatus::FULL;
        methodStatus_["hasCommand"] = CompatStatus::FULL;
        methodStatus_["getCommandInfo"] = CompatStatus::FULL;
        methodStatus_["getPluginCommands"] = CompatStatus::FULL;
        
        // Command execution
        methodStatus_["executeCommand"] = CompatStatus::FULL;
        methodStatus_["executeCommandByName"] = CompatStatus::FULL;
        methodStatus_["executeCommandAsync"] = CompatStatus::FULL;
        
        // Parameter management
        methodStatus_["setParameter"] = CompatStatus::FULL;
        methodStatus_["getParameter"] = CompatStatus::FULL;
        methodStatus_["getParameters"] = CompatStatus::FULL;
        methodStatus_["parseParameters"] = CompatStatus::FULL;
        
        // Dependencies
        methodStatus_["checkDependencies"] = CompatStatus::FULL;
        methodStatus_["getMissingDependencies"] = CompatStatus::FULL;
        methodStatus_["getDependents"] = CompatStatus::FULL;
        
        // Event hooks
        methodStatus_["registerEventHandler"] = CompatStatus::FULL;
        methodStatus_["unregisterEventHandler"] = CompatStatus::FULL;
        
        // Execution context
        methodStatus_["getCurrentContext"] = CompatStatus::FULL;
        methodStatus_["isExecuting"] = CompatStatus::FULL;
        methodStatus_["getCurrentPlugin"] = CompatStatus::FULL;
        
        // Error handling
        methodStatus_["getLastError"] = CompatStatus::FULL;
        methodStatus_["clearLastError"] = CompatStatus::FULL;
        methodStatus_["setErrorHandler"] = CompatStatus::FULL;
        methodStatus_["exportFailureDiagnosticsJsonl"] = CompatStatus::FULL;
        methodStatus_["clearFailureDiagnostics"] = CompatStatus::FULL;
    }
}

// ============================================================================
// Plugin Lifecycle
// ============================================================================

bool PluginManager::loadPlugin(const std::string& path) {
    std::lock_guard<std::recursive_mutex> lock(impl_->stateMutex_);
    impl_->lastError_.clear();
    const std::filesystem::path pluginPath(path);
    const std::string pluginHint = pluginPath.has_stem() ? pluginPath.stem().string() : path;
    const auto reportLoadFailure = [&](const std::string& pluginName,
                                       const std::string& commandName,
                                       const std::string& operation,
                                       const std::string& message,
                                       const std::string& severity = "HARD_FAIL") {
        impl_->reportFailure(pluginName, commandName, operation, message, severity);
    };

    // Fixture path: JSON files contain executable plugin command fixtures.
    if (pluginPath.has_extension() && pluginPath.extension() == ".json" &&
        std::filesystem::exists(pluginPath)) {
        std::ifstream input(pluginPath);
        if (!input.is_open()) {
            reportLoadFailure(
                pluginHint,
                "",
                "load_plugin_fixture_open",
                "Failed to open plugin fixture: " + path
            );
            return false;
        }

        nlohmann::json fixture = nlohmann::json::parse(input, nullptr, false);
        if (fixture.is_discarded() || !fixture.is_object()) {
            reportLoadFailure(
                pluginHint,
                "",
                "load_plugin_fixture_parse",
                "Invalid plugin fixture JSON: " + path
            );
            return false;
        }

        if (const auto fixtureNameIt = fixture.find("name");
            fixtureNameIt != fixture.end() && !fixtureNameIt->is_string()) {
            reportLoadFailure(
                pluginHint,
                "",
                "load_plugin_fixture_name",
                "Fixture plugin name must be string: " + path
            );
            return false;
        }
        std::string name = fixture.value("name", pluginPath.stem().string());
        if (name.empty()) {
            reportLoadFailure(
                pluginHint,
                "",
                "load_plugin_fixture_name",
                "Fixture plugin name cannot be empty: " + path
            );
            return false;
        }
        if (impl_->plugins_.find(name) != impl_->plugins_.end()) {
            reportLoadFailure(
                name,
                "",
                "load_plugin_duplicate",
                "Plugin already registered: " + name
            );
            return false;
        }

        PluginInfo info;
        info.name = name;
        info.version = fixture.value("version", "");
        info.author = fixture.value("author", "");
        info.description = fixture.value("description", "");
        info.enabled = fixture.value("enabled", true);
        info.loaded = true;
        if (const auto depsIt = fixture.find("dependencies");
            depsIt != fixture.end()) {
            if (!depsIt->is_array()) {
                reportLoadFailure(
                    name,
                    "",
                    "load_plugin_dependencies",
                    "Fixture plugin 'dependencies' must be array: " + path
                );
                return false;
            }
            for (size_t depIndex = 0; depIndex < depsIt->size(); ++depIndex) {
                const auto& dep = (*depsIt)[depIndex];
                if (!dep.is_string()) {
                    reportLoadFailure(
                        name,
                        "",
                        "load_plugin_dependency_entry",
                        "Fixture plugin dependency must be string at index " +
                            std::to_string(depIndex) + ": " + path
                    );
                    return false;
                }
                info.dependencies.push_back(dep.get<std::string>());
            }
        }
        impl_->plugins_[name] = std::move(info);
        impl_->pluginSourcePaths_[name] = path;

        const auto rollbackFixturePlugin = [this, &name]() {
            unregisterAllCommands(name);
            impl_->parameters_.erase(name);
            impl_->plugins_.erase(name);
            impl_->pluginSourcePaths_.erase(name);
            impl_->destroyPluginContext(name);
        };

        if (const auto paramsIt = fixture.find("parameters");
            paramsIt != fixture.end()) {
            if (!paramsIt->is_object()) {
                reportLoadFailure(
                    name,
                    "",
                    "load_plugin_parameters",
                    "Fixture plugin 'parameters' must be object: " + path
                );
                rollbackFixturePlugin();
                return false;
            }
            for (const auto& [key, paramValue] : paramsIt->items()) {
                setParameter(name, key, jsonToValue(paramValue));
            }
        }

        if (const auto cmdsIt = fixture.find("commands");
            cmdsIt != fixture.end()) {
            if (!cmdsIt->is_array()) {
                reportLoadFailure(
                    name,
                    "",
                    "load_plugin_commands",
                    "Fixture plugin 'commands' must be array: " + path
                );
                rollbackFixturePlugin();
                return false;
            }
            for (size_t commandIndex = 0; commandIndex < cmdsIt->size(); ++commandIndex) {
                const auto& cmd = (*cmdsIt)[commandIndex];
                if (!cmd.is_object()) {
                    reportLoadFailure(
                        name,
                        "",
                        "load_plugin_command_shape",
                        "Fixture command entry must be an object at index " +
                            std::to_string(commandIndex)
                    );
                    rollbackFixturePlugin();
                    return false;
                }
                if (const auto commandNameIt = cmd.find("name");
                    commandNameIt != cmd.end() && !commandNameIt->is_string()) {
                    reportLoadFailure(
                        name,
                        "",
                        "load_plugin_command_name",
                        "Fixture command name must be string at index " +
                            std::to_string(commandIndex)
                    );
                    rollbackFixturePlugin();
                    return false;
                }
                const std::string commandName =
                    cmd.contains("name") && cmd["name"].is_string()
                        ? cmd["name"].get<std::string>()
                        : "";
                if (commandName.empty()) {
                    reportLoadFailure(
                        name,
                        "",
                        "load_plugin_command_name",
                        "Fixture command name cannot be empty at index " +
                            std::to_string(commandIndex)
                    );
                    rollbackFixturePlugin();
                    return false;
                }

                const bool hasJs = cmd.contains("js");
                const bool hasScript = cmd.contains("script");
                if (hasJs && !cmd["js"].is_string()) {
                    reportLoadFailure(
                        name,
                        commandName,
                        "load_plugin_js_payload",
                        "Fixture JS command requires string 'js' payload: " + commandName
                    );
                    rollbackFixturePlugin();
                    return false;
                }
                if (hasScript && !cmd["script"].is_array()) {
                    reportLoadFailure(
                        name,
                        commandName,
                        "load_plugin_script_payload",
                        "Fixture script command requires array 'script' payload: " + commandName
                    );
                    rollbackFixturePlugin();
                    return false;
                }
                if (hasJs && hasScript) {
                    reportLoadFailure(
                        name,
                        commandName,
                        "load_plugin_command_mode",
                        "Fixture command cannot declare both 'js' and 'script': " + commandName
                    );
                    rollbackFixturePlugin();
                    return false;
                }

                if (const auto dropContextIt = cmd.find("dropContextBeforeCall");
                    dropContextIt != cmd.end() && !dropContextIt->is_boolean()) {
                    reportLoadFailure(
                        name,
                        commandName,
                        "load_plugin_drop_context_flag",
                        "Fixture command 'dropContextBeforeCall' must be boolean: " + commandName
                    );
                    rollbackFixturePlugin();
                    return false;
                }

                const bool dropContextBeforeCall = cmd.value("dropContextBeforeCall", false);

                if (const auto descriptionIt = cmd.find("description");
                    descriptionIt != cmd.end() && !descriptionIt->is_string()) {
                    reportLoadFailure(
                        name,
                        commandName,
                        "load_plugin_command_description",
                        "Fixture command 'description' must be string: " + commandName
                    );
                    rollbackFixturePlugin();
                    return false;
                }
                const std::string description =
                    cmd.contains("description") && cmd["description"].is_string()
                        ? cmd["description"].get<std::string>()
                        : "";
                if (const auto jsIt = cmd.find("js");
                    jsIt != cmd.end() && jsIt->is_string()) {
                    QuickJSContext* pluginContext = impl_->ensurePluginContext(name);
                    if (!pluginContext) {
                        reportLoadFailure(
                            name,
                            commandName,
                            "load_plugin_quickjs_context",
                            "Failed to initialize QuickJS context for plugin: " + name
                        );
                        rollbackFixturePlugin();
                        return false;
                    }

                    if (const auto entryIt = cmd.find("entry");
                        entryIt != cmd.end() && !entryIt->is_string()) {
                        reportLoadFailure(
                            name,
                            commandName,
                            "load_plugin_js_entry",
                            "Fixture JS command requires string 'entry' payload: " + commandName
                        );
                        rollbackFixturePlugin();
                        return false;
                    }
                    const std::string entryPoint = cmd.value("entry", commandName);
                    if (entryPoint.empty()) {
                        reportLoadFailure(
                            name,
                            commandName,
                            "load_plugin_js_entry",
                            "Fixture JS command entry cannot be empty: " + commandName
                        );
                        rollbackFixturePlugin();
                        return false;
                    }

                    const std::string jsSource = jsIt->get<std::string>();
                    const ScriptResult evalResult =
                        pluginContext->eval(jsSource, path + "#" + name + "." + commandName);
                    if (!evalResult.success) {
                        const std::string message =
                            !evalResult.error.empty()
                                ? evalResult.error
                                : ("Failed to evaluate fixture JS for command: " + commandName);
                        reportLoadFailure(
                            name,
                            commandName,
                            "load_plugin_js_eval",
                            message,
                            compatSeverityToString(evalResult.severity)
                        );
                        rollbackFixturePlugin();
                        return false;
                    }

                    if (!registerCommand(
                        name,
                        commandName,
                        [this, name, commandName, entryPoint, dropContextBeforeCall](const std::vector<Value>& args) -> Value {
                            if (dropContextBeforeCall) {
                                impl_->destroyPluginContext(name);
                            }
                            QuickJSContext* context = impl_->getPluginContext(name);
                            if (!context) {
                                impl_->reportFailure(
                                    name,
                                    commandName,
                                    "execute_command_quickjs_context_missing",
                                    "QuickJS context missing for plugin: " + name
                                );
                                return Value::Nil();
                            }

                            const ScriptResult result = context->call(entryPoint, args);
                            if (!result.success) {
                                const std::string message =
                                    !result.error.empty()
                                        ? result.error
                                        : ("QuickJS command failed: " + name + "_" + commandName);
                                impl_->reportFailure(
                                    name,
                                    commandName,
                                    "execute_command_quickjs_call",
                                    message,
                                    compatSeverityToString(result.severity)
                                );
                                return Value::Nil();
                            }
                            return result.value;
                        },
                        description)) {
                        reportLoadFailure(
                            name,
                            commandName,
                            "load_plugin_register_command",
                            impl_->lastError_.empty()
                                ? ("Failed registering command: " + commandName)
                                : impl_->lastError_
                        );
                        rollbackFixturePlugin();
                        return false;
                    }
                    continue;
                }

                if (const auto scriptIt = cmd.find("script");
                    scriptIt != cmd.end() && scriptIt->is_array()) {
                    QuickJSContext* pluginContext = impl_->ensurePluginContext(name);
                    if (!pluginContext) {
                        reportLoadFailure(
                            name,
                            commandName,
                            "load_plugin_quickjs_context",
                            "Failed to initialize QuickJS context for plugin: " + name
                        );
                        rollbackFixturePlugin();
                        return false;
                    }

                    const nlohmann::json script = *scriptIt;
                    const std::string pluginForCommand = name;
                    const std::string commandForCommand = commandName;
                    if (!pluginContext->registerFunction(
                            commandForCommand,
                            [this, script, pluginForCommand, commandForCommand](const std::vector<Value>& args) -> Value {
                                static const std::unordered_map<std::string, Value> emptyParams;
                                const auto paramsIt = impl_->parameters_.find(pluginForCommand);
                                const auto& params =
                                    paramsIt != impl_->parameters_.end() ? paramsIt->second : emptyParams;
                                return executeFixtureScript(
                                    script,
                                    pluginForCommand,
                                    commandForCommand,
                                    args,
                                    params,
                                    [this](const std::string& targetPlugin,
                                           const std::string& targetCommand,
                                           const std::vector<Value>& targetArgs) -> Value {
                                        return executeCommand(targetPlugin, targetCommand, targetArgs);
                                    },
                                    [this](const std::string& fullName,
                                           const std::vector<Value>& targetArgs) -> Value {
                                        return executeCommandByName(fullName, targetArgs);
                                    }
                                );
                            })) {
                        reportLoadFailure(
                            name,
                            commandName,
                            "load_plugin_register_script_fn",
                            "Failed to register QuickJS fixture function for command: " + commandName
                        );
                        rollbackFixturePlugin();
                        return false;
                    }

                    if (!registerCommand(
                        pluginForCommand,
                        commandForCommand,
                        [this, pluginForCommand, commandForCommand, dropContextBeforeCall](const std::vector<Value>& args) -> Value {
                            if (dropContextBeforeCall) {
                                impl_->destroyPluginContext(pluginForCommand);
                            }
                            QuickJSContext* context = impl_->getPluginContext(pluginForCommand);
                            if (!context) {
                                impl_->reportFailure(
                                    pluginForCommand,
                                    commandForCommand,
                                    "execute_command_quickjs_context_missing",
                                    "QuickJS context missing for plugin: " + pluginForCommand
                                );
                                return Value::Nil();
                            }

                            const ScriptResult result = context->call(commandForCommand, args);
                            if (!result.success) {
                                const std::string message =
                                    !result.error.empty()
                                        ? result.error
                                        : ("QuickJS command failed: " + pluginForCommand + "_" + commandForCommand);
                                impl_->reportFailure(
                                    pluginForCommand,
                                    commandForCommand,
                                    "execute_command_quickjs_call",
                                    message,
                                    compatSeverityToString(result.severity)
                                );
                                return Value::Nil();
                            }
                            return result.value;
                        },
                        description)) {
                        reportLoadFailure(
                            name,
                            commandName,
                            "load_plugin_register_command",
                            impl_->lastError_.empty()
                                ? ("Failed registering command: " + commandName)
                                : impl_->lastError_
                        );
                        rollbackFixturePlugin();
                        return false;
                    }
                    continue;
                }

                if (const auto modeIt = cmd.find("mode");
                    modeIt != cmd.end() && !modeIt->is_string()) {
                    reportLoadFailure(
                        name,
                        commandName,
                        "load_plugin_command_mode",
                        "Fixture command 'mode' must be string: " + commandName
                    );
                    rollbackFixturePlugin();
                    return false;
                }
                const std::string mode =
                    cmd.contains("mode") && cmd["mode"].is_string()
                        ? cmd["mode"].get<std::string>()
                        : "const";
                if (mode != "const" && mode != "arg_count") {
                    reportLoadFailure(
                        name,
                        commandName,
                        "load_plugin_command_mode",
                        "Fixture command 'mode' unsupported: " + mode + " for " + commandName
                    );
                    rollbackFixturePlugin();
                    return false;
                }
                if (mode == "arg_count") {
                    if (!registerCommand(
                        name,
                        commandName,
                        [](const std::vector<Value>& args) -> Value {
                            return Value::Int(static_cast<int64_t>(args.size()));
                        },
                        description)) {
                        reportLoadFailure(
                            name,
                            commandName,
                            "load_plugin_register_command",
                            impl_->lastError_.empty()
                                ? ("Failed registering command: " + commandName)
                                : impl_->lastError_
                        );
                        rollbackFixturePlugin();
                        return false;
                    }
                    continue;
                }

                const Value constantValue =
                    cmd.contains("result") ? jsonToValue(cmd["result"]) : Value::Nil();
                if (!registerCommand(
                    name,
                    commandName,
                    [constantValue](const std::vector<Value>&) -> Value {
                        return constantValue;
                    },
                    description)) {
                    reportLoadFailure(
                        name,
                        commandName,
                        "load_plugin_register_command",
                        impl_->lastError_.empty()
                            ? ("Failed registering command: " + commandName)
                            : impl_->lastError_
                    );
                    rollbackFixturePlugin();
                    return false;
                }
            }
        }

        triggerEvent(name, PluginEvent::ON_LOAD);
        return true;
    }

    // Legacy fallback used by existing tests: derive name from path only.
    const std::string name = pluginNameFromPath(path);
    if (name.empty()) {
        reportLoadFailure("", "", "load_plugin_name", "Plugin name cannot be empty");
        return false;
    }
    if (impl_->plugins_.find(name) != impl_->plugins_.end()) {
        reportLoadFailure(name, "", "load_plugin_duplicate", "Plugin already registered: " + name);
        return false;
    }

    PluginInfo info;
    info.name = name;
    info.loaded = true;
    info.enabled = true;

    impl_->plugins_[name] = std::move(info);
    impl_->pluginSourcePaths_[name] = path;
    if (pluginPath.has_extension() && pluginPath.extension() == ".js") {
        if (!impl_->ensurePluginContext(name)) {
            impl_->plugins_.erase(name);
            impl_->pluginSourcePaths_.erase(name);
            reportLoadFailure(
                name,
                "",
                "load_plugin_quickjs_context",
                "Failed to initialize QuickJS context for plugin: " + name
            );
            return false;
        }
    }
    triggerEvent(name, PluginEvent::ON_LOAD);

    return true;
}

bool PluginManager::unloadPlugin(const std::string& name) {
    std::lock_guard<std::recursive_mutex> lock(impl_->stateMutex_);
    impl_->lastError_.clear();
    
    auto it = impl_->plugins_.find(name);
    if (it == impl_->plugins_.end()) {
        impl_->lastError_ = "Plugin not found: " + name;
        return false;
    }
    
    // Unregister all commands for this plugin
    unregisterAllCommands(name);
    
    // Remove parameters
    impl_->parameters_.erase(name);
    impl_->pluginSourcePaths_.erase(name);
    impl_->destroyPluginContext(name);
    
    // Trigger event before removal
    triggerEvent(name, PluginEvent::ON_UNLOAD);
    
    // Remove plugin
    impl_->plugins_.erase(it);
    
    return true;
}

bool PluginManager::reloadPlugin(const std::string& name) {
    std::lock_guard<std::recursive_mutex> lock(impl_->stateMutex_);
    impl_->lastError_.clear();
    
    auto it = impl_->plugins_.find(name);
    if (it == impl_->plugins_.end()) {
        impl_->lastError_ = "Plugin not found: " + name;
        return false;
    }
    
    std::string path = name;
    const auto sourceIt = impl_->pluginSourcePaths_.find(name);
    if (sourceIt != impl_->pluginSourcePaths_.end() && !sourceIt->second.empty()) {
        path = sourceIt->second;
    }
    
    // Unload and reload
    if (!unloadPlugin(name)) {
        return false;
    }
    
    return loadPlugin(path);
}

int32_t PluginManager::loadPluginsFromDirectory(const std::string& directory) {
    std::lock_guard<std::recursive_mutex> lock(impl_->stateMutex_);
    impl_->lastError_.clear();

    std::error_code ec;
    const std::filesystem::path dirPath(directory);
    if (!std::filesystem::exists(dirPath, ec) || !std::filesystem::is_directory(dirPath, ec)) {
        impl_->reportFailure("", "", "load_plugins_directory", "Plugin directory not found: " + directory);
        return 0;
    }

    if (dirPath.filename().string().find(kFailDirectoryScanMarker) != std::string::npos) {
        impl_->reportFailure(
            "",
            "",
            "load_plugins_directory_scan",
            "Failed scanning plugin directory: " + directory
        );
        return 0;
    }

    std::vector<std::filesystem::path> candidates;
    std::filesystem::directory_iterator it(dirPath, ec);
    if (ec) {
        impl_->reportFailure(
            "",
            "",
            "load_plugins_directory_scan",
            "Failed scanning plugin directory: " + directory
        );
        return 0;
    }

    for (const auto& entry : it) {
        if (ec) {
            impl_->reportFailure("", "", "load_plugins_directory_scan", "Failed scanning plugin directory: " + directory);
            return 0;
        }
        if (entry.path().filename().string().find(kFailDirectoryEntryStatusMarker) !=
            std::string::npos) {
            impl_->reportFailure(
                "",
                "",
                "load_plugins_directory_scan_entry",
                "Failed reading plugin directory entry: " + entry.path().string()
            );
            return 0;
        }

        std::error_code entryEc;
        const bool isRegular = entry.is_regular_file(entryEc);
        if (entryEc) {
            impl_->reportFailure(
                "",
                "",
                "load_plugins_directory_scan_entry",
                "Failed reading plugin directory entry: " + entry.path().string()
            );
            return 0;
        }
        if (!isRegular) {
            continue;
        }
        const auto ext = entry.path().extension().string();
        if (ext != ".json" && ext != ".js") {
            continue;
        }
        candidates.push_back(entry.path());
    }

    std::sort(
        candidates.begin(),
        candidates.end(),
        [](const std::filesystem::path& lhs, const std::filesystem::path& rhs) {
            return lhs.lexically_normal().generic_string() <
                   rhs.lexically_normal().generic_string();
        }
    );

    int32_t loadedCount = 0;
    for (const auto& candidate : candidates) {
        if (loadPlugin(candidate.string())) {
            ++loadedCount;
        }
    }

    return loadedCount;
}

void PluginManager::unloadAllPlugins() {
    std::lock_guard<std::recursive_mutex> lock(impl_->stateMutex_);
    // Get list of plugins to unload (copy because we're modifying)
    std::vector<std::string> names;
    for (const auto& [name, info] : impl_->plugins_) {
        names.push_back(name);
    }

    std::sort(names.begin(), names.end());
    
    // Unload each plugin
    for (const auto& name : names) {
        unloadPlugin(name);
    }
}

// ============================================================================
// Plugin Registration
// ============================================================================

bool PluginManager::registerPlugin(const PluginInfo& info) {
    std::lock_guard<std::recursive_mutex> lock(impl_->stateMutex_);
    impl_->lastError_.clear();
    
    if (info.name.empty()) {
        impl_->lastError_ = "Plugin name cannot be empty";
        return false;
    }
    
    if (impl_->plugins_.find(info.name) != impl_->plugins_.end()) {
        impl_->lastError_ = "Plugin already registered: " + info.name;
        return false;
    }
    
    impl_->plugins_[info.name] = info;
    impl_->plugins_[info.name].loaded = true;
    
    triggerEvent(info.name, PluginEvent::ON_LOAD);
    
    return true;
}

bool PluginManager::unregisterPlugin(const std::string& name) {
    std::lock_guard<std::recursive_mutex> lock(impl_->stateMutex_);
    return unloadPlugin(name);
}

bool PluginManager::isPluginLoaded(const std::string& name) const {
    std::lock_guard<std::recursive_mutex> lock(impl_->stateMutex_);
    auto it = impl_->plugins_.find(name);
    return it != impl_->plugins_.end() && it->second.loaded;
}

const PluginInfo* PluginManager::getPluginInfo(const std::string& name) const {
    std::lock_guard<std::recursive_mutex> lock(impl_->stateMutex_);
    auto it = impl_->plugins_.find(name);
    if (it != impl_->plugins_.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<std::string> PluginManager::getLoadedPlugins() const {
    std::lock_guard<std::recursive_mutex> lock(impl_->stateMutex_);
    std::vector<std::string> result;
    for (const auto& [name, info] : impl_->plugins_) {
        if (info.loaded) {
            result.push_back(name);
        }
    }

    std::sort(result.begin(), result.end());
    return result;
}

// ============================================================================
// Command Registration
// ============================================================================

bool PluginManager::registerCommand(const std::string& pluginName,
                                   const std::string& commandName,
                                   PluginCommandHandler handler,
                                   const std::string& description) {
    std::lock_guard<std::recursive_mutex> lock(impl_->stateMutex_);
    impl_->lastError_.clear();
    
    if (!handler) {
        impl_->lastError_ = "Command handler cannot be null";
        return false;
    }
    
    // Check if plugin exists
    if (impl_->plugins_.find(pluginName) == impl_->plugins_.end()) {
        // Auto-register plugin
        PluginInfo info;
        info.name = pluginName;
        info.loaded = true;
        impl_->plugins_[pluginName] = info;
    }
    
    std::string fullKey = pluginName + "_" + commandName;
    
    // Check if command already exists
    if (impl_->commands_.find(fullKey) != impl_->commands_.end()) {
        impl_->lastError_ = "Command already registered: " + fullKey;
        return false;
    }
    
    CommandInfo cmdInfo;
    cmdInfo.pluginName = pluginName;
    cmdInfo.commandName = commandName;
    cmdInfo.description = description;
    cmdInfo.handler = std::move(handler);
    
    impl_->commands_[fullKey] = std::move(cmdInfo);
    
    triggerEvent(pluginName, PluginEvent::ON_COMMAND_REGISTERED);
    
    return true;
}

bool PluginManager::unregisterCommand(const std::string& pluginName,
                                     const std::string& commandName) {
    std::lock_guard<std::recursive_mutex> lock(impl_->stateMutex_);
    impl_->lastError_.clear();
    
    std::string fullKey = pluginName + "_" + commandName;
    
    auto it = impl_->commands_.find(fullKey);
    if (it == impl_->commands_.end()) {
        impl_->lastError_ = "Command not found: " + fullKey;
        return false;
    }
    
    impl_->commands_.erase(it);
    
    triggerEvent(pluginName, PluginEvent::ON_COMMAND_UNREGISTERED);
    
    return true;
}

int32_t PluginManager::unregisterAllCommands(const std::string& pluginName) {
    std::lock_guard<std::recursive_mutex> lock(impl_->stateMutex_);
    int32_t count = 0;
    
    // Find and remove all commands for this plugin
    auto it = impl_->commands_.begin();
    while (it != impl_->commands_.end()) {
        if (it->second.pluginName == pluginName) {
            it = impl_->commands_.erase(it);
            ++count;
        } else {
            ++it;
        }
    }
    
    if (count > 0) {
        triggerEvent(pluginName, PluginEvent::ON_COMMAND_UNREGISTERED);
    }
    
    return count;
}

bool PluginManager::hasCommand(const std::string& pluginName,
                              const std::string& commandName) const {
    std::lock_guard<std::recursive_mutex> lock(impl_->stateMutex_);
    std::string fullKey = pluginName + "_" + commandName;
    return impl_->commands_.find(fullKey) != impl_->commands_.end();
}

const CommandInfo* PluginManager::getCommandInfo(const std::string& pluginName,
                                                const std::string& commandName) const {
    std::lock_guard<std::recursive_mutex> lock(impl_->stateMutex_);
    std::string fullKey = pluginName + "_" + commandName;
    auto it = impl_->commands_.find(fullKey);
    if (it != impl_->commands_.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<std::string> PluginManager::getPluginCommands(const std::string& pluginName) const {
    std::lock_guard<std::recursive_mutex> lock(impl_->stateMutex_);
    std::vector<std::string> result;
    
    for (const auto& [key, cmd] : impl_->commands_) {
        if (cmd.pluginName == pluginName) {
            result.push_back(cmd.commandName);
        }
    }

    std::sort(result.begin(), result.end());
    return result;
}

// ============================================================================
// Command Execution
// ============================================================================

Value PluginManager::executeCommand(const std::string& pluginName,
                                   const std::string& commandName,
                                   const std::vector<Value>& args) {
    std::lock_guard<std::recursive_mutex> lock(impl_->stateMutex_);
    impl_->lastError_.clear();

    const auto reportCommandError = [&](const std::string& message,
                                        const std::string& severity = "HARD_FAIL") {
        impl_->reportFailure(pluginName, commandName, "execute_command", message, severity);
    };
    
    std::string fullKey = pluginName + "_" + commandName;
    
    auto it = impl_->commands_.find(fullKey);
    if (it == impl_->commands_.end()) {
        impl_->reportFailure(
            pluginName,
            commandName,
            "execute_command",
            "Command not found: " + fullKey,
            "WARN"
        );
        return Value();
    }

    const auto missingDependencies = getMissingDependencies(pluginName);
    if (!missingDependencies.empty()) {
        std::ostringstream message;
        message << "Missing dependencies for " << fullKey << ": ";
        for (size_t i = 0; i < missingDependencies.size(); ++i) {
            if (i > 0) {
                message << ", ";
            }
            message << missingDependencies[i];
        }

        impl_->reportFailure(
            pluginName,
            commandName,
            "execute_command_dependency_missing",
            message.str(),
            "SOFT_FAIL"
        );
        return Value();
    }
    
    // Push execution context
    PluginContext ctx;
    ctx.pluginName = pluginName;
    ctx.currentCommand = commandName;
    ctx.callDepth = static_cast<int32_t>(impl_->contextStack_.size());
    impl_->contextStack_.push_back(ctx);
    
    // Execute command
    Value result;
    try {
        result = it->second.handler(args);
    } catch (const std::exception& e) {
        reportCommandError(std::string("Command execution error: ") + e.what());
    } catch (...) {
        reportCommandError("Unknown command execution error", "CRASH_PREVENTED");
    }
    
    // Pop execution context
    impl_->contextStack_.pop_back();
    
    return result;
}

Value PluginManager::executeCommandByName(const std::string& fullName,
                                         const std::vector<Value>& args) {
    std::lock_guard<std::recursive_mutex> lock(impl_->stateMutex_);
    impl_->lastError_.clear();

    const auto reportParseFailure = [&]() {
        impl_->reportFailure(
            "",
            fullName,
            "execute_command_by_name_parse",
            "Invalid command name format: " + fullName,
            "WARN"
        );
    };

    // Fast path: exact full key match supports plugin/command names containing underscores.
    if (const auto commandIt = impl_->commands_.find(fullName);
        commandIt != impl_->commands_.end()) {
        return executeCommand(commandIt->second.pluginName, commandIt->second.commandName, args);
    }

    // Parse full name (pluginName_commandName)
    size_t underscorePos = fullName.rfind('_');
    if (underscorePos == std::string::npos || underscorePos == 0 ||
        underscorePos + 1 >= fullName.size()) {
        reportParseFailure();
        return Value();
    }
    
    std::string pluginName = fullName.substr(0, underscorePos);
    std::string commandName = fullName.substr(underscorePos + 1);
    
    return executeCommand(pluginName, commandName, args);
}

void PluginManager::executeCommandAsync(const std::string& pluginName,
                                       const std::string& commandName,
                                       const std::vector<Value>& args,
                                       std::function<void(const Value&)> callback) {
    PluginManagerImpl::AsyncCommandTask task;
    task.pluginName = pluginName;
    task.commandName = commandName;
    task.args = args;
    task.callback = std::move(callback);
    impl_->enqueueAsyncTask(std::move(task));
}

// ============================================================================
// Parameter Management
// ============================================================================

void PluginManager::setParameter(const std::string& pluginName,
                                const std::string& paramName,
                                const Value& value) {
    std::lock_guard<std::recursive_mutex> lock(impl_->stateMutex_);
    impl_->parameters_[pluginName][paramName] = value;
    
    triggerEvent(pluginName, PluginEvent::ON_PARAMETER_CHANGED);
}

Value PluginManager::getParameter(const std::string& pluginName,
                                 const std::string& paramName) const {
    std::lock_guard<std::recursive_mutex> lock(impl_->stateMutex_);
    auto pluginIt = impl_->parameters_.find(pluginName);
    if (pluginIt == impl_->parameters_.end()) {
        return Value();
    }
    
    auto paramIt = pluginIt->second.find(paramName);
    if (paramIt == pluginIt->second.end()) {
        return Value();
    }
    
    return paramIt->second;
}

Value PluginManager::getParameter(const std::string& pluginName,
                                 const std::string& paramName,
                                 const Value& defaultValue) const {
    Value result = getParameter(pluginName, paramName);
    if (result.v.index() == 0) { // null/empty
        return defaultValue;
    }
    return result;
}

std::unordered_map<std::string, Value> PluginManager::getParameters(const std::string& pluginName) const {
    std::lock_guard<std::recursive_mutex> lock(impl_->stateMutex_);
    auto it = impl_->parameters_.find(pluginName);
    if (it != impl_->parameters_.end()) {
        return it->second;
    }
    return {};
}

bool PluginManager::parseParameters(const std::string& pluginName,
                                   const std::string& json) {
    std::lock_guard<std::recursive_mutex> lock(impl_->stateMutex_);
    impl_->lastError_.clear();

    if (pluginName.empty()) {
        impl_->reportFailure("", "", "parse_parameters_name", "Plugin name cannot be empty");
        return false;
    }

    nlohmann::json parsed = nlohmann::json::parse(json, nullptr, false);
    if (parsed.is_discarded() || !parsed.is_object()) {
        impl_->reportFailure(pluginName, "", "parse_parameters_json", "Parameter JSON must be a valid object");
        return false;
    }

    if (impl_->plugins_.find(pluginName) == impl_->plugins_.end()) {
        PluginInfo info;
        info.name = pluginName;
        info.loaded = true;
        info.enabled = true;
        impl_->plugins_[pluginName] = std::move(info);
        triggerEvent(pluginName, PluginEvent::ON_LOAD);
    }

    for (const auto& [key, value] : parsed.items()) {
        setParameter(pluginName, key, jsonToValue(value));
    }

    return true;
}

// ============================================================================
// Plugin Dependencies
// ============================================================================

bool PluginManager::checkDependencies(const std::string& pluginName) const {
    auto missing = getMissingDependencies(pluginName);
    return missing.empty();
}

std::vector<std::string> PluginManager::getMissingDependencies(const std::string& pluginName) const {
    std::lock_guard<std::recursive_mutex> lock(impl_->stateMutex_);
    std::vector<std::string> missing;
    
    auto it = impl_->plugins_.find(pluginName);
    if (it == impl_->plugins_.end()) {
        return missing;
    }
    
    for (const auto& dep : it->second.dependencies) {
        if (!isPluginLoaded(dep)) {
            missing.push_back(dep);
        }
    }
    
    return missing;
}

std::vector<std::string> PluginManager::getDependents(const std::string& pluginName) const {
    std::lock_guard<std::recursive_mutex> lock(impl_->stateMutex_);
    std::vector<std::string> dependents;
    
    for (const auto& [name, info] : impl_->plugins_) {
        for (const auto& dep : info.dependencies) {
            if (dep == pluginName) {
                dependents.push_back(name);
                break;
            }
        }
    }

    std::sort(dependents.begin(), dependents.end());
    return dependents;
}

// ============================================================================
// Event Hooks
// ============================================================================

int32_t PluginManager::registerEventHandler(PluginEvent event, PluginEventHandler handler) {
    std::lock_guard<std::recursive_mutex> lock(impl_->stateMutex_);
    int32_t id = impl_->nextHandlerId_++;
    impl_->eventHandlers_[event][id] = std::move(handler);
    return id;
}

void PluginManager::unregisterEventHandler(int32_t handlerId) {
    std::lock_guard<std::recursive_mutex> lock(impl_->stateMutex_);
    for (auto& [event, handlers] : impl_->eventHandlers_) {
        handlers.erase(handlerId);
    }
}

void PluginManager::triggerEvent(const std::string& pluginName, PluginEvent event) {
    std::lock_guard<std::recursive_mutex> lock(impl_->stateMutex_);
    auto it = impl_->eventHandlers_.find(event);
    if (it != impl_->eventHandlers_.end()) {
        for (const auto& [id, handler] : it->second) {
            handler(pluginName, event);
        }
    }
}

// ============================================================================
// Execution Context
// ============================================================================

const PluginContext* PluginManager::getCurrentContext() const {
    std::lock_guard<std::recursive_mutex> lock(impl_->stateMutex_);
    if (impl_->contextStack_.empty()) {
        return nullptr;
    }
    return &impl_->contextStack_.back();
}

bool PluginManager::isExecuting() const {
    std::lock_guard<std::recursive_mutex> lock(impl_->stateMutex_);
    return !impl_->contextStack_.empty();
}

std::string PluginManager::getCurrentPlugin() const {
    std::lock_guard<std::recursive_mutex> lock(impl_->stateMutex_);
    if (impl_->contextStack_.empty()) {
        return "";
    }
    return impl_->contextStack_.back().pluginName;
}

// ============================================================================
// Error Handling
// ============================================================================

std::string PluginManager::getLastError() const {
    std::lock_guard<std::recursive_mutex> lock(impl_->stateMutex_);
    return impl_->lastError_;
}

void PluginManager::clearLastError() {
    std::lock_guard<std::recursive_mutex> lock(impl_->stateMutex_);
    impl_->lastError_.clear();
}

void PluginManager::setErrorHandler(std::function<void(const std::string& pluginName,
                                                      const std::string& commandName,
                                                      const std::string& error)> handler) {
    std::lock_guard<std::recursive_mutex> lock(impl_->stateMutex_);
    impl_->errorHandler_ = std::move(handler);
}

std::string PluginManager::exportFailureDiagnosticsJsonl() const {
    std::lock_guard<std::recursive_mutex> lock(impl_->stateMutex_);
    if (impl_->failureDiagnosticsJsonl_.empty()) {
        return "";
    }

    std::ostringstream out;
    for (size_t i = 0; i < impl_->failureDiagnosticsJsonl_.size(); ++i) {
        out << impl_->failureDiagnosticsJsonl_[i];
        if (i + 1 < impl_->failureDiagnosticsJsonl_.size()) {
            out << '\n';
        }
    }
    return out.str();
}

void PluginManager::clearFailureDiagnostics() {
    std::lock_guard<std::recursive_mutex> lock(impl_->stateMutex_);
    impl_->failureDiagnosticsJsonl_.clear();
}

// ============================================================================
// CompatStatus API Surface
// ============================================================================

CompatStatus PluginManager::getMethodStatus(const std::string& methodName) const {
    auto it = methodStatus_.find(methodName);
    if (it != methodStatus_.end()) {
        return it->second;
    }
    return CompatStatus::UNSUPPORTED;
}

std::string PluginManager::getMethodDeviation(const std::string& methodName) const {
    auto it = methodDeviations_.find(methodName);
    if (it != methodDeviations_.end()) {
        return it->second;
    }
    return "";
}

const std::unordered_map<std::string, CompatStatus>& PluginManager::getAllMethodStatuses() const {
    return methodStatus_;
}

} // namespace compat
} // namespace urpg
