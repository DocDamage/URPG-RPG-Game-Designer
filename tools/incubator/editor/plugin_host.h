#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <map>

namespace urpg::editor {

/**
 * @brief Represents the current state of a plugin within the host.
 */
enum class PluginStatus {
    Unloaded,
    Loading,
    Active,
    Error,
    Disabled
};

/**
 * @brief Metadata for a native or scripted plugin.
 */
struct PluginInfo {
    std::string id;
    std::string name;
    std::string version;
    std::string author;
    std::string description;
    std::vector<std::string> dependencies;
    bool isNative = false;
    PluginStatus status = PluginStatus::Unloaded;
};

/**
 * @brief Interface for a native URPG plugin.
 */
class IURPGPlugin {
public:
    virtual ~IURPGPlugin() = default;
    virtual void onInitialize() = 0;
    virtual void onShutdown() = 0;
    virtual void onTick(float deltaTime) = 0;
    virtual const PluginInfo& getInfo() const = 0;
};

/**
 * @brief Manages the lifecycle and discovery of native/scripted plugins.
 *
 * Incubating harness seam retained only for the stale EngineAssembly path.
 */
class PluginHost {
public:
    static PluginHost& instance() {
        static PluginHost inst;
        return inst;
    }

    // Lifecycle
    void discoverPlugins(const std::string& directory);
    bool loadPlugin(const std::string& pluginId);
    void unloadPlugin(const std::string& pluginId);
    void reloadPlugin(const std::string& pluginId);

    // native Registration
    void registerNativePlugin(std::unique_ptr<IURPGPlugin> plugin);

    // Queries
    const std::vector<PluginInfo>& getAvailablePlugins() const { return m_availablePlugins; }
    IURPGPlugin* getActivePlugin(const std::string& id) const;

    // Execution
    void tickActivePlugins(float deltaTime);

    // Error Handling
    std::string getLastErrorMessage() const { return m_lastError; }

private:
    PluginHost() = default;
    
    std::vector<PluginInfo> m_availablePlugins;
    std::map<std::string, std::unique_ptr<IURPGPlugin>> m_activePlugins;
    std::string m_lastError;

    // Internal helpers
    void logError(const std::string& msg) { m_lastError = msg; }
};

} // namespace urpg::editor
