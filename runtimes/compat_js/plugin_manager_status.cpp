#include "plugin_manager_status.h"

namespace urpg::compat::plugin_manager_detail {

void initializePluginManagerMethodStatus(std::unordered_map<std::string, CompatStatus>& methodStatus,
                                         std::unordered_map<std::string, std::string>& methodDeviations) {
    if (!methodStatus.empty()) {
        return;
    }

    const auto setStatus = [&](const std::string& method, CompatStatus status, const std::string& deviation = "") {
        methodStatus[method] = status;
        if (deviation.empty()) {
            methodDeviations.erase(method);
        } else {
            methodDeviations[method] = deviation;
        }
    };

    setStatus("loadPlugin", CompatStatus::FULL);
    setStatus("unloadPlugin", CompatStatus::FULL);
    setStatus("reloadPlugin", CompatStatus::FULL);
    setStatus("loadPluginsFromDirectory", CompatStatus::FULL);
    setStatus("unloadAllPlugins", CompatStatus::FULL);

    setStatus("registerPlugin", CompatStatus::FULL);
    setStatus("unregisterPlugin", CompatStatus::FULL);
    setStatus("isPluginLoaded", CompatStatus::FULL);
    setStatus("getPluginInfo", CompatStatus::FULL);
    setStatus("getLoadedPlugins", CompatStatus::FULL);

    setStatus("registerCommand", CompatStatus::FULL);
    setStatus("unregisterCommand", CompatStatus::FULL);
    setStatus("unregisterAllCommands", CompatStatus::FULL);
    setStatus("hasCommand", CompatStatus::FULL);
    setStatus("getCommandInfo", CompatStatus::FULL);
    setStatus("getPluginCommands", CompatStatus::FULL);

    setStatus("executeCommand", CompatStatus::FULL);
    setStatus("executeCommandByName", CompatStatus::FULL);
    setStatus("executeCommandAsync", CompatStatus::FULL);
    setStatus("dispatchPendingAsyncCallbacks", CompatStatus::FULL);

    setStatus("setParameter", CompatStatus::FULL);
    setStatus("getParameter", CompatStatus::FULL);
    setStatus("getParameters", CompatStatus::FULL);
    setStatus("parseParameters", CompatStatus::FULL);

    setStatus("checkDependencies", CompatStatus::FULL);
    setStatus("getMissingDependencies", CompatStatus::FULL);
    setStatus("getDependents", CompatStatus::FULL);

    setStatus("registerEventHandler", CompatStatus::FULL);
    setStatus("unregisterEventHandler", CompatStatus::FULL);
    setStatus("triggerEvent", CompatStatus::FULL);

    setStatus("getCurrentContext", CompatStatus::FULL);
    setStatus("isExecuting", CompatStatus::FULL);
    setStatus("getCurrentPlugin", CompatStatus::FULL);
    setStatus("getCurrentCommand", CompatStatus::FULL);

    setStatus("getLastError", CompatStatus::FULL);
    setStatus("clearError", CompatStatus::FULL);
    setStatus("setErrorHandler", CompatStatus::FULL);
    setStatus("exportFailureDiagnosticsJsonl", CompatStatus::FULL);
    setStatus("clearFailureDiagnostics", CompatStatus::FULL);
}

} // namespace urpg::compat::plugin_manager_detail
