#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>

#include <nlohmann/json.hpp>

#include "engine/core/input/input_core.h"

namespace urpg::input {

/**
 * @brief Persistable store for custom key-to-action mappings.
 *
 * InputRemapStore maintains the active binding set as well as the
 * factory-default (MZ-compatible) bindings.  It can serialize to
 * and deserialize from a versioned JSON format.
 */
class InputRemapStore {
public:
    InputRemapStore();

    /**
     * @brief Replace the active mapping for a key code.
     */
    void setMapping(int32_t keyCode, InputAction action);

    /**
     * @brief Remove a key code from the active mapping set.
     */
    void removeMapping(int32_t keyCode);

    /**
     * @brief Lookup the action bound to a key code, if any.
     */
    [[nodiscard]] std::optional<InputAction> getMapping(int32_t keyCode) const;

    /**
     * @brief Snapshot of the full active mapping set.
     */
    [[nodiscard]] std::map<int32_t, InputAction> getAllMappings() const;

    /**
     * @brief Restore the baked-in default mappings and clear custom ones.
     */
    void resetToDefaults();

    /**
     * @brief Clear all active mappings (including defaults).
     */
    void clear();

    /**
     * @brief Serialize the active mapping set to JSON.
     */
    [[nodiscard]] nlohmann::json saveToJson() const;

    /**
     * @brief Deserialize and replace the active mapping set from JSON.
     * @throws std::invalid_argument if the version field is missing or incompatible.
     */
    void loadFromJson(const nlohmann::json& j);

    /**
     * @brief True if the store has been mutated since the last load/reset.
     */
    [[nodiscard]] bool hasUnsavedChanges() const;

private:
    std::map<int32_t, InputAction> m_mappings;
    const std::map<int32_t, InputAction> m_defaults;
    bool m_unsavedChanges{false};

    static std::map<int32_t, InputAction> buildDefaultMappings();
    static std::string actionToString(InputAction action);
    static InputAction stringToAction(const std::string& name);
};

} // namespace urpg::input
