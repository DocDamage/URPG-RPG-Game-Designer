#include "plugin_manager_fixture_script.h"
#include "plugin_manager_support.h"

#include <algorithm>
#include <initializer_list>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string_view>

namespace urpg::compat::plugin_manager_detail {

namespace {

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
            value.v = lhsNumeric && rhsNumeric && (valueAsDouble(lhs) > valueAsDouble(rhs));
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
            value.v = lhsNumeric && rhsNumeric && (valueAsDouble(lhs) < valueAsDouble(rhs));
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

} // namespace

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

} // namespace urpg::compat::plugin_manager_detail
