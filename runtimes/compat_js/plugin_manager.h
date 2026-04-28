#pragma once

// PluginManager - MZ Plugin Command Registry + Execution
// Phase 2 - Compat Layer
//
// This defines the PluginManager compatibility surface for MZ plugins.
// Per Section 4 - WindowCompat Explicit Surface:
// Plugin command registration and execution is required for plugin compatibility.
// This is the active and remaining in-tree scripting/plugin execution path.
// It supports fixture-backed import/validation descriptors and live JavaScript
// plugin files through the QuickJS compat runtime.
//
// The PluginManager provides:
// - Plugin command registration (registerCommand)
// - Command execution (executeCommand)
// - Plugin metadata management
// - Plugin lifecycle management (load, unload, reload)

#include "engine/runtimes/bridge/value.h"
#include "quickjs_runtime.h"
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace urpg {
namespace compat {

// Forward declarations
class PluginManagerImpl;

// Plugin command handler function type
using PluginCommandHandler = std::function<Value(const std::vector<Value>& args)>;

// Plugin metadata
struct PluginInfo {
    std::string name{};
    std::string version{};
    std::string author{};
    std::string description{};
    std::vector<std::string> dependencies{};
    std::vector<std::string> parameters{};
    bool enabled = true;
    bool loaded = false;
};

// Plugin parameter definition
struct PluginParameter {
    std::string name;
    std::string type; // "string", "number", "boolean", "select", "file", etc.
    std::string defaultValue;
    std::string description;
    std::vector<std::string> options; // For "select" type
    bool required = false;
};

// Registered command info
struct CommandInfo {
    std::string pluginName;
    std::string commandName;
    std::string description;
    PluginCommandHandler handler;
    std::vector<std::string> argNames;
};

// Plugin execution context
struct PluginContext {
    std::string pluginName;
    std::string currentCommand;
    int32_t callDepth = 0;
    bool isYielding = false;
};

// PluginManager - MZ compatibility layer for plugin management
class PluginManager {
  public:
    PluginManager();
    ~PluginManager();

    // Non-copyable
    PluginManager(const PluginManager&) = delete;
    PluginManager& operator=(const PluginManager&) = delete;

    // Singleton access for compatibility
    static PluginManager& instance();

    // ========================================================================
    // Plugin Lifecycle
    // ========================================================================

    // Status: FULL - Loads fixture descriptors and live JavaScript plugins through QuickJS
    bool loadPlugin(const std::string& path);

    // Status: FULL - Unloads tracked fixture/live plugin descriptors and runtime contexts
    bool unloadPlugin(const std::string& name);

    // Status: FULL - Reload works for tracked fixture/live plugin source paths
    bool reloadPlugin(const std::string& name);

    // Status: FULL - Directory scan loads fixture descriptors and live JavaScript plugin files
    int32_t loadPluginsFromDirectory(const std::string& directory);

    // Status: FULL - Unloads tracked fixture/live plugin descriptors and runtime contexts
    void unloadAllPlugins();

    // ========================================================================
    // Plugin Registration
    // ========================================================================

    // Status: FULL - Registers compat metadata
    bool registerPlugin(const PluginInfo& info);

    // Status: FULL - Unregisters compat metadata and command/runtime state
    bool unregisterPlugin(const std::string& name);

    // Status: FULL - Reports compat metadata state
    bool isPluginLoaded(const std::string& name) const;

    // Status: FULL - Returns compat metadata
    const PluginInfo* getPluginInfo(const std::string& name) const;

    // Status: FULL - Returns compat metadata
    std::vector<std::string> getLoadedPlugins() const;

    // ========================================================================
    // Command Registration
    // ========================================================================

    // Status: FULL - Registers compat command handlers, including live JS command bridges
    // The command will be callable as: pluginName_commandName
    bool registerCommand(const std::string& pluginName, const std::string& commandName, PluginCommandHandler handler,
                         const std::string& description = "");

    // Status: FULL - Unregisters compat command handlers, not live engine plugin commands
    bool unregisterCommand(const std::string& pluginName, const std::string& commandName);

    // Status: FULL - Unregisters compat command handlers, not live engine plugin commands
    int32_t unregisterAllCommands(const std::string& pluginName);

    // Status: FULL - Checks compat command registration
    bool hasCommand(const std::string& pluginName, const std::string& commandName) const;

    // Status: FULL - Returns compat command registration
    const CommandInfo* getCommandInfo(const std::string& pluginName, const std::string& commandName) const;

    // Status: FULL - Returns compat command registration
    std::vector<std::string> getPluginCommands(const std::string& pluginName) const;

    // ========================================================================
    // Command Execution
    // ========================================================================

    // Status: FULL - Reliable for registered handlers, fixtures, and live JS commands
    Value executeCommand(const std::string& pluginName, const std::string& commandName, const std::vector<Value>& args);

    // Status: FULL - Reliable for registered handlers, fixtures, and live JS commands
    Value executeCommandByName(const std::string& fullName, const std::vector<Value>& args);

    // Status: FULL - FIFO worker queue is deterministic and callback delivery is
    // deferred to the owning thread.
    // Threading contract: command execution happens on the worker thread, but callbacks
    // are deferred until dispatchPendingAsyncCallbacks() is called on the owning thread.
    void executeCommandAsync(const std::string& pluginName, const std::string& commandName,
                             const std::vector<Value>& args, std::function<void(const Value&)> callback);

    // Status: FULL - Drains deferred async callbacks in FIFO order on the owning thread
    int32_t dispatchPendingAsyncCallbacks();

    // ========================================================================
    // Parameter Management
    // ========================================================================

    // Status: FULL - Stores compat parameter state
    void setParameter(const std::string& pluginName, const std::string& paramName, const Value& value);

    // Status: FULL - Returns compat parameter state
    Value getParameter(const std::string& pluginName, const std::string& paramName) const;

    // Status: FULL - Returns compat parameter state
    Value getParameter(const std::string& pluginName, const std::string& paramName, const Value& defaultValue) const;

    // Status: FULL - Returns compat parameter state
    std::unordered_map<std::string, Value> getParameters(const std::string& pluginName) const;

    // Status: FULL - Parses compat parameter JSON into parameter state
    bool parseParameters(const std::string& pluginName, const std::string& json);

    // ========================================================================
    // Plugin Dependencies
    // ========================================================================

    // Status: FULL - Checks compat metadata dependencies
    bool checkDependencies(const std::string& pluginName) const;

    // Status: FULL - Reports compat metadata dependencies
    std::vector<std::string> getMissingDependencies(const std::string& pluginName) const;

    // Status: FULL - Reports compat metadata dependencies
    std::vector<std::string> getDependents(const std::string& pluginName) const;

    // ========================================================================
    // Event Hooks
    // ========================================================================

    // Status: FULL - Event hooks apply to compat metadata/command registration
    enum class PluginEvent : uint8_t {
        ON_LOAD = 0,
        ON_UNLOAD = 1,
        ON_ENABLE = 2,
        ON_DISABLE = 3,
        ON_COMMAND_REGISTERED = 4,
        ON_COMMAND_UNREGISTERED = 5,
        ON_PARAMETER_CHANGED = 6
    };

    // Event handler type
    using PluginEventHandler = std::function<void(const std::string& pluginName, PluginEvent event)>;

    // Status: FULL - Registers compat event handlers
    int32_t registerEventHandler(PluginEvent event, PluginEventHandler handler);

    // Status: FULL - Unregisters compat event handlers
    void unregisterEventHandler(int32_t handlerId);

    // ========================================================================
    // Execution Context
    // ========================================================================

    // Status: FULL - Returns compat command execution context
    const PluginContext* getCurrentContext() const;

    // Status: FULL - Reports compat command execution state
    bool isExecuting() const;

    // Status: FULL - Reports compat command execution state
    std::string getCurrentPlugin() const;

    // ========================================================================
    // Error Handling
    // ========================================================================

    // Status: FULL - Reports compat bridge error state
    std::string getLastError() const;

    // Status: FULL - Clears compat bridge error state
    void clearLastError();

    // Status: FULL - Registers compat bridge error handlers
    void setErrorHandler(
        std::function<void(const std::string& pluginName, const std::string& commandName, const std::string& error)>
            handler);

    // Status: FULL - Exports compat bridge failure diagnostics
    std::string exportFailureDiagnosticsJsonl() const;

    // Status: FULL - Clears compat bridge failure diagnostics
    void clearFailureDiagnostics();

    // Status: FULL - Exports successful compat command execution diagnostics for editor/report parity
    std::string exportExecutionDiagnosticsJsonl() const;

    // Status: FULL - Clears successful compat command execution diagnostics
    void clearExecutionDiagnostics();

    // ========================================================================
    // CompatStatus API Surface
    // ========================================================================

    // Status: FULL - Returns compat status registry entries
    CompatStatus getMethodStatus(const std::string& methodName) const;

    // Status: FULL - Returns compat deviation descriptions
    std::string getMethodDeviation(const std::string& methodName) const;

    // Status: FULL - Returns compat status registry entries
    const std::unordered_map<std::string, CompatStatus>& getAllMethodStatuses() const;

  private:
    std::unique_ptr<PluginManagerImpl> impl_;

    // Method status registry
    static std::unordered_map<std::string, CompatStatus> methodStatus_;
    static std::unordered_map<std::string, std::string> methodDeviations_;

    // Initialize method status registry
    void initializeMethodStatus();

    // Trigger event
    void triggerEvent(const std::string& pluginName, PluginEvent event);
};

} // namespace compat
} // namespace urpg
