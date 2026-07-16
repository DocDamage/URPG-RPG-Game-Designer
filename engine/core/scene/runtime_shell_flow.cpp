#include "engine/core/scene/runtime_shell_flow.h"
#include "engine/core/accessibility/inclusive_runtime_policy.h"

#include <algorithm>
#include <set>
#include <tuple>
#include <utility>

namespace urpg::scene {

const char* runtimeShellStateName(const RuntimeShellState state) {
    switch (state) {
    case RuntimeShellState::Startup: return "startup";
    case RuntimeShellState::Title: return "title";
    case RuntimeShellState::ProfileSelect: return "profile_select";
    case RuntimeShellState::SaveSelect: return "save_select";
    case RuntimeShellState::Settings: return "settings";
    case RuntimeShellState::Gameplay: return "gameplay";
    case RuntimeShellState::Pause: return "pause";
    case RuntimeShellState::QuitConfirm: return "quit_confirm";
    case RuntimeShellState::Error: return "error";
    case RuntimeShellState::Exited: return "exited";
    }
    return "unknown";
}

RuntimeShellFlow::RuntimeShellFlow(Callbacks callbacks) : callbacks_(std::move(callbacks)) {
    rebuildCommands();
}

bool RuntimeShellFlow::setInclusiveSettings(const urpg::accessibility::InclusiveSettings& settings) {
    return urpg::accessibility::applyInclusiveRuntimePolicy(settings, &feedback_stack_, nullptr, nullptr);
}

void RuntimeShellFlow::setProfiles(std::vector<RuntimeProfileSummary> profiles) {
    std::set<std::string> ids;
    profiles.erase(std::remove_if(profiles.begin(), profiles.end(), [&](const auto& profile) {
        return profile.id.empty() || profile.display_name.empty() || !ids.insert(profile.id).second;
    }), profiles.end());
    std::stable_sort(profiles.begin(), profiles.end(), [](const auto& left, const auto& right) {
        return std::tie(left.display_name, left.id) < std::tie(right.display_name, right.id);
    });
    profiles_ = std::move(profiles);
    rebuildCommands();
}

void RuntimeShellFlow::setSaves(std::vector<RuntimeSaveSummary> saves) {
    std::set<int> slots;
    saves.erase(std::remove_if(saves.begin(), saves.end(), [&](const auto& save) {
        return save.slot < 0 || save.profile_id.empty() || save.label.empty() || !slots.insert(save.slot).second;
    }), saves.end());
    std::stable_sort(saves.begin(), saves.end(), [](const auto& left, const auto& right) { return left.slot < right.slot; });
    saves_ = std::move(saves);
    rebuildCommands();
}

void RuntimeShellFlow::setActiveDevice(const RuntimeInputDevice device) {
    active_device_ = device;
    rebuildCommands();
}

RuntimeShellActionResult RuntimeShellFlow::completeStartup(const bool success, std::string diagnostic) {
    if (state_ != RuntimeShellState::Startup) return result(false, "startup_already_completed", "Startup is already complete.");
    if (!success) {
        acknowledgement_ = diagnostic.empty() ? "Startup failed. Review diagnostics or retry." : std::move(diagnostic);
        transition(RuntimeShellState::Error, "startup_failed", 0);
        rebuildCommands();
        return result(false, "startup_failed", acknowledgement_);
    }
    transition(RuntimeShellState::Title, "startup_ready", 160);
    rebuildCommands();
    emitFeedback(urpg::presentation::RuntimeFeedbackSignal::Confirm,
                 urpg::presentation::RuntimeFeedbackPriority::Primary, "Startup complete");
    return result(true, "startup_ready", "Startup complete. Title controls are ready.");
}

RuntimeShellActionResult RuntimeShellFlow::activate(const std::string& command_id) {
    const auto* command = findCommand(command_id);
    if (command == nullptr) return result(false, "shell_command_missing", "The selected shell command is unavailable.");
    if (!command->enabled) return result(false, "shell_command_disabled", command->disabled_reason);
    const auto command_label = command->label;

    if (command_id == "new_game") {
        transition(RuntimeShellState::ProfileSelect, "new_game_selected", 160);
    } else if (command_id == "continue") {
        transition(RuntimeShellState::SaveSelect, "continue_selected", 160);
    } else if (command_id == "settings") {
        settings_return_state_ = state_;
        transition(RuntimeShellState::Settings, "settings_opened", 160);
        if (callbacks_.open_settings) callbacks_.open_settings();
    } else if (command_id == "quit" || command_id == "quit_game") {
        quit_return_state_ = state_;
        transition(RuntimeShellState::QuitConfirm, "quit_confirmation_opened", 90);
    } else if (command_id == "cancel_quit") {
        transition(quit_return_state_, "quit_cancelled", 90);
    } else if (command_id == "confirm_quit") {
        transition(RuntimeShellState::Exited, "quit_confirmed", 160);
        if (callbacks_.request_exit) callbacks_.request_exit();
    } else if (command_id == "resume") {
        transition(RuntimeShellState::Gameplay, "pause_resumed", 90);
    } else if (command_id == "quit_to_title") {
        transition(RuntimeShellState::Title, "returned_to_title", 160);
        active_save_slot_.reset();
    } else if (command_id == "save") {
        if (!active_save_slot_) active_save_slot_ = 0;
        if (!callbacks_.save_game) return result(false, "save_handler_missing", "Save service is unavailable.");
        const auto saved = callbacks_.save_game(*active_save_slot_);
        acknowledgement_ = saved.message;
        last_result_ = saved;
        emitFeedback(saved.success ? urpg::presentation::RuntimeFeedbackSignal::Confirm
                                   : urpg::presentation::RuntimeFeedbackSignal::Error,
                     saved.success ? urpg::presentation::RuntimeFeedbackPriority::Secondary
                                   : urpg::presentation::RuntimeFeedbackPriority::Critical,
                     saved.message.empty() ? (saved.success ? "Save complete" : "Save failed") : saved.message);
        rebuildCommands();
        return last_result_;
    } else if (command_id == "recover") {
        transition(RuntimeShellState::Title, "error_recovered", 160);
    } else if (command_id.starts_with("profile:")) {
        const auto id = command_id.substr(8);
        const auto profile = std::find_if(profiles_.begin(), profiles_.end(), [&](const auto& row) { return row.id == id; });
        if (profile == profiles_.end() || !profile->valid) return result(false, "profile_invalid", profile == profiles_.end() ? "Profile is missing." : profile->diagnostic);
        if (!callbacks_.start_new_game) return result(false, "new_game_handler_missing", "New-game service is unavailable.");
        const auto started = callbacks_.start_new_game(id);
        if (!started.success) return result(false, started.code, started.message);
        active_profile_id_ = id;
        active_save_slot_.reset();
        transition(RuntimeShellState::Gameplay, "profile_started", 240);
    } else if (command_id.starts_with("save:")) {
        const auto slot = std::stoi(command_id.substr(5));
        const auto save = std::find_if(saves_.begin(), saves_.end(), [&](const auto& row) { return row.slot == slot; });
        if (save == saves_.end() || !save->valid) return result(false, "save_invalid", save == saves_.end() ? "Save slot is missing." : save->diagnostic);
        if (!callbacks_.load_game) return result(false, "load_handler_missing", "Load service is unavailable.");
        const auto loaded = callbacks_.load_game(slot);
        if (!loaded.success) return result(false, loaded.code, loaded.message);
        active_profile_id_ = save->profile_id;
        active_save_slot_ = slot;
        transition(RuntimeShellState::Gameplay, "save_loaded", 240);
    } else if (command_id == "back") {
        transition(RuntimeShellState::Title, "shell_back", 90);
    } else {
        return result(false, "shell_command_unhandled", "The shell command has no action.");
    }
    rebuildCommands();
    const auto signal = command_id == "cancel_quit" || command_id == "back"
                            ? urpg::presentation::RuntimeFeedbackSignal::Cancel
                            : urpg::presentation::RuntimeFeedbackSignal::Confirm;
    emitFeedback(signal, urpg::presentation::RuntimeFeedbackPriority::Secondary,
                 "Input acknowledged: " + command_label);
    return result(true, "shell_command_completed", "Input acknowledged: " + command_label + ".");
}

RuntimeShellActionResult RuntimeShellFlow::handleInput(const urpg::input::InputCore& input) {
    if (state_ == RuntimeShellState::Gameplay && input.isActionJustPressed(urpg::input::InputAction::Menu)) return openPause();
    if (state_ == RuntimeShellState::Settings && input.isActionJustPressed(urpg::input::InputAction::Cancel))
        return returnFromSettings();
    if (commands_.empty()) return result(false, "shell_no_commands", "No shell commands are available.");
    if (input.isActionJustPressed(urpg::input::InputAction::MoveUp)) {
        moveSelection(-1);
        return result(true, "shell_focus_moved", "Focus moved to " + commands_[selected_index_].label + ".");
    }
    if (input.isActionJustPressed(urpg::input::InputAction::MoveDown)) {
        moveSelection(1);
        return result(true, "shell_focus_moved", "Focus moved to " + commands_[selected_index_].label + ".");
    }
    if (input.isActionJustPressed(urpg::input::InputAction::Confirm)) return activate(commands_[selected_index_].id);
    if (input.isActionJustPressed(urpg::input::InputAction::Cancel)) {
        if (state_ == RuntimeShellState::QuitConfirm) return activate("cancel_quit");
        if (state_ == RuntimeShellState::ProfileSelect || state_ == RuntimeShellState::SaveSelect) return activate("back");
        if (state_ == RuntimeShellState::Pause) return activate("resume");
    }
    return {false, false, "shell_input_ignored", "Input did not map to the current shell."};
}

RuntimeShellActionResult RuntimeShellFlow::openPause() {
    if (state_ != RuntimeShellState::Gameplay) return result(false, "pause_unavailable", "Pause is available only during gameplay.");
    transition(RuntimeShellState::Pause, "pause_opened", 90);
    rebuildCommands();
    emitFeedback(urpg::presentation::RuntimeFeedbackSignal::Confirm,
                 urpg::presentation::RuntimeFeedbackPriority::Secondary, "Pause opened");
    return result(true, "pause_opened", "Paused. Focus is on Resume.");
}

RuntimeShellActionResult RuntimeShellFlow::returnFromSettings() {
    if (state_ != RuntimeShellState::Settings) return result(false, "settings_not_open", "Settings are not open.");
    if (callbacks_.close_settings) callbacks_.close_settings();
    transition(settings_return_state_, "settings_closed", 160);
    rebuildCommands();
    emitFeedback(urpg::presentation::RuntimeFeedbackSignal::Cancel,
                 urpg::presentation::RuntimeFeedbackPriority::Secondary, "Settings closed");
    return result(true, "settings_closed", "Settings closed and previous focus restored.");
}

RuntimeShellSnapshot RuntimeShellFlow::snapshot() const {
    return {state_, active_device_, commands_, selected_index_, active_profile_id_, active_save_slot_, acknowledgement_,
            glyphFor(urpg::input::InputAction::Confirm), glyphFor(urpg::input::InputAction::Cancel),
            glyphFor(urpg::input::InputAction::Menu), transitions_};
}

void RuntimeShellFlow::rebuildCommands() {
    commands_.clear();
    const auto add = [&](std::string id, std::string label, const bool enabled = true, std::string reason = {}) {
        commands_.push_back({std::move(id), std::move(label), enabled, std::move(reason), glyphFor(urpg::input::InputAction::Confirm)});
    };
    switch (state_) {
    case RuntimeShellState::Startup: break;
    case RuntimeShellState::Title:
        add("new_game", "New Game");
        add("continue", "Continue", !saves_.empty(), saves_.empty() ? "No save data found." : "");
        add("settings", "Settings"); add("quit", "Quit"); break;
    case RuntimeShellState::ProfileSelect:
        for (const auto& profile : profiles_) add("profile:" + profile.id, profile.display_name, profile.valid, profile.diagnostic);
        add("back", "Back"); break;
    case RuntimeShellState::SaveSelect:
        for (const auto& save : saves_) add("save:" + std::to_string(save.slot), save.label, save.valid, save.diagnostic);
        add("back", "Back"); break;
    case RuntimeShellState::Settings: break;
    case RuntimeShellState::Gameplay: break;
    case RuntimeShellState::Pause:
        add("resume", "Resume"); add("save", "Save Game"); add("settings", "Settings");
        add("quit_to_title", "Return to Title"); add("quit_game", "Quit Game"); break;
    case RuntimeShellState::QuitConfirm:
        add("confirm_quit", "Confirm Quit"); add("cancel_quit", "Cancel"); break;
    case RuntimeShellState::Error: add("recover", "Return to Title"); break;
    case RuntimeShellState::Exited: break;
    }
    selected_index_ = commands_.empty() ? 0 : std::min(selected_index_, commands_.size() - 1);
}

void RuntimeShellFlow::moveSelection(const int direction) {
    if (commands_.empty()) return;
    const auto count = commands_.size();
    selected_index_ = direction < 0 ? (selected_index_ + count - 1) % count : (selected_index_ + 1) % count;
    emitFeedback(urpg::presentation::RuntimeFeedbackSignal::Focus,
                 urpg::presentation::RuntimeFeedbackPriority::Ambient,
                 "Focus moved to " + commands_[selected_index_].label);
}

void RuntimeShellFlow::transition(const RuntimeShellState target, std::string reason, const uint32_t duration_ms) {
    const auto bounded = std::min(duration_ms, uint32_t{250});
    transitions_.push_back({next_transition_sequence_++, state_, target, std::move(reason), bounded, true});
    if (transitions_.size() > 128) transitions_.erase(transitions_.begin());
    state_ = target;
}

RuntimeShellActionResult RuntimeShellFlow::result(const bool success, std::string code, std::string message) {
    acknowledgement_ = message;
    last_result_ = {true, success, std::move(code), std::move(message)};
    if (!success) {
        emitFeedback(urpg::presentation::RuntimeFeedbackSignal::Error,
                     urpg::presentation::RuntimeFeedbackPriority::Critical, acknowledgement_);
    }
    rebuildCommands();
    return last_result_;
}

void RuntimeShellFlow::emitFeedback(const urpg::presentation::RuntimeFeedbackSignal signal,
                                    const urpg::presentation::RuntimeFeedbackPriority priority,
                                    const std::string& visible_text) {
    (void)feedback_stack_.submit({"shell:" + std::to_string(next_feedback_sequence_++), signal, priority,
                                  visible_text.empty() ? "Runtime shell feedback" : visible_text, {}});
}

std::string RuntimeShellFlow::glyphFor(const urpg::input::InputAction action) const {
    if (active_device_ == RuntimeInputDevice::GenericController) {
        switch (action) {
        case urpg::input::InputAction::Confirm: return "urpg.glyph.controller.face_bottom";
        case urpg::input::InputAction::Cancel: return "urpg.glyph.controller.face_right";
        case urpg::input::InputAction::Menu: return "urpg.glyph.controller.menu";
        default: return "urpg.glyph.controller.dpad";
        }
    }
    switch (action) {
    case urpg::input::InputAction::Confirm: return "urpg.glyph.keyboard.enter";
    case urpg::input::InputAction::Cancel: return "urpg.glyph.keyboard.escape";
    case urpg::input::InputAction::Menu: return "urpg.glyph.keyboard.tab";
    default: return "urpg.glyph.keyboard.arrows";
    }
}

const RuntimeShellCommand* RuntimeShellFlow::findCommand(const std::string& id) const {
    const auto found = std::find_if(commands_.begin(), commands_.end(), [&](const auto& command) { return command.id == id; });
    return found == commands_.end() ? nullptr : &*found;
}

} // namespace urpg::scene
