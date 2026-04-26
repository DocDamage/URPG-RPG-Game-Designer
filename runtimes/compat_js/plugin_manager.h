#pragma once

// PluginManager - MZ Plugin Command Registry + Execution
// Phase 2 - Compat Layer
//
// This defines the PluginManager compatibility surface for MZ plugins.
// Per Section 4 - WindowCompat Explicit Surface:
// Plugin command registration and execution is required for plugin compatibility.
// This is the active and only remaining in-tree scripting/plugin execution path.
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

    // Status: PARTIAL - Loads fixture descriptors and live JavaScript plugins through QuickJS
    bool loadPlugin(const std::string& path);

    // Status: PARTIAL - Unloads tracked fixture/live plugin descriptors and runtime contexts
    bool unloadPlugin(const std::string& name);

    // Status: PARTIAL - Reload works for tracked fixture/live plugin source paths
    bool reloadPlugin(const std::string& name);

    // Status: PARTIAL - Directory scan loads fixture descriptors and live JavaScript plugin files
    int32_t loadPluginsFromDirectory(const std::string& directory);

    // Status: PARTIAL - Unloads tracked fixture/live plugin descriptors and runtime contexts
    void unloadAllPlugins();

    // ========================================================================
    // Plugin Registration
    // ========================================================================

    // Status: PARTIAL - Registers compat metadata
    bool registerPlugin(const PluginInfo& info);

    // Status: PARTIAL - Unregisters compat metadata and command/runtime state
    bool unregisterPlugin(const std::string& name);

    // Status: PARTIAL - Reports compat metadata state
    bool isPluginLoaded(const std::string& name) const;

    // Status: PARTIAL - Returns compat metadata
    const PluginInfo* getPluginInfo(const std::string& name) const;

    // Status: PARTIAL - Returns compat metadata
    std::vector<std::string> getLoadedPlugins() const;

    // ========================================================================
    // Command Registration
    // ========================================================================

    // Status: PARTIAL - Registers compat command handlers, including live JS command bridges
    // The command will be callable as: pluginName_commandName
    bool registerCommand(const std::string& pluginName, const std::string& commandName, PluginCommandHandler handler,
                         const std::string& description = "");

    // Status: PARTIAL - Unregisters compat command handlers, not live engine plugin commands
    bool unregisterCommand(const std::string& pluginName, const std::string& commandName);

    // Status: PARTIAL - Unregisters compat command handlers, not live engine plugin commands
    int32_t unregisterAllCommands(const std::string& pluginName);

    // Status: PARTIAL - Checks compat command registration
    bool hasCommand(const std::string& pluginName, const std::string& commandName) const;

    // Status: PARTIAL - Returns compat command registration
    const CommandInfo* getCommandInfo(const std::string& pluginName, const std::string& commandName) const;

    // Status: PARTIAL - Returns compat command registration
    std::vector<std::string> getPluginCommands(const std::string& pluginName) const;

    // ========================================================================
    // Command Execution
    // ========================================================================

    // Status: PARTIAL - Reliable for registered handlers, fixtures, and live JS commands
    Value executeCommand(const std::string& pluginName, const std::string& commandName, const std::vector<Value>& args);

    // Status: PARTIAL - Reliable for registered handlers, fixtures, and live JS commands
    Value executeCommandByName(const std::string& fullName, const std::vector<Value>& args);

    // Status: PARTIAL - FIFO worker queue is deterministic and callback delivery is
    // deferred to the owning thread.
    // Threading contract: command execution happens on the worker thread, but callbacks
    // are deferred until dispatchPendingAsyncCallbacks() is called on the owning thread.
    void executeCommandAsync(const std::string& pluginName, const std::string& commandName,
                             const std::vector<Value>& args, std::function<void(const Value&)> callback);

    // Status: PARTIAL - Drains deferred async callbacks in FIFO order on the owning thread only
    int32_t dispatchPendingAsyncCallbacks();

    // ========================================================================
    // Parameter Management
    // ========================================================================

    // Status: PARTIAL - Stores compat parameter state only
    void setParameter(const std::string& pluginName, const std::string& paramName, const Value& value);

    // Status: PARTIAL - Returns compat parameter state only
    Value getParameter(const std::string& pluginName, const std::string& paramName) const;

    // Status: PARTIAL - Returns compat parameter state only
    Value getParameter(const std::string& pluginName, const std::string& paramName, const Value& defaultValue) const;

    // Status: PARTIAL - Returns compat parameter state only
    std::unordered_map<std::string, Value> getParameters(const std::string& pluginName) const;

    // Status: PARTIAL - Parses compat parameter JSON into parameter state
    bool parseParameters(const std::string& pluginName, const std::string& json);

    // ========================================================================
    // Plugin Dependencies
    // ========================================================================

    // Status: PARTIAL - Checks compat metadata dependencies only
    bool checkDependencies(const std::string& pluginName) const;

    // Status: PARTIAL - Reports compat metadata dependencies only
    std::vector<std::string> getMissingDependencies(const std::string& pluginName) const;

    // Status: PARTIAL - Reports compat metadata dependencies only
    std::vector<std::string> getDependents(const std::string& pluginName) const;

    // ========================================================================
    // Event Hooks
    // ========================================================================

    // Status: PARTIAL - Event hooks apply to compat metadata/command registration only
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

    // Status: PARTIAL - Registers compat event handlers only
    int32_t registerEventHandler(PluginEvent event, PluginEventHandler handler);

    // Status: PARTIAL - Unregisters compat event handlers only
    void unregisterEventHandler(int32_t handlerId);

    // ========================================================================
    // Execution Context
    // ========================================================================

    // Status: PARTIAL - Returns compat command execution context only
    const PluginContext* getCurrentContext() const;

    // Status: PARTIAL - Reports compat command execution state only
    bool isExecuting() const;

    // Status: PARTIAL - Reports compat command execution state only
    std::string getCurrentPlugin() const;

    // ========================================================================
    // Error Handling
    // ========================================================================

    // Status: PARTIAL - Reports compat bridge error state only
    std::string getLastError() const;

    // Status: PARTIAL - Clears compat bridge error state only
    void clearLastError();

    // Status: PARTIAL - Registers compat bridge error handlers only
    void setErrorHandler(
        std::function<void(const std::string& pluginName, const std::string& commandName, const std::string& error)>
            handler);

    // Status: PARTIAL - Exports compat bridge failure diagnostics only
    std::string exportFailureDiagnosticsJsonl() const;

    // Status: PARTIAL - Clears compat bridge failure diagnostics only
    void clearFailureDiagnostics();

    // ========================================================================
    // CompatStatus API Surface
    // ========================================================================

    // Status: PARTIAL - Returns compat status registry entries only
    CompatStatus getMethodStatus(const std::string& methodName) const;

    // Status: PARTIAL - Returns compat deviation descriptions only
    std::string getMethodDeviation(const std::string& methodName) const;

    // Status: PARTIAL - Returns compat status registry entries only
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
