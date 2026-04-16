#pragma once

#include <string>
#include <vector>
#include <memory>
#include <variant>
#include <functional>

namespace urpg::editor {

/**
 * @brief Represents a value passed between the C++ engine and the Scripting Bridge.
 */
using BridgeValue = std::variant<std::monostate, bool, int, float, std::string>;

/**
 * @brief Interface for the high-level scripting bridge (QuickJS).
 * 
 * Wave 7.3 implementation: JavaScript/QuickJS Bridge.
 */
class ScriptBridge {
public:
    static ScriptBridge& instance() {
        static ScriptBridge inst;
        return inst;
    }

    // Initialization
    void startup();
    void shutdown();

    // Script Execution
    BridgeValue eval(const std::string& code);
    void executeModule(const std::string& modulePath);

    // native <-> JS Registration
    using NativeCallback = std::function<BridgeValue(const std::vector<BridgeValue>&)>;
    void registerNative(const std::string& name, NativeCallback callback);

    // Asset Hooks
    void onAssetUpdate(const std::string& path);

    // Diagnostics
    std::string getLastError() const { return m_lastError; }
    void clearError() { m_lastError.clear(); }

private:
    ScriptBridge() = default;

    std::string m_lastError;
    // placeholder for the QuickJS JSContext* and JSRuntime* 
    void* m_qjsRuntime = nullptr;
    void* m_qjsContext = nullptr;

    void logError(const std::string& msg) { m_lastError = msg; }
};

} // namespace urpg::editor
