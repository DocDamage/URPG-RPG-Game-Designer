#pragma once

#include <string>
#include <unordered_map>
#include <variant>
#include <mutex>

namespace urpg {

/**
 * @brief Thread-safe store for global game state (Switches and Variables).
 * This persists across scenes and is the primary target for Save/Load.
 */
class GlobalStateHub {
public:
    static GlobalStateHub& getInstance() {
        static GlobalStateHub instance;
        return instance;
    }

    using Value = std::variant<bool, int32_t, float, std::string>;

    /**
     * @brief Sets a global switch (boolean).
     */
    void setSwitch(const std::string& id, bool value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_switches[id] = value;
    }

    /**
     * @brief Gets a global switch value. Returns false if not found.
     */
    bool getSwitch(const std::string& id) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_switches.find(id);
        return it != m_switches.end() ? it->second : false;
    }

    /**
     * @brief Sets a global variable.
     */
    void setVariable(const std::string& id, Value value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_variables[id] = value;
    }

    /**
     * @brief Gets a global variable value.
     */
    Value getVariable(const std::string& id) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_variables.find(id);
        return it != m_variables.end() ? it->second : Value(0);
    }

    /**
     * @brief Sets an engine configuration value (e.g. "ui.scale", "audio.master_volume").
     */
    void setConfig(const std::string& key, const std::string& value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_config[key] = value;
    }

    std::string getConfig(const std::string& key, const std::string& defaultVal = "") const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_config.find(key);
        return it != m_config.end() ? it->second : defaultVal;
    }

    /**
     * @brief Clears all session-specific state (switches/variables) but keeps config.
     */
    void clearSessionState() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_switches.clear();
        m_variables.clear();
    }

    // Accessors for serialization
    const std::unordered_map<std::string, bool>& getAllSwitches() const { return m_switches; }
    const std::unordered_map<std::string, Value>& getAllVariables() const { return m_variables; }

private:
    GlobalStateHub() = default;

    std::unordered_map<std::string, bool> m_switches;
    std::unordered_map<std::string, Value> m_variables;
    std::unordered_map<std::string, std::string> m_config;
    mutable std::mutex m_mutex;
};

} // namespace urpg
