#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <vector>

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
    Confirm, // 'Z', 'Enter', 'Space'
    Cancel,  // 'X', 'Esc'
    Menu,    // 'C', 'Tab'
    Debug    // '~'
};

/**
 * @brief State of an individual action.
 */
enum class ActionState : uint8_t { Pressed, Released, Held };

/**
 * @brief Native Input Core for event handling and action mapping.
 */
class InputCore {
  public:
    using ActionHandler = std::function<void(InputAction, ActionState)>;

    /**
     * @brief Maps a raw key code (from window system) to a conceptual InputAction.
     */
    void mapKey(int32_t keyCode, InputAction action) { m_keyMaps[keyCode] = action; }

    /**
     * @brief Processes a raw key event from the OS/Platform layer.
     */
    void processKeyEvent(int32_t keyCode, ActionState state) {
        // Fallback or explicit mapping?
        // For MZ compatibility, we typically map certain keys to core actions by default.
        if (auto it = m_keyMaps.find(keyCode); it != m_keyMaps.end()) {
            InputAction action = it->second;
            updateActionState(action, state);
        } else {
            // Try default MZ mapping for keys that aren't explicitly mapped yet
            // This allows the EngineShell to start simple.
        }
    }

    /**
     * @brief Provides a way to manually update an action state (e.g., from a mapping function).
     */
    void updateActionState(InputAction action, ActionState state) {
        m_actionStates[action] = state;
        for (auto& handler : m_handlers) {
            handler(action, state);
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

    /**
     * @brief Checks whether an action was newly pressed on the current input tick.
     */
    bool isActionJustPressed(InputAction action) const {
        if (auto it = m_actionStates.find(action); it != m_actionStates.end()) {
            return it->second == ActionState::Pressed;
        }
        return false;
    }

    void addHandler(ActionHandler handler) { m_handlers.push_back(std::move(handler)); }

  private:
    std::map<int32_t, InputAction> m_keyMaps;
    std::map<InputAction, ActionState> m_actionStates;
    std::vector<ActionHandler> m_handlers;
};

} // namespace urpg::input
