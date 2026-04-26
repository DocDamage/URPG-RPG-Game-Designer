#include "security_manager.h"
#include "engine/core/diagnostics/runtime_diagnostics.h"
#include <algorithm>

namespace urpg::editor {

bool PluginSecurityManager::requestPermission(const std::string& pluginId, PluginPermission permission) {
    if (pluginId.empty())
        return false;

    // Check if we already have it
    if (hasPermission(pluginId, permission))
        return true;

    // Check if it's in the default policy
    if (m_defaultPolicy.test(static_cast<size_t>(permission))) {
        m_pluginPerms[pluginId].set(static_cast<size_t>(permission));
        logAudit(pluginId, permission, true);
        return true;
    }

    // In a real editor, this would pop-up a UI permission prompt (Wave 6.1 compat)
    // For now, we simulate a 'Deny' of unsafe permissions.
    if (permission == PluginPermission::FileSystemWrite || permission == PluginPermission::NetworkAccess) {
        logAudit(pluginId, permission, false);
        return false;
    }

    // Auto-grant benign ones
    m_pluginPerms[pluginId].set(static_cast<size_t>(permission));
    logAudit(pluginId, permission, true);
    return true;
}

bool PluginSecurityManager::hasPermission(const std::string& pluginId, PluginPermission permission) const {
    auto it = m_pluginPerms.find(pluginId);
    if (it != m_pluginPerms.end()) {
        return it->second.test(static_cast<size_t>(permission));
    }
    return false;
}

void PluginSecurityManager::revokeAll(const std::string& pluginId) {
    m_pluginPerms.erase(pluginId);
}

void PluginSecurityManager::logAudit(const std::string& pluginId, PluginPermission permission, bool granted) {
    std::string permStr;
    switch (permission) {
    case PluginPermission::FileSystemRead:
        permStr = "FileSystemRead";
        break;
    case PluginPermission::FileSystemWrite:
        permStr = "FileSystemWrite";
        break;
    case PluginPermission::NetworkAccess:
        permStr = "NetworkAccess";
        break;
    case PluginPermission::GlobalStateEdit:
        permStr = "GlobalStateEdit";
        break;
    case PluginPermission::ECSDestructive:
        permStr = "ECSDestructive";
        break;
    default:
        permStr = "Unknown";
        break;
    }

    urpg::diagnostics::RuntimeDiagnostics::emit(
        granted ? urpg::diagnostics::DiagnosticSeverity::Info : urpg::diagnostics::DiagnosticSeverity::Warning,
        "editor.security", granted ? "plugin.permission.granted" : "plugin.permission.denied",
        "Plugin '" + pluginId + "' requested " + permStr + (granted ? ": GRANTED" : ": DENIED"));
}

} // namespace urpg::editor
