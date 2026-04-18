#pragma once

#include <string>
#include <vector>
#include <variant>
#include <memory>
#include <cstdint>

namespace urpg {

/**
 * @brief Opcode IDs for RPG Maker MZ style event commands.
 */
enum class EventOpcode : uint32_t {
    ShowMessage = 101,
    ShowChoices = 102,
    InputNumber = 103,
    ControlSwitches = 121,
    ControlVariables = 122,
    ControlSelfSwitch = 123,
    TransferPlayer = 201,
    SetMovementRoute = 205,
    PlaySE = 250,
    PlayBGM = 241,
    BattleProcessing = 301,
    Script = 355,
    End = 0
};

/**
 * @brief Representation of a single event command and its parameters.
 */
struct EventCommand {
    EventOpcode code = EventOpcode::End;
    int32_t indent = 0;
    std::vector<std::variant<int32_t, std::string, bool>> parameters;
};

/**
 * @brief Native interpreter for map event command lists.
 */
class MapEventInterpreter {
public:
    MapEventInterpreter(const std::vector<EventCommand>& commands) 
        : m_commands(commands), m_index(0) {}

    bool isRunning() const { return m_index < m_commands.size(); }

    /**
     * @brief Executes the current command and advances.
     * @return True if we should continue immediately, false if we need to wait (e.g. Message).
     */
    bool update() {
        if (!isRunning()) return false;

        const auto& cmd = m_commands[m_index];
        bool wait = processCommand(cmd);
        
        m_index++;
        return !wait;
    }

    int getIndex() const { return m_index; }

private:
    /**
     * @brief Core dispatcher for opcodes.
     * @return True if this command causes a "Wait" state.
     */
    bool processCommand(const EventCommand& cmd) {
        switch (cmd.code) {
            case EventOpcode::ShowMessage:
                // Log for diagnostics/test
                return true; // Wait for user input
            case EventOpcode::ControlSwitches:
                // Logic for setting globally
                return false;
            case EventOpcode::TransferPlayer:
                // Scene transition logic
                return true; 
            default:
                return false;
        }
    }

    std::vector<EventCommand> m_commands;
    size_t m_index;
};

} // namespace urpg
