#pragma once

#include "engine/core/input/input_core.h"
#include "engine/core/presentation/runtime_feedback_stack.h"

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace urpg::scene {

enum class RuntimeShellState : uint8_t {
    Startup,
    Title,
    ProfileSelect,
    SaveSelect,
    Settings,
    Gameplay,
    Pause,
    QuitConfirm,
    Error,
    Exited
};

enum class RuntimeInputDevice : uint8_t {
    KeyboardMouse,
    GenericController
};

struct RuntimeProfileSummary {
    std::string id;
    std::string display_name;
    bool valid = true;
    std::string diagnostic;
};

struct RuntimeSaveSummary {
    int slot = 0;
    std::string profile_id;
    std::string label;
    bool valid = true;
    std::string diagnostic;
};

struct RuntimeShellCommand {
    std::string id;
    std::string label;
    bool enabled = true;
    std::string disabled_reason;
    std::string glyph;
};

struct RuntimeShellTransition {
    uint64_t sequence = 0;
    RuntimeShellState from = RuntimeShellState::Startup;
    RuntimeShellState to = RuntimeShellState::Startup;
    std::string reason;
    uint32_t duration_ms = 0;
    bool input_acknowledged = false;
};

struct RuntimeShellActionResult {
    bool handled = false;
    bool success = false;
    std::string code;
    std::string message;
};

struct RuntimeShellSnapshot {
    RuntimeShellState state = RuntimeShellState::Startup;
    RuntimeInputDevice active_device = RuntimeInputDevice::KeyboardMouse;
    std::vector<RuntimeShellCommand> commands;
    size_t selected_index = 0;
    std::optional<std::string> active_profile_id;
    std::optional<int> active_save_slot;
    std::string acknowledgement;
    std::string confirm_glyph;
    std::string cancel_glyph;
    std::string menu_glyph;
    std::vector<RuntimeShellTransition> transitions;
};

class RuntimeShellFlow {
public:
    struct Callbacks {
        std::function<RuntimeShellActionResult(const std::string& profile_id)> start_new_game;
        std::function<RuntimeShellActionResult(int slot)> load_game;
        std::function<RuntimeShellActionResult(int slot)> save_game;
        std::function<void()> open_settings;
        std::function<void()> close_settings;
        std::function<void()> request_exit;
    };

    explicit RuntimeShellFlow(Callbacks callbacks = {});

    void setProfiles(std::vector<RuntimeProfileSummary> profiles);
    void setSaves(std::vector<RuntimeSaveSummary> saves);
    void setActiveDevice(RuntimeInputDevice device);
    RuntimeShellActionResult completeStartup(bool success, std::string diagnostic = {});
    RuntimeShellActionResult activate(const std::string& command_id);
    RuntimeShellActionResult handleInput(const urpg::input::InputCore& input);
    RuntimeShellActionResult openPause();
    RuntimeShellActionResult returnFromSettings();
    void advanceFeedback(uint32_t delta_ms) { feedback_stack_.advance(delta_ms); }
    RuntimeShellSnapshot snapshot() const;

    RuntimeShellState state() const { return state_; }
    const RuntimeShellActionResult& lastResult() const { return last_result_; }
    urpg::presentation::RuntimeFeedbackStack& feedbackStack() { return feedback_stack_; }
    const urpg::presentation::RuntimeFeedbackStack& feedbackStack() const { return feedback_stack_; }

private:
    void rebuildCommands();
    void moveSelection(int direction);
    void transition(RuntimeShellState target, std::string reason, uint32_t duration_ms);
    RuntimeShellActionResult result(bool success, std::string code, std::string message);
    std::string glyphFor(urpg::input::InputAction action) const;
    const RuntimeShellCommand* findCommand(const std::string& id) const;
    void emitFeedback(urpg::presentation::RuntimeFeedbackSignal signal,
                      urpg::presentation::RuntimeFeedbackPriority priority,
                      const std::string& visible_text);

    Callbacks callbacks_;
    RuntimeShellState state_ = RuntimeShellState::Startup;
    RuntimeShellState settings_return_state_ = RuntimeShellState::Title;
    RuntimeShellState quit_return_state_ = RuntimeShellState::Title;
    RuntimeInputDevice active_device_ = RuntimeInputDevice::KeyboardMouse;
    std::vector<RuntimeProfileSummary> profiles_;
    std::vector<RuntimeSaveSummary> saves_;
    std::vector<RuntimeShellCommand> commands_;
    size_t selected_index_ = 0;
    std::optional<std::string> active_profile_id_;
    std::optional<int> active_save_slot_;
    std::string acknowledgement_;
    std::vector<RuntimeShellTransition> transitions_;
    RuntimeShellActionResult last_result_;
    uint64_t next_transition_sequence_ = 1;
    urpg::presentation::RuntimeFeedbackStack feedback_stack_;
    uint64_t next_feedback_sequence_ = 1;
};

const char* runtimeShellStateName(RuntimeShellState state);

} // namespace urpg::scene
