#pragma once

#include <functional>
#include <optional>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace urpg {

/**
 * @brief Thread-safe store for global game state (Switches and Variables).
 * This persists across scenes and is the primary target for Save/Load.
 *
 * Wave 2 Expansion: Added ChangeCallback subscription system for UI/Audio sync.
 */
class GlobalStateHub {
  public:
    static GlobalStateHub& getInstance() {
        static GlobalStateHub instance;
        return instance;
    }

    using Value = std::variant<bool, int32_t, float, std::string>;
    using ChangeCallback = std::function<void(const std::string& id, const Value& newValue)>;

    /**
     * @brief Snapshots current state as the 'baseline' for differential saving.
     */
    void snapshotBaseline() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_baseline_switches = m_switches;
        m_baseline_variables = m_variables;
    }

    /**
     * @brief Gets only switches that differ from the baseline.
     */
    std::unordered_map<std::string, bool> getDiffSwitches() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::unordered_map<std::string, bool> diff;
        for (const auto& [id, val] : m_switches) {
            auto it = m_baseline_switches.find(id);
            if (it == m_baseline_switches.end() || it->second != val) {
                diff[id] = val;
            }
        }
        return diff;
    }

    /**
     * @brief Gets only variables that differ from the baseline.
     */
    std::unordered_map<std::string, Value> getDiffVariables() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::unordered_map<std::string, Value> diff;
        for (const auto& [id, val] : m_variables) {
            auto it = m_baseline_variables.find(id);
            if (it == m_baseline_variables.end() || it->second != val) {
                diff[id] = val;
            }
        }
        return diff;
    }

    /**
     * @brief Sets a global switch (boolean).
     */
    void setSwitch(const std::string& id, bool value) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_switches.count(id) && m_switches[id] == value)
                return;
            m_switches[id] = value;
        }
        notify(id, Value(value));
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
     * @brief Retrieves all active switches for world knowledge serialization.
     */
    std::unordered_map<std::string, bool> getAllSwitches() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_switches;
    }

    /**
     * @brief Sets a global variable.
     */
    void setVariable(const std::string& id, Value value) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_variables.count(id) && m_variables[id] == value)
                return;
            m_variables[id] = value;
        }
        notify(id, value);
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
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_config.count(key) && m_config[key] == value)
                return;
            m_config[key] = value;
        }
        notify(key, Value(value));
    }

    std::string getConfig(const std::string& key, const std::string& defaultVal = "") const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_config.find(key);
        return it != m_config.end() ? it->second : defaultVal;
    }

    /**
     * @brief Subscribe to changes for a specific key or prefix.
     * @param pattern The key to watch (exact match for now), or "*" for all changes.
     * @return A subscription handle.
     */
    uint32_t subscribe(const std::string& pattern, ChangeCallback callback) {
        std::lock_guard<std::mutex> lock(m_mutex);
        uint32_t handle = m_nextHandle++;
        m_subscriptions.push_back({handle, pattern, std::move(callback)});
        return handle;
    }

    /**
     * @brief Diff-First state update for audio/UI sync.
     * Triggers notifications ONLY if the value actually changed.
     */
    template<typename T> void updateState(const std::string& id, T newValue) {
        bool changed = false;
        Value val(newValue);
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if constexpr (std::is_same_v<T, bool>) {
                if (!m_switches.count(id) || m_switches[id] != newValue) {
                    m_switches[id] = newValue;
                    changed = true;
                }
            } else {
                if (!m_variables.count(id) || m_variables[id] != val) {
                    m_variables[id] = val;
                    changed = true;
                }
            }
        }
        if (changed) {
            notify(id, val);
        }
    }

    void unsubscribe(uint32_t handle) {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto it = m_subscriptions.begin(); it != m_subscriptions.end(); ++it) {
            if (it->handle == handle) {
                m_subscriptions.erase(it);
                return;
            }
        }
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
    std::unordered_map<std::string, Value> getAllVariables() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_variables;
    }

    /**
     * @brief Resets all internal state (switches, variables, configs, subscriptions).
     * Primarily used for testing or New Game logic.
     */
    void resetAll() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_switches.clear();
        m_variables.clear();
        m_baseline_switches.clear();
        m_baseline_variables.clear();
        m_config.clear();
        m_subscriptions.clear();
        m_nextHandle = 1;
    }

  private:
    GlobalStateHub() = default;

    void notify(const std::string& id, const Value& value) {
        std::vector<ChangeCallback> callbacks;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            for (const auto& sub : m_subscriptions) {
                const bool exact_match = sub.pattern == id;
                const bool all_match = sub.pattern == "*";
                const bool prefix_match = sub.pattern.size() > 1 && sub.pattern.back() == '*' &&
                                          id.rfind(sub.pattern.substr(0, sub.pattern.size() - 1), 0) == 0;
                if (exact_match || all_match || prefix_match) {
                    callbacks.push_back(sub.callback);
                }
            }
        }
        for (const auto& cb : callbacks) {
            cb(id, value);
        }
    }

    struct Subscription {
        uint32_t handle;
        std::string pattern;
        ChangeCallback callback;
    };

    std::unordered_map<std::string, bool> m_switches;
    std::unordered_map<std::string, Value> m_variables;
    std::unordered_map<std::string, bool> m_baseline_switches;
    std::unordered_map<std::string, Value> m_baseline_variables;
    std::unordered_map<std::string, std::string> m_config;

    std::vector<Subscription> m_subscriptions;
    uint32_t m_nextHandle = 1;

    mutable std::mutex m_mutex;
};

enum class StateBankScope : uint8_t {
    Global,
    Map,
    Self,
    Scoped,
    Js
};

struct StateBankContext {
    std::string map_id;
    std::string event_id;
    std::string scope_id;
};

class ScopedStateBanks {
  public:
    using Value = GlobalStateHub::Value;

    void setSwitch(StateBankScope scope, const StateBankContext& context, const std::string& id, bool value) {
        m_switches[makeKey(scope, context, id)] = value;
    }

    [[nodiscard]] bool getSwitch(StateBankScope scope, const StateBankContext& context, const std::string& id) const {
        const auto it = m_switches.find(makeKey(scope, context, id));
        return it != m_switches.end() ? it->second : false;
    }

    void setVariable(StateBankScope scope, const StateBankContext& context, const std::string& id, Value value) {
        m_variables[makeKey(scope, context, id)] = std::move(value);
    }

    [[nodiscard]] Value getVariable(StateBankScope scope, const StateBankContext& context, const std::string& id) const {
        const auto it = m_variables.find(makeKey(scope, context, id));
        return it != m_variables.end() ? it->second : Value(0);
    }

    [[nodiscard]] std::optional<Value> findVariable(StateBankScope scope,
                                                    const StateBankContext& context,
                                                    const std::string& id) const {
        const auto it = m_variables.find(makeKey(scope, context, id));
        if (it == m_variables.end()) {
            return std::nullopt;
        }
        return it->second;
    }

    [[nodiscard]] size_t switchCount() const { return m_switches.size(); }
    [[nodiscard]] size_t variableCount() const { return m_variables.size(); }

  private:
    static std::string scopeLabel(StateBankScope scope) {
        switch (scope) {
            case StateBankScope::Map: return "map";
            case StateBankScope::Self: return "self";
            case StateBankScope::Scoped: return "scoped";
            case StateBankScope::Js: return "js";
            case StateBankScope::Global: return "global";
        }
        return "global";
    }

    static std::string makeKey(StateBankScope scope, const StateBankContext& context, const std::string& id) {
        return scopeLabel(scope) + ":" + context.map_id + ":" + context.event_id + ":" + context.scope_id + ":" + id;
    }

    std::unordered_map<std::string, bool> m_switches;
    std::unordered_map<std::string, Value> m_variables;
};

} // namespace urpg
