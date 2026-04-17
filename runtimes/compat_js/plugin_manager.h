#pragma once

// PluginManager - MZ Plugin Command Registry + Execution
// Phase 2 - Compat Layer
//
// This defines the PluginManager compatibility surface for MZ plugins.
// Per Section 4 - WindowCompat Explicit Surface:
// Plugin command registration and execution is required for plugin compatibility.
//
// The PluginManager provides:
// - Plugin command registration (registerCommand)
// - Command execution (executeCommand)
// - Plugin metadata management
// - Plugin lifecycle management (load, unload, reload)

#include "quickjs_runtime.h"
#include "engine/runtimes/bridge/value.h"
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
    std::string name;
    std::string version;
    std::string author;
    std::string description;
    std::vector<std::string> dependencies;
    std::vector<std::string> parameters;
    bool enabled = true;
    bool loaded = false;
};

// Plugin parameter definition
struct PluginParameter {
    std::string name;
    std::string type;       // "string", "number", "boolean", "select", "file", etc.
    std::string defaultValue;
    std::string description;
    std::vector<std::string> options;  // For "select" type
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
    
    // Status: PARTIAL - Loads fixture-backed plugin descriptors, not a live MZ runtime plugin
    bool loadPlugin(const std::string& path);
    
    // Status: FULL - Unload plugin by name
    bool unloadPlugin(const std::string& name);
    
    // Status: PARTIAL - Reload works for tracked fixture-backed plugins
    bool reloadPlugin(const std::string& name);
    
    // Status: PARTIAL - Directory scan loads fixture-backed descriptors, not a live runtime plugin lane
    int32_t loadPluginsFromDirectory(const std::string& directory);
    
    // Status: FULL - Unload all plugins
    void unloadAllPlugins();
    
    // ========================================================================
    // Plugin Registration
    // ========================================================================
    
    // Status: FULL - Register plugin metadata
    bool registerPlugin(const PluginInfo& info);
    
    // Status: FULL - Unregister plugin
    bool unregisterPlugin(const std::string& name);
    
    // Status: FULL - Check if plugin is loaded
    bool isPluginLoaded(const std::string& name) const;
    
    // Status: FULL - Get plugin info
    const PluginInfo* getPluginInfo(const std::string& name) const;
    
    // Status: FULL - Get all loaded plugins
    std::vector<std::string> getLoadedPlugins() const;
    
    // ========================================================================
    // Command Registration
    // ========================================================================
    
    // Status: FULL - Register a plugin command
    // The command will be callable as: pluginName_commandName
    bool registerCommand(const std::string& pluginName,
                        const std::string& commandName,
                        PluginCommandHandler handler,
                        const std::string& description = "");
    
    // Status: FULL - Unregister a plugin command
    bool unregisterCommand(const std::string& pluginName,
                          const std::string& commandName);
    
    // Status: FULL - Unregister all commands for a plugin
    int32_t unregisterAllCommands(const std::string& pluginName);
    
    // Status: FULL - Check if command exists
    bool hasCommand(const std::string& pluginName,
                   const std::string& commandName) const;
    
    // Status: FULL - Get command info
    const CommandInfo* getCommandInfo(const std::string& pluginName,
                                      const std::string& commandName) const;
    
    // Status: FULL - Get all commands for a plugin
    std::vector<std::string> getPluginCommands(const std::string& pluginName) const;
    
    // ========================================================================
    // Command Execution
    // ========================================================================
    
    // Status: PARTIAL - Reliable for registered handlers/fixtures, but still fixture-bridge backed
    Value executeCommand(const std::string& pluginName,
                        const std::string& commandName,
                        const std::vector<Value>& args);
    
    // Status: PARTIAL - Reliable for registered handlers/fixtures, but still fixture-bridge backed
    Value executeCommandByName(const std::string& fullName,
                              const std::vector<Value>& args);
    
    // Status: PARTIAL - FIFO worker queue is deterministic, but callbacks run on the worker thread
    void executeCommandAsync(const std::string& pluginName,
                            const std::string& commandName,
                            const std::vector<Value>& args,
                            std::function<void(const Value&)> callback);
    
    // ========================================================================
    // Parameter Management
    // ========================================================================
    
    // Status: FULL - Set plugin parameter
    void setParameter(const std::string& pluginName,
                     const std::string& paramName,
                     const Value& value);
    
    // Status: FULL - Get plugin parameter
    Value getParameter(const std::string& pluginName,
                      const std::string& paramName) const;
    
    // Status: FULL - Get parameter with default value
    Value getParameter(const std::string& pluginName,
                      const std::string& paramName,
                      const Value& defaultValue) const;
    
    // Status: FULL - Get all parameters for a plugin
    std::unordered_map<std::string, Value> getParameters(const std::string& pluginName) const;
    
    // Status: FULL - Parse plugin parameters from JSON
    bool parseParameters(const std::string& pluginName,
                        const std::string& json);
    
    // ========================================================================
    // Plugin Dependencies
    // ========================================================================
    
    // Status: FULL - Check if dependencies are satisfied
    bool checkDependencies(const std::string& pluginName) const;
    
    // Status: FULL - Get missing dependencies
    std::vector<std::string> getMissingDependencies(const std::string& pluginName) const;
    
    // Status: FULL - Get plugins that depend on this one
    std::vector<std::string> getDependents(const std::string& pluginName) const;
    
    // ========================================================================
    // Event Hooks
    // ========================================================================
    
    // Status: FULL - Plugin event types
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
    
    // Status: FULL - Register event handler
    int32_t registerEventHandler(PluginEvent event, PluginEventHandler handler);
    
    // Status: FULL - Unregister event handler
    void unregisterEventHandler(int32_t handlerId);
    
    // ========================================================================
    // Execution Context
    // ========================================================================
    
    // Status: FULL - Get current execution context
    const PluginContext* getCurrentContext() const;
    
    // Status: FULL - Check if currently executing a command
    bool isExecuting() const;
    
    // Status: FULL - Get current plugin being executed
    std::string getCurrentPlugin() const;
    
    // ========================================================================
    // Error Handling
    // ========================================================================
    
    // Status: FULL - Get last error message
    std::string getLastError() const;
    
    // Status: FULL - Clear last error
    void clearLastError();
    
    // Status: FULL - Set error handler
    void setErrorHandler(std::function<void(const std::string& pluginName,
                                           const std::string& commandName,
                                           const std::string& error)> handler);

    // Status: FULL - Export structured failure diagnostics as JSONL
    std::string exportFailureDiagnosticsJsonl() const;

    // Status: FULL - Clear accumulated failure diagnostics
    void clearFailureDiagnostics();
    
    // ========================================================================
    // CompatStatus API Surface
    // ========================================================================
    
    // Status: FULL - Get method compatibility status
    CompatStatus getMethodStatus(const std::string& methodName) const;
    
    // Status: FULL - Get method deviation description
    std::string getMethodDeviation(const std::string& methodName) const;
    
    // Status: FULL - Get all registered methods and their status
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
