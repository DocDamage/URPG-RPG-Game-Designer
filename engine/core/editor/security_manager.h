#pragma once

#include <string>
#include <vector>
#include <bitset>

namespace urpg::editor {

/**
 * @brief Categorized permissions for plugins to access engine subsystems.
 */
enum class PluginPermission : uint16_t {
    None = 0,
    FileSystemRead = 1 << 0,
    FileSystemWrite = 1 << 1,
    NetworkAccess = 1 << 2,
    GlobalStateEdit = 1 << 3,
    ECSDestructive = 1 << 4,
    InputSpying = 1 << 5,
    SceneNavigation = 1 << 6,
    Count = 7
};

/**
 * @brief Manages the security context and permission validation for plugins.
 * 
 * Part of the Post-Gold Security Hardening track.
 */
class PluginSecurityManager {
public:
    static PluginSecurityManager& instance() {
        static PluginSecurityManager inst;
        return inst;
    }

    /**
     * @brief Request a specific permission for a plugin ID.
     * @return True if granted (automatically or by user), false otherwise.
     */
    bool requestPermission(const std::string& pluginId, PluginPermission permission);

    /**
     * @brief Hard-check if a plugin currently has a permission.
     */
    bool hasPermission(const std::string& pluginId, PluginPermission permission) const;

    /**
     * @brief Revoke all permissions for a plugin.
     */
    void revokeAll(const std::string& pluginId);

    /**
     * @brief Set the default policy for new plugins.
     */
    void setDefaultPolicy(std::bitset<static_cast<size_t>(PluginPermission::Count)> policy) { m_defaultPolicy = policy; }

private:
    PluginSecurityManager() { 
        // Default: least privilege (empty)
    }

    std::map<std::string, std::bitset<static_cast<size_t>(PluginPermission::Count)>> m_pluginPerms;
    std::bitset<static_cast<size_t>(PluginPermission::Count)> m_defaultPolicy;

    void logAudit(const std::string& pluginId, PluginPermission permission, bool granted);
};

} // namespace urpg::editor
