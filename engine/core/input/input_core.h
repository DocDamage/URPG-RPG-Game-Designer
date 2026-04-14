#pragma once

#include <vector>
#include <string>
#include <map>
#include <functional>
#include <cstdint>

namespace urpg::input {

/**
 * @brief Native representation of Input Actions (e.g., UP, OK, CANCEL).
 * This abstracts away specific keys (like 'W' or 'ArrowUp').
 */
enum class InputAction : uint32_t {
    None = 0,
    MoveUp,
    MoveDown,
    MoveLeft,
    MoveRight,
    Confirm,    // 'Z', 'Enter', 'Space'
    Cancel,     // 'X', 'Esc'
    Menu,       // 'C', 'Tab'
    Debug       // '~'
};

/**
 * @brief State of an individual action.
 */
enum class ActionState : uint8_t {
    Pressed,
    Released,
    Held
};

/**
 * @brief Native Input Core for event handling and action mapping.
 */
class InputCore {
public:
    using ActionHandler = std::function<void(InputAction, ActionState)>;

    /**
     * @brief Maps a raw key code (from window system) to a conceptual InputAction.
     */
    void mapKey(int32_t keyCode, InputAction action) {
        m_keyMaps[keyCode] = action;
    }

    /**
     * @brief Processes a raw key event from the OS/Platform layer.
     */
    void processKeyEvent(int32_t keyCode, ActionState state) {
        if (auto it = m_keyMaps.find(keyCode); it != m_keyMaps.end()) {
            InputAction action = it->second;
            m_actionStates[action] = state;
            
            // Invoke handlers
            for (auto& handler : m_handlers) {
                handler(action, state);
            }
        }
    }

    /**
     * @brief Checks if an action is currently in a specific state.
     */
    bool isActionActive(InputAction action) const {
        if (auto it = m_actionStates.find(action); it != m_actionStates.end()) {
            return it->second == ActionState::Pressed || it->second == ActionState::Held;
        }
        return false;
    }

    void addHandler(ActionHandler handler) {
        m_handlers.push_back(std::move(handler));
    }

private:
    std::map<int32_t, InputAction> m_keyMaps;
    std::map<InputAction, ActionState> m_actionStates;
    std::vector<ActionHandler> m_handlers;
};

} // namespace urpg::input
