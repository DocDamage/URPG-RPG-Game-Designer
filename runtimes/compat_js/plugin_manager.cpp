// PluginManager - MZ Plugin Command Registry + Execution - Implementation
// Phase 2 - Compat Layer

#include "plugin_manager.h"
#include "plugin_manager_directory_scan.h"
#include "plugin_manager_fixture_script.h"
#include "plugin_manager_status.h"
#include "plugin_manager_support.h"
#include <algorithm>
#include <cassert>
#include <condition_variable>
#include <deque>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <nlohmann/json.hpp>
#include <sstream>
#include <thread>

namespace urpg {
namespace compat {

namespace {

using plugin_manager_detail::compatSeverityToString;
using plugin_manager_detail::currentUtcTimestampIso8601;
using plugin_manager_detail::executeFixtureScript;
using plugin_manager_detail::initializePluginManagerMethodStatus;
using plugin_manager_detail::jsonToValue;
using plugin_manager_detail::pluginNameFromPath;
using plugin_manager_detail::scanPluginDirectory;

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

    struct CompletedAsyncCallbackTask {
        std::function<void(const Value&)> callback;
        Value result = Value::Nil();
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
    std::mutex completedAsyncMutex_;
    std::deque<CompletedAsyncCallbackTask> completedAsyncCallbacks_;
    std::thread asyncWorker_;
    bool stopAsyncWorker_ = false;
    uint64_t nextAsyncSequence_ = 1;
    std::thread::id owningThreadId_ = std::this_thread::get_id();

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
                // Async command execution happens on the worker thread, but callback delivery
                // is deferred until the owning thread explicitly drains the completed queue.
                std::lock_guard<std::mutex> lock(impl_->completedAsyncMutex_);
                impl_->completedAsyncCallbacks_.push_back(
                    PluginManagerImpl::CompletedAsyncCallbackTask{
                        .callback = std::move(task.callback),
                        .result = result,
                        .sequence = task.sequence,
                    }
                );
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
    initializePluginManagerMethodStatus(methodStatus_, methodDeviations_);
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

    const auto scanResult = scanPluginDirectory(directory);
    if (!scanResult.ok()) {
        impl_->reportFailure("", "", scanResult.errorOperation, scanResult.errorMessage);
        return 0;
    }

    int32_t loadedCount = 0;
    for (const auto& candidate : scanResult.candidates) {
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

int32_t PluginManager::dispatchPendingAsyncCallbacks() {
    if (std::this_thread::get_id() != impl_->owningThreadId_) {
        impl_->lastError_ = "dispatchPendingAsyncCallbacks must be called on the owning thread";
        return 0;
    }

    impl_->lastError_.clear();

    std::deque<PluginManagerImpl::CompletedAsyncCallbackTask> pending;
    {
        std::lock_guard<std::mutex> lock(impl_->completedAsyncMutex_);
        pending.swap(impl_->completedAsyncCallbacks_);
    }

    int32_t dispatched = 0;
    for (auto& task : pending) {
        if (!task.callback) {
            continue;
        }
        task.callback(task.result);
        ++dispatched;
    }
    return dispatched;
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
