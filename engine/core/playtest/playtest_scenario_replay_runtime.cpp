#include "engine/core/playtest/playtest_scenario_replay_runtime.h"

namespace urpg::playtest {

bool PlaytestScenarioReplayRuntime::start(std::string session_id, const nlohmann::json& initial_state,
                                          std::string* diagnostic) {
    if (session_id.empty() || !initial_state.is_object() || config_.project_revision.empty() ||
        config_.runtime_version.empty()) {
        if (diagnostic) *diagnostic = "scenario_replay_runtime_config_invalid";
        return false;
    }
    session_id_ = std::move(session_id);
    capture_.emplace(config_);
    const auto checkpoint = capture_->captureCheckpoint(0, "launch", initial_state);
    if (!checkpoint.success) {
        if (diagnostic) *diagnostic = checkpoint.code;
        capture_.reset();
        return false;
    }
    revision_ = 0;
    next_tick_ = 0;
    last_control_id_ = 0;
    last_result_ = {};
    return publish(diagnostic);
}

void PlaytestScenarioReplayRuntime::observeInput(const input::InputAction action,
                                                 const input::ActionState state) {
    if (!capture_ || replaying_ || state != input::ActionState::Pressed) return;
    if (const auto semantic = semanticInputFor(action)) pending_inputs_.push_back(*semantic);
}

bool PlaytestScenarioReplayRuntime::finishFrame(const nlohmann::json& deterministic_state,
                                                std::string* diagnostic) {
    if (!capture_ || replaying_ || pending_inputs_.empty()) return true;
    for (auto& input : pending_inputs_) {
        const auto recorded = capture_->recordInput(next_tick_++, input.kind, std::move(input.action),
                                                     nlohmann::json::object(), deterministic_state);
        if (!recorded.success) {
            pending_inputs_.clear();
            if (diagnostic) *diagnostic = recorded.code;
            return false;
        }
    }
    pending_inputs_.clear();
    return publish(diagnostic);
}

bool PlaytestScenarioReplayRuntime::poll(const ReplayHandler& handler, std::string* diagnostic) {
    if (!capture_) return true;
    const auto poll_result = bridge_.pollControls([&](const ScenarioReplayControl& control) {
        last_control_id_ = control.control_id;
        if (control.expected_revision != revision_) {
            last_result_ = {false, "replay_stale_revision",
                            "Replay request rejected because the capture revision changed.", control.mode,
                            0, std::nullopt};
            return false;
        }
        if (!handler) {
            last_result_ = {false, "replay_executor_missing", "The live runtime has no replay executor.", control.mode,
                            0, std::nullopt};
            return false;
        }
        replaying_ = true;
        last_result_ = handler(artifact(), control.mode);
        replaying_ = false;
        return true;
    });
    if (poll_result.io_error) {
        if (diagnostic) *diagnostic = poll_result.error;
        return false;
    }
    if (poll_result.processed > 0) return publish(diagnostic);
    return true;
}

replay::ReplayArtifact PlaytestScenarioReplayRuntime::artifact() const {
    return capture_ ? capture_->finish(session_id_ + "-reported-failure") : replay::ReplayArtifact{};
}

bool PlaytestScenarioReplayRuntime::publish(std::string* diagnostic) {
    ++revision_;
    return bridge_.publish({revision_, session_id_, true, artifact(), last_control_id_, last_result_}, diagnostic);
}

std::optional<PlaytestScenarioReplayRuntime::PendingInput>
PlaytestScenarioReplayRuntime::semanticInputFor(const input::InputAction action) {
    using input::InputAction;
    switch (action) {
    case InputAction::MoveUp: return PendingInput{replay::SemanticInputKind::Move, "move_up"};
    case InputAction::MoveDown: return PendingInput{replay::SemanticInputKind::Move, "move_down"};
    case InputAction::MoveLeft: return PendingInput{replay::SemanticInputKind::Move, "move_left"};
    case InputAction::MoveRight: return PendingInput{replay::SemanticInputKind::Move, "move_right"};
    case InputAction::Confirm: return PendingInput{replay::SemanticInputKind::Confirm, "confirm"};
    case InputAction::Cancel: return PendingInput{replay::SemanticInputKind::Cancel, "cancel"};
    case InputAction::Menu: return PendingInput{replay::SemanticInputKind::Menu, "menu"};
    case InputAction::PageLeft: return PendingInput{replay::SemanticInputKind::Menu, "page_left"};
    case InputAction::PageRight: return PendingInput{replay::SemanticInputKind::Menu, "page_right"};
    case InputAction::BattleAttack: return PendingInput{replay::SemanticInputKind::Confirm, "battle_attack"};
    case InputAction::BattleSkill: return PendingInput{replay::SemanticInputKind::Confirm, "battle_skill"};
    case InputAction::BattleItem: return PendingInput{replay::SemanticInputKind::Confirm, "battle_item"};
    case InputAction::BattleDefend: return PendingInput{replay::SemanticInputKind::Confirm, "battle_defend"};
    case InputAction::BattleEscape: return PendingInput{replay::SemanticInputKind::Cancel, "battle_escape"};
    case InputAction::Debug: return PendingInput{replay::SemanticInputKind::DebugCommand, "debug"};
    case InputAction::None: break;
    }
    return std::nullopt;
}

std::optional<input::InputAction> PlaytestScenarioReplayRuntime::inputActionFor(const std::string_view action) {
    using input::InputAction;
    if (action == "move_up") return InputAction::MoveUp;
    if (action == "move_down") return InputAction::MoveDown;
    if (action == "move_left") return InputAction::MoveLeft;
    if (action == "move_right") return InputAction::MoveRight;
    if (action == "confirm") return InputAction::Confirm;
    if (action == "cancel") return InputAction::Cancel;
    if (action == "menu") return InputAction::Menu;
    if (action == "page_left") return InputAction::PageLeft;
    if (action == "page_right") return InputAction::PageRight;
    if (action == "battle_attack") return InputAction::BattleAttack;
    if (action == "battle_skill") return InputAction::BattleSkill;
    if (action == "battle_item") return InputAction::BattleItem;
    if (action == "battle_defend") return InputAction::BattleDefend;
    if (action == "battle_escape") return InputAction::BattleEscape;
    if (action == "debug") return InputAction::Debug;
    return std::nullopt;
}

} // namespace urpg::playtest
