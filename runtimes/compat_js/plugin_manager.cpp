// PluginManager - MZ Plugin Command Registry + Execution - Implementation
// Phase 2 - Compat Layer

#include "plugin_manager.h"
#include <algorithm>
#include <cassert>

namespace urpg {
namespace compat {

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
    
    PluginManagerImpl() = default;
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
    // TODO: Implement actual plugin loading from file
    // For now, just register a placeholder
    impl_->lastError_.clear();
    
    // Extract plugin name from path (simplified)
    std::string name = path;
    size_t lastSlash = name.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        name = name.substr(lastSlash + 1);
    }
    size_t lastDot = name.find_last_of('.');
    if (lastDot != std::string::npos) {
        name = name.substr(0, lastDot);
    }
    
    PluginInfo info;
    info.name = name;
    info.loaded = true;
    info.enabled = true;
    
    impl_->plugins_[name] = info;
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
    
    // Store the path (would need to track this properly)
    std::string path = it->second.name; // Simplified
    
    // Unload and reload
    if (!unloadPlugin(name)) {
        return false;
    }
    
    return loadPlugin(path);
}

int32_t PluginManager::loadPluginsFromDirectory(const std::string& directory) {
    // TODO: Implement directory scanning
    // For now, return 0
    impl_->lastError_.clear();
    return 0;
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
    size_t underscorePos = fullName.find('_');
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
    // TODO: Implement JSON parsing
    // For now, return true
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
