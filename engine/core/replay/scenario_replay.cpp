#include "engine/core/replay/scenario_replay.h"

#include <algorithm>
#include <cctype>
#include <utility>

namespace urpg::replay {

std::string_view semanticInputKindName(const SemanticInputKind kind) {
    switch (kind) {
    case SemanticInputKind::Move: return "move";
    case SemanticInputKind::Interact: return "interact";
    case SemanticInputKind::Menu: return "menu";
    case SemanticInputKind::Confirm: return "confirm";
    case SemanticInputKind::Cancel: return "cancel";
    case SemanticInputKind::DebugCommand: return "debug_command";
    }
    return "unknown";
}

std::string_view replayExecutionModeName(const ReplayExecutionMode mode) {
    return mode == ReplayExecutionMode::Headless ? "headless" : "interactive";
}

ScenarioReplayCapture::ScenarioReplayCapture(ScenarioReplayCaptureConfig config) : config_(std::move(config)) {
    config_.sensitive_fields.insert("password");
    config_.sensitive_fields.insert("token");
    config_.sensitive_fields.insert("secret");
    config_.sensitive_fields.insert("api_key");
    config_.sensitive_fields.insert("email");
    config_.sensitive_fields.insert("local_path");
}

ScenarioReplayCaptureResult ScenarioReplayCapture::recordInput(
    const int64_t tick, const SemanticInputKind kind, std::string action, nlohmann::json payload,
    const nlohmann::json& deterministic_state) {
    if (tick < 0 || (!inputs_.empty() && tick <= last_tick_) || action.empty()) {
        return {false, "replay_semantic_input_invalid", "Semantic input requires a strictly increasing tick and action."};
    }
    if (inputs_.size() >= config_.max_inputs) {
        return {false, "replay_input_limit_reached", "Replay input limit reached."};
    }
    inputs_.push_back({tick, std::move(action), redact(payload, "payload"),
                       std::string(semanticInputKindName(kind))});
    hashes_[tick] = ReplayRecorder::hashState(deterministic_state);
    last_tick_ = tick;
    return {true, "replay_semantic_input_recorded", "Semantic input recorded."};
}

ScenarioReplayCaptureResult ScenarioReplayCapture::captureCheckpoint(
    const int64_t tick, std::string label, const nlohmann::json& deterministic_state) {
    if (tick < 0 || label.empty()) {
        return {false, "replay_checkpoint_invalid", "Checkpoint requires a nonnegative tick and label."};
    }
    if (checkpoints_.size() >= config_.max_checkpoints) {
        return {false, "replay_checkpoint_limit_reached", "Replay checkpoint limit reached."};
    }
    checkpoints_.push_back({tick, std::move(label), ReplayRecorder::hashState(deterministic_state),
                            redact(deterministic_state, "checkpoint")});
    std::stable_sort(checkpoints_.begin(), checkpoints_.end(), [](const auto& left, const auto& right) {
        return std::tie(left.tick, left.label) < std::tie(right.tick, right.label);
    });
    return {true, "replay_checkpoint_recorded", "Replay checkpoint recorded."};
}

ReplayArtifact ScenarioReplayCapture::finish(std::string id) const {
    ReplayArtifact artifact;
    artifact.id = std::move(id);
    artifact.seed = config_.seed;
    artifact.project_version = config_.project_revision;
    artifact.project_revision = config_.project_revision;
    artifact.runtime_version = config_.runtime_version;
    artifact.input_log = inputs_;
    artifact.state_hashes = hashes_;
    artifact.checkpoints = checkpoints_;
    artifact.redacted_fields = redacted_fields_;
    artifact.labels = {"scenario_capture"};
    return artifact;
}

nlohmann::json ScenarioReplayCapture::redact(const nlohmann::json& value, const std::string path) {
    if (value.is_object()) {
        auto result = nlohmann::json::object();
        for (const auto& [key, child] : value.items()) {
            const auto child_path = path.empty() ? key : path + "." + key;
            if (isSensitive(key)) {
                result[key] = "[REDACTED]";
                redacted_fields_.insert(child_path);
            } else {
                result[key] = redact(child, child_path);
            }
        }
        return result;
    }
    if (value.is_array()) {
        auto result = nlohmann::json::array();
        for (size_t index = 0; index < value.size(); ++index) {
            result.push_back(redact(value[index], path + "[" + std::to_string(index) + "]"));
        }
        return result;
    }
    return value;
}

bool ScenarioReplayCapture::isSensitive(const std::string_view key) const {
    std::string normalized(key);
    std::transform(normalized.begin(), normalized.end(), normalized.begin(),
                   [](const unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return std::any_of(config_.sensitive_fields.begin(), config_.sensitive_fields.end(),
                       [&](const auto& sensitive) { return normalized == sensitive; });
}

ReplayExecutionResult ScenarioReplayRunner::run(const ReplayExecutionRequest& request,
                                                const SemanticInputExecutor& executor) {
    ReplayExecutionResult result;
    result.mode = request.mode;
    if (request.artifact.project_revision.empty() || request.artifact.runtime_version.empty() ||
        request.artifact.project_revision != request.project_revision ||
        request.artifact.runtime_version != request.runtime_version) {
        result.code = "replay_environment_mismatch";
        result.message = "Replay project revision or runtime version does not match the capture.";
        return result;
    }
    if (!executor) {
        result.code = "replay_executor_missing";
        result.message = "Replay has no semantic input executor.";
        return result;
    }
    for (const auto& input : request.artifact.input_log) {
        const auto state = executor(input, request.artifact.seed, request.mode);
        if (!state) {
            result.code = "replay_semantic_input_unsupported";
            result.message = "Replay stopped because a semantic input was not supported at tick " +
                             std::to_string(input.tick) + ".";
            return result;
        }
        ++result.applied_inputs;
        const auto expected = request.artifact.state_hashes.find(input.tick);
        const auto actual_hash = ReplayRecorder::hashState(*state);
        if (expected == request.artifact.state_hashes.end() || expected->second != actual_hash) {
            result.code = "replay_diverged";
            result.message = "Replay diverged at tick " + std::to_string(input.tick) + ".";
            result.divergence = ReplayComparison{false, input.tick,
                expected == request.artifact.state_hashes.end() ? std::string{} : expected->second, actual_hash};
            return result;
        }
    }
    result.success = true;
    result.code = "replay_completed";
    result.message = std::string("Replay completed in ") + std::string(replayExecutionModeName(request.mode)) + " mode.";
    return result;
}

} // namespace urpg::replay
