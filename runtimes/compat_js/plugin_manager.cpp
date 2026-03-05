// PluginManager - MZ Plugin Command Registry + Execution - Implementation
// Phase 2 - Compat Layer

#include "plugin_manager.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace urpg {
namespace compat {

namespace {

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

Value resolveFixtureScriptValue(const nlohmann::json& node, const FixtureScriptContext& ctx);
FixtureScriptResult runFixtureScriptSteps(const nlohmann::json& script,
                                          const std::string& pluginName,
                                          const std::string& commandName,
                                          const std::vector<Value>& args,
                                          const std::unordered_map<std::string, Value>& parameters,
                                          Object& locals);

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
            Value value;
            value.v = ctx.pluginName;
            return value;
        }
        if (source == "commandName") {
            Value value;
            value.v = ctx.commandName;
            return value;
        }
        if (source == "argCount") {
            return Value::Int(static_cast<int64_t>(ctx.args.size()));
        }
        if (source == "args") {
            Array array;
            array.reserve(ctx.args.size());
            for (const auto& arg : ctx.args) {
                array.push_back(arg);
            }
            return Value::Arr(std::move(array));
        }
        if (source == "arg") {
            const int64_t index = node.value("index", static_cast<int64_t>(-1));
            if (index >= 0 && static_cast<size_t>(index) < ctx.args.size()) {
                return ctx.args[static_cast<size_t>(index)];
            }
            return resolveValueWithDefault(nlohmann::json{}, node, ctx);
        }
        if (source == "param") {
            const std::string name = node.value("name", "");
            const auto it = ctx.parameters.find(name);
            if (!name.empty() && it != ctx.parameters.end()) {
                return it->second;
            }
            return resolveValueWithDefault(nlohmann::json{}, node, ctx);
        }
        if (source == "local") {
            const std::string name = node.value("name", "");
            const auto it = ctx.locals.find(name);
            if (!name.empty() && it != ctx.locals.end()) {
                return it->second;
            }
            return resolveValueWithDefault(nlohmann::json{}, node, ctx);
        }
        if (source == "hasParam") {
            const std::string name = node.value("name", "");
            Value value;
            value.v = !name.empty() && ctx.parameters.find(name) != ctx.parameters.end();
            return value;
        }
        if (source == "paramKeys") {
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
            const auto leftIt = node.find("left");
            const auto rightIt = node.find("right");
            const Value lhs =
                leftIt != node.end() ? resolveFixtureScriptValue(*leftIt, ctx) : Value::Nil();
            const Value rhs =
                rightIt != node.end() ? resolveFixtureScriptValue(*rightIt, ctx) : Value::Nil();
            Value value;
            value.v = valuesEqual(lhs, rhs);
            return value;
        }
        if (source == "coalesce") {
            if (const auto valuesIt = node.find("values");
                valuesIt != node.end() && valuesIt->is_array()) {
                for (const auto& valueNode : *valuesIt) {
                    const Value candidate = resolveFixtureScriptValue(valueNode, ctx);
                    if (!std::holds_alternative<std::monostate>(candidate.v)) {
                        return candidate;
                    }
                }
            }
            return resolveValueWithDefault(nlohmann::json{}, node, ctx);
        }
        if (source == "concat") {
            std::string out;
            if (const auto partsIt = node.find("parts");
                partsIt != node.end() && partsIt->is_array()) {
                for (const auto& part : *partsIt) {
                    out += valueToString(resolveFixtureScriptValue(part, ctx));
                }
            }
            Value value;
            value.v = std::move(out);
            return value;
        }
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
                                          Object& locals) {
    FixtureScriptResult out;
    if (!script.is_array()) {
        return out;
    }

    for (const auto& step : script) {
        if (!step.is_object()) {
            continue;
        }
        const std::string op = step.value("op", "");
        FixtureScriptContext ctx{pluginName, commandName, args, parameters, locals};

        if (op == "set") {
            const std::string key = step.value("key", "");
            if (key.empty()) {
                continue;
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
                continue;
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
            if (branchIt != step.end() && branchIt->is_array()) {
                const auto branchResult = runFixtureScriptSteps(
                    *branchIt,
                    pluginName,
                    commandName,
                    args,
                    parameters,
                    locals
                );
                if (branchResult.returned) {
                    return branchResult;
                }
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
    }

    return out;
}

Value executeFixtureScript(const nlohmann::json& script,
                           const std::string& pluginName,
                           const std::string& commandName,
                           const std::vector<Value>& args,
                           const std::unordered_map<std::string, Value>& parameters) {
    Object locals;
    const auto result =
        runFixtureScriptSteps(script, pluginName, commandName, args, parameters, locals);
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

    // Plugin source path tracking (used by reload)
    std::unordered_map<std::string, std::string> pluginSourcePaths_;

    // QuickJS runtime bridge for fixture-backed command execution
    QuickJSRuntime runtime_;
    bool runtimeInitialized_ = false;

    PluginManagerImpl()
        : runtimeInitialized_(runtime_.initialize()) {}

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
}

PluginManager::~PluginManager() = default;

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
        methodStatus_["executeCommandAsync"] = CompatStatus::PARTIAL;
        methodDeviations_["executeCommandAsync"] = "Async execution may have different timing than MZ";
        
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
    }
}

// ============================================================================
// Plugin Lifecycle
// ============================================================================

bool PluginManager::loadPlugin(const std::string& path) {
    impl_->lastError_.clear();
    const std::filesystem::path pluginPath(path);

    // Fixture path: JSON files contain executable plugin command fixtures.
    if (pluginPath.has_extension() && pluginPath.extension() == ".json" &&
        std::filesystem::exists(pluginPath)) {
        std::ifstream input(pluginPath);
        if (!input.is_open()) {
            impl_->lastError_ = "Failed to open plugin fixture: " + path;
            return false;
        }

        nlohmann::json fixture = nlohmann::json::parse(input, nullptr, false);
        if (fixture.is_discarded() || !fixture.is_object()) {
            impl_->lastError_ = "Invalid plugin fixture JSON: " + path;
            return false;
        }

        std::string name = fixture.value("name", pluginPath.stem().string());
        if (name.empty()) {
            impl_->lastError_ = "Fixture plugin name cannot be empty: " + path;
            return false;
        }
        if (impl_->plugins_.find(name) != impl_->plugins_.end()) {
            impl_->lastError_ = "Plugin already registered: " + name;
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
            depsIt != fixture.end() && depsIt->is_array()) {
            for (const auto& dep : *depsIt) {
                if (dep.is_string()) {
                    info.dependencies.push_back(dep.get<std::string>());
                }
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
            paramsIt != fixture.end() && paramsIt->is_object()) {
            for (const auto& [key, paramValue] : paramsIt->items()) {
                setParameter(name, key, jsonToValue(paramValue));
            }
        }

        if (const auto cmdsIt = fixture.find("commands");
            cmdsIt != fixture.end() && cmdsIt->is_array()) {
            for (const auto& cmd : *cmdsIt) {
                if (!cmd.is_object()) {
                    continue;
                }
                const std::string commandName = cmd.value("name", "");
                if (commandName.empty()) {
                    continue;
                }

                const std::string description = cmd.value("description", "");
                if (const auto jsIt = cmd.find("js");
                    jsIt != cmd.end() && jsIt->is_string()) {
                    QuickJSContext* pluginContext = impl_->ensurePluginContext(name);
                    if (!pluginContext) {
                        impl_->lastError_ = "Failed to initialize QuickJS context for plugin: " + name;
                        rollbackFixturePlugin();
                        return false;
                    }

                    const std::string entryPoint = cmd.value("entry", commandName);
                    if (entryPoint.empty()) {
                        impl_->lastError_ = "Fixture JS command entry cannot be empty: " + commandName;
                        rollbackFixturePlugin();
                        return false;
                    }

                    const std::string jsSource = jsIt->get<std::string>();
                    const ScriptResult evalResult =
                        pluginContext->eval(jsSource, path + "#" + name + "." + commandName);
                    if (!evalResult.success) {
                        impl_->lastError_ =
                            !evalResult.error.empty()
                                ? evalResult.error
                                : ("Failed to evaluate fixture JS for command: " + commandName);
                        rollbackFixturePlugin();
                        return false;
                    }

                    if (!registerCommand(
                        name,
                        commandName,
                        [this, name, commandName, entryPoint](const std::vector<Value>& args) -> Value {
                            QuickJSContext* context = impl_->getPluginContext(name);
                            if (!context) {
                                impl_->lastError_ = "QuickJS context missing for plugin: " + name;
                                if (impl_->errorHandler_) {
                                    impl_->errorHandler_(name, commandName, impl_->lastError_);
                                }
                                return Value::Nil();
                            }

                            const ScriptResult result = context->call(entryPoint, args);
                            if (!result.success) {
                                impl_->lastError_ =
                                    !result.error.empty()
                                        ? result.error
                                        : ("QuickJS command failed: " + name + "_" + commandName);
                                if (impl_->errorHandler_) {
                                    impl_->errorHandler_(name, commandName, impl_->lastError_);
                                }
                                return Value::Nil();
                            }
                            return result.value;
                        },
                        description)) {
                        rollbackFixturePlugin();
                        return false;
                    }
                    continue;
                }

                if (const auto scriptIt = cmd.find("script");
                    scriptIt != cmd.end() && scriptIt->is_array()) {
                    QuickJSContext* pluginContext = impl_->ensurePluginContext(name);
                    if (!pluginContext) {
                        impl_->lastError_ = "Failed to initialize QuickJS context for plugin: " + name;
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
                                    params
                                );
                            })) {
                        impl_->lastError_ = "Failed to register QuickJS fixture function for command: " + commandName;
                        rollbackFixturePlugin();
                        return false;
                    }

                    if (!registerCommand(
                        pluginForCommand,
                        commandForCommand,
                        [this, pluginForCommand, commandForCommand](const std::vector<Value>& args) -> Value {
                            QuickJSContext* context = impl_->getPluginContext(pluginForCommand);
                            if (!context) {
                                impl_->lastError_ = "QuickJS context missing for plugin: " + pluginForCommand;
                                if (impl_->errorHandler_) {
                                    impl_->errorHandler_(
                                        pluginForCommand,
                                        commandForCommand,
                                        impl_->lastError_
                                    );
                                }
                                return Value::Nil();
                            }

                            const ScriptResult result = context->call(commandForCommand, args);
                            if (!result.success) {
                                impl_->lastError_ =
                                    !result.error.empty()
                                        ? result.error
                                        : ("QuickJS command failed: " + pluginForCommand + "_" + commandForCommand);
                                if (impl_->errorHandler_) {
                                    impl_->errorHandler_(
                                        pluginForCommand,
                                        commandForCommand,
                                        impl_->lastError_
                                    );
                                }
                                return Value::Nil();
                            }
                            return result.value;
                        },
                        description)) {
                        rollbackFixturePlugin();
                        return false;
                    }
                    continue;
                }

                const std::string mode = cmd.value("mode", "const");
                if (mode == "arg_count") {
                    if (!registerCommand(
                        name,
                        commandName,
                        [](const std::vector<Value>& args) -> Value {
                            return Value::Int(static_cast<int64_t>(args.size()));
                        },
                        description)) {
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
        impl_->lastError_ = "Plugin name cannot be empty";
        return false;
    }
    if (impl_->plugins_.find(name) != impl_->plugins_.end()) {
        impl_->lastError_ = "Plugin already registered: " + name;
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
            impl_->lastError_ = "Failed to initialize QuickJS context for plugin: " + name;
            return false;
        }
    }
    triggerEvent(name, PluginEvent::ON_LOAD);

    return true;
}

bool PluginManager::unloadPlugin(const std::string& name) {
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
    impl_->lastError_.clear();

    std::error_code ec;
    const std::filesystem::path dirPath(directory);
    if (!std::filesystem::exists(dirPath, ec) || !std::filesystem::is_directory(dirPath, ec)) {
        impl_->lastError_ = "Plugin directory not found: " + directory;
        return 0;
    }

    int32_t loadedCount = 0;
    for (const auto& entry : std::filesystem::directory_iterator(dirPath, ec)) {
        if (ec) {
            impl_->lastError_ = "Failed scanning plugin directory: " + directory;
            return loadedCount;
        }
        if (!entry.is_regular_file(ec)) {
            continue;
        }
        if (ec) {
            continue;
        }
        const auto ext = entry.path().extension().string();
        if (ext != ".json" && ext != ".js") {
            continue;
        }
        if (loadPlugin(entry.path().string())) {
            ++loadedCount;
        }
    }

    return loadedCount;
}

void PluginManager::unloadAllPlugins() {
    // Get list of plugins to unload (copy because we're modifying)
    std::vector<std::string> names;
    for (const auto& [name, info] : impl_->plugins_) {
        names.push_back(name);
    }
    
    // Unload each plugin
    for (const auto& name : names) {
        unloadPlugin(name);
    }
}

// ============================================================================
// Plugin Registration
// ============================================================================

bool PluginManager::registerPlugin(const PluginInfo& info) {
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
    return unloadPlugin(name);
}

bool PluginManager::isPluginLoaded(const std::string& name) const {
    auto it = impl_->plugins_.find(name);
    return it != impl_->plugins_.end() && it->second.loaded;
}

const PluginInfo* PluginManager::getPluginInfo(const std::string& name) const {
    auto it = impl_->plugins_.find(name);
    if (it != impl_->plugins_.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<std::string> PluginManager::getLoadedPlugins() const {
    std::vector<std::string> result;
    for (const auto& [name, info] : impl_->plugins_) {
        if (info.loaded) {
            result.push_back(name);
        }
    }
    return result;
}

// ============================================================================
// Command Registration
// ============================================================================

bool PluginManager::registerCommand(const std::string& pluginName,
                                   const std::string& commandName,
                                   PluginCommandHandler handler,
                                   const std::string& description) {
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
    std::string fullKey = pluginName + "_" + commandName;
    return impl_->commands_.find(fullKey) != impl_->commands_.end();
}

const CommandInfo* PluginManager::getCommandInfo(const std::string& pluginName,
                                                const std::string& commandName) const {
    std::string fullKey = pluginName + "_" + commandName;
    auto it = impl_->commands_.find(fullKey);
    if (it != impl_->commands_.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<std::string> PluginManager::getPluginCommands(const std::string& pluginName) const {
    std::vector<std::string> result;
    
    for (const auto& [key, cmd] : impl_->commands_) {
        if (cmd.pluginName == pluginName) {
            result.push_back(cmd.commandName);
        }
    }
    
    return result;
}

// ============================================================================
// Command Execution
// ============================================================================

Value PluginManager::executeCommand(const std::string& pluginName,
                                   const std::string& commandName,
                                   const std::vector<Value>& args) {
    impl_->lastError_.clear();
    
    std::string fullKey = pluginName + "_" + commandName;
    
    auto it = impl_->commands_.find(fullKey);
    if (it == impl_->commands_.end()) {
        impl_->lastError_ = "Command not found: " + fullKey;
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
        impl_->lastError_ = std::string("Command execution error: ") + e.what();
        
        if (impl_->errorHandler_) {
            impl_->errorHandler_(pluginName, commandName, impl_->lastError_);
        }
    } catch (...) {
        impl_->lastError_ = "Unknown command execution error";
        
        if (impl_->errorHandler_) {
            impl_->errorHandler_(pluginName, commandName, impl_->lastError_);
        }
    }
    
    // Pop execution context
    impl_->contextStack_.pop_back();
    
    return result;
}

Value PluginManager::executeCommandByName(const std::string& fullName,
                                         const std::vector<Value>& args) {
    // Parse full name (pluginName_commandName)
    size_t underscorePos = fullName.rfind('_');
    if (underscorePos == std::string::npos) {
        impl_->lastError_ = "Invalid command name format: " + fullName;
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
    // TODO: Implement proper async execution with task queue
    // For now, execute synchronously and call callback
    Value result = executeCommand(pluginName, commandName, args);
    
    if (callback) {
        callback(result);
    }
}

// ============================================================================
// Parameter Management
// ============================================================================

void PluginManager::setParameter(const std::string& pluginName,
                                const std::string& paramName,
                                const Value& value) {
    impl_->parameters_[pluginName][paramName] = value;
    
    triggerEvent(pluginName, PluginEvent::ON_PARAMETER_CHANGED);
}

Value PluginManager::getParameter(const std::string& pluginName,
                                 const std::string& paramName) const {
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
    auto it = impl_->parameters_.find(pluginName);
    if (it != impl_->parameters_.end()) {
        return it->second;
    }
    return {};
}

bool PluginManager::parseParameters(const std::string& pluginName,
                                   const std::string& json) {
    impl_->lastError_.clear();

    if (pluginName.empty()) {
        impl_->lastError_ = "Plugin name cannot be empty";
        return false;
    }

    nlohmann::json parsed = nlohmann::json::parse(json, nullptr, false);
    if (parsed.is_discarded() || !parsed.is_object()) {
        impl_->lastError_ = "Parameter JSON must be a valid object";
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
    std::vector<std::string> dependents;
    
    for (const auto& [name, info] : impl_->plugins_) {
        for (const auto& dep : info.dependencies) {
            if (dep == pluginName) {
                dependents.push_back(name);
                break;
            }
        }
    }
    
    return dependents;
}

// ============================================================================
// Event Hooks
// ============================================================================

int32_t PluginManager::registerEventHandler(PluginEvent event, PluginEventHandler handler) {
    int32_t id = impl_->nextHandlerId_++;
    impl_->eventHandlers_[event][id] = std::move(handler);
    return id;
}

void PluginManager::unregisterEventHandler(int32_t handlerId) {
    for (auto& [event, handlers] : impl_->eventHandlers_) {
        handlers.erase(handlerId);
    }
}

void PluginManager::triggerEvent(const std::string& pluginName, PluginEvent event) {
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
    if (impl_->contextStack_.empty()) {
        return nullptr;
    }
    return &impl_->contextStack_.back();
}

bool PluginManager::isExecuting() const {
    return !impl_->contextStack_.empty();
}

std::string PluginManager::getCurrentPlugin() const {
    if (impl_->contextStack_.empty()) {
        return "";
    }
    return impl_->contextStack_.back().pluginName;
}

// ============================================================================
// Error Handling
// ============================================================================

std::string PluginManager::getLastError() const {
    return impl_->lastError_;
}

void PluginManager::clearLastError() {
    impl_->lastError_.clear();
}

void PluginManager::setErrorHandler(std::function<void(const std::string& pluginName,
                                                      const std::string& commandName,
                                                      const std::string& error)> handler) {
    impl_->errorHandler_ = std::move(handler);
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
