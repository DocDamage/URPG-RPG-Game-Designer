#pragma once

#include "engine/core/input/input_core.h"
#include "engine/core/playtest/playtest_scenario_replay_bridge.h"

#include <optional>
#include <vector>

namespace urpg::playtest {

class PlaytestScenarioReplayRuntime {
public:
    using ReplayHandler = std::function<replay::ReplayExecutionResult(
        const replay::ReplayArtifact&, replay::ReplayExecutionMode)>;

    PlaytestScenarioReplayRuntime(std::filesystem::path session_directory,
                                  replay::ScenarioReplayCaptureConfig config)
        : bridge_(std::move(session_directory)), config_(std::move(config)) {}

    bool start(std::string session_id, const nlohmann::json& initial_state,
               std::string* diagnostic = nullptr);
    void observeInput(input::InputAction action, input::ActionState state);
    bool finishFrame(const nlohmann::json& deterministic_state, std::string* diagnostic = nullptr);
    bool poll(const ReplayHandler& handler, std::string* diagnostic = nullptr);

    bool replaying() const { return replaying_; }
    uint64_t revision() const { return revision_; }
    replay::ReplayArtifact artifact() const;

    static std::optional<input::InputAction> inputActionFor(std::string_view action);

private:
    struct PendingInput {
        replay::SemanticInputKind kind = replay::SemanticInputKind::Confirm;
        std::string action;
    };

    bool publish(std::string* diagnostic);
    static std::optional<PendingInput> semanticInputFor(input::InputAction action);

    PlaytestScenarioReplayBridge bridge_;
    replay::ScenarioReplayCaptureConfig config_;
    std::optional<replay::ScenarioReplayCapture> capture_;
    std::vector<PendingInput> pending_inputs_;
    std::string session_id_;
    uint64_t revision_ = 0;
    uint64_t last_control_id_ = 0;
    int64_t next_tick_ = 0;
    replay::ReplayExecutionResult last_result_;
    bool replaying_ = false;
};

} // namespace urpg::playtest
