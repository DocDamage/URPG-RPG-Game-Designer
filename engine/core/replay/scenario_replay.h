#pragma once

#include "engine/core/replay/replay_player.h"

#include <functional>
#include <optional>
#include <string_view>

namespace urpg::replay {

enum class SemanticInputKind : uint8_t {
    Move,
    Interact,
    Menu,
    Confirm,
    Cancel,
    DebugCommand
};

enum class ReplayExecutionMode : uint8_t {
    Headless,
    Interactive
};

std::string_view semanticInputKindName(SemanticInputKind kind);
std::string_view replayExecutionModeName(ReplayExecutionMode mode);

struct ScenarioReplayCaptureConfig {
    uint64_t seed = 0;
    std::string project_revision;
    std::string runtime_version;
    std::set<std::string> sensitive_fields;
    size_t max_inputs = 100000;
    size_t max_checkpoints = 64;
};

struct ScenarioReplayCaptureResult {
    bool success = false;
    std::string code;
    std::string message;
};

class ScenarioReplayCapture {
public:
    explicit ScenarioReplayCapture(ScenarioReplayCaptureConfig config);

    ScenarioReplayCaptureResult recordInput(int64_t tick, SemanticInputKind kind, std::string action,
                                            nlohmann::json payload, const nlohmann::json& deterministic_state);
    ScenarioReplayCaptureResult captureCheckpoint(int64_t tick, std::string label,
                                                   const nlohmann::json& deterministic_state);
    ReplayArtifact finish(std::string id) const;

private:
    nlohmann::json redact(const nlohmann::json& value, std::string path);
    bool isSensitive(std::string_view key) const;

    ScenarioReplayCaptureConfig config_;
    std::vector<ReplayInput> inputs_;
    std::map<int64_t, std::string> hashes_;
    std::vector<ReplayCheckpoint> checkpoints_;
    std::set<std::string> redacted_fields_;
    int64_t last_tick_ = -1;
};

struct ReplayExecutionRequest {
    ReplayArtifact artifact;
    ReplayExecutionMode mode = ReplayExecutionMode::Headless;
    std::string project_revision;
    std::string runtime_version;
};

struct ReplayExecutionResult {
    bool success = false;
    std::string code;
    std::string message;
    ReplayExecutionMode mode = ReplayExecutionMode::Headless;
    size_t applied_inputs = 0;
    std::optional<ReplayComparison> divergence;
};

using SemanticInputExecutor =
    std::function<std::optional<nlohmann::json>(const ReplayInput&, uint64_t, ReplayExecutionMode)>;

class ScenarioReplayRunner {
public:
    static ReplayExecutionResult run(const ReplayExecutionRequest& request, const SemanticInputExecutor& executor);
};

} // namespace urpg::replay
