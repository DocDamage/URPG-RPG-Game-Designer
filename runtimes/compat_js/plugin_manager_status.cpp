#include "plugin_manager_status.h"

namespace urpg::compat::plugin_manager_detail {

void initializePluginManagerMethodStatus(
    std::unordered_map<std::string, CompatStatus>& methodStatus,
    std::unordered_map<std::string, std::string>& methodDeviations) {
    if (!methodStatus.empty()) {
        return;
    }

    const auto setStatus = [&](const std::string& method,
                               CompatStatus status,
                               const std::string& deviation = "") {
        methodStatus[method] = status;
        if (deviation.empty()) {
            methodDeviations.erase(method);
        } else {
            methodDeviations[method] = deviation;
        }
    };

    setStatus("loadPlugin", CompatStatus::PARTIAL,
              "Loads fixture JSON plugins and stub JS contexts; live MZ plugin runtime loading is not implemented.");
    setStatus("unloadPlugin", CompatStatus::PARTIAL);
    setStatus("reloadPlugin", CompatStatus::PARTIAL,
              "Reload works for tracked fixture-backed plugins, not a general live plugin runtime.");
    setStatus("loadPluginsFromDirectory", CompatStatus::PARTIAL,
              "Directory scan loads fixture-backed plugin descriptors, not a live MZ plugin runtime.");
    setStatus("unloadAllPlugins", CompatStatus::PARTIAL);

    setStatus("registerPlugin", CompatStatus::PARTIAL);
    setStatus("unregisterPlugin", CompatStatus::PARTIAL);
    setStatus("isPluginLoaded", CompatStatus::PARTIAL);
    setStatus("getPluginInfo", CompatStatus::PARTIAL);
    setStatus("getLoadedPlugins", CompatStatus::PARTIAL);

    setStatus("registerCommand", CompatStatus::PARTIAL);
    setStatus("unregisterCommand", CompatStatus::PARTIAL);
    setStatus("unregisterAllCommands", CompatStatus::PARTIAL);
    setStatus("hasCommand", CompatStatus::PARTIAL);
    setStatus("getCommandInfo", CompatStatus::PARTIAL);
    setStatus("getPluginCommands", CompatStatus::PARTIAL);

    setStatus("executeCommand", CompatStatus::PARTIAL,
              "Execution is reliable for registered handlers and fixtures, but still depends on a fixture-backed JS bridge.");
    setStatus("executeCommandByName", CompatStatus::PARTIAL,
              "Qualified-name routing is reliable for registered handlers and fixtures, but still depends on a fixture-backed JS bridge.");
    setStatus("executeCommandAsync", CompatStatus::PARTIAL,
              "FIFO async dispatch works with owning-thread-only callback delivery, but still relies on the fixture-backed JS bridge.");
    setStatus("dispatchPendingAsyncCallbacks", CompatStatus::PARTIAL,
              "Callbacks are explicitly drained on the owning thread; no host event loop integration yet.");

    setStatus("setParameter", CompatStatus::PARTIAL);
    setStatus("getParameter", CompatStatus::PARTIAL);
    setStatus("getParameters", CompatStatus::PARTIAL);
    setStatus("parseParameters", CompatStatus::PARTIAL);

    setStatus("checkDependencies", CompatStatus::PARTIAL);
    setStatus("getMissingDependencies", CompatStatus::PARTIAL);
    setStatus("getDependents", CompatStatus::PARTIAL);

    setStatus("registerEventHandler", CompatStatus::PARTIAL);
    setStatus("unregisterEventHandler", CompatStatus::PARTIAL);
    setStatus("triggerEvent", CompatStatus::PARTIAL);

    setStatus("getCurrentContext", CompatStatus::PARTIAL);
    setStatus("isExecuting", CompatStatus::PARTIAL);
    setStatus("getCurrentPlugin", CompatStatus::PARTIAL);
    setStatus("getCurrentCommand", CompatStatus::PARTIAL);

    setStatus("getLastError", CompatStatus::PARTIAL);
    setStatus("clearError", CompatStatus::PARTIAL);
    setStatus("setErrorHandler", CompatStatus::PARTIAL);
    setStatus("exportFailureDiagnosticsJsonl", CompatStatus::PARTIAL);
    setStatus("clearFailureDiagnostics", CompatStatus::PARTIAL);
}

} // namespace urpg::compat::plugin_manager_detail
