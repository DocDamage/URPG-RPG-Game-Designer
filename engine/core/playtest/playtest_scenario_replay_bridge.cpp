#include "engine/core/playtest/playtest_scenario_replay_bridge.h"

#include <fstream>

namespace urpg::playtest {
namespace {

constexpr std::string_view kSnapshotSchema = "urpg.playtest_scenario_replay_snapshot.v1";
constexpr std::string_view kControlSchema = "urpg.playtest_scenario_replay_control.v1";

bool atomicWrite(const std::filesystem::path& target, const nlohmann::json& value,
                 std::string* diagnostic) {
    std::error_code error;
    std::filesystem::create_directories(target.parent_path(), error);
    if (error) {
        if (diagnostic) *diagnostic = "scenario_replay_directory_failed:" + error.message();
        return false;
    }
    auto temporary = target;
    temporary += ".tmp";
    std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
    if (!output) {
        if (diagnostic) *diagnostic = "scenario_replay_temporary_open_failed";
        return false;
    }
    output << value.dump() << '\n';
    output.close();
    if (!output) {
        std::filesystem::remove(temporary, error);
        if (diagnostic) *diagnostic = "scenario_replay_temporary_flush_failed";
        return false;
    }
    auto backup = target;
    backup += ".bak";
    const bool replacing = std::filesystem::exists(target);
    if (replacing) {
        std::filesystem::remove(backup, error);
        error.clear();
        std::filesystem::rename(target, backup, error);
        if (error) {
            std::filesystem::remove(temporary, error);
            if (diagnostic) *diagnostic = "scenario_replay_backup_failed:" + error.message();
            return false;
        }
    }
    std::filesystem::rename(temporary, target, error);
    if (error) {
        std::filesystem::remove(temporary, error);
        if (replacing) {
            std::error_code restore_error;
            std::filesystem::rename(backup, target, restore_error);
        }
        if (diagnostic) *diagnostic = "scenario_replay_publish_failed:" + error.message();
        return false;
    }
    if (replacing) std::filesystem::remove(backup, error);
    return true;
}

nlohmann::json resultToJson(const replay::ReplayExecutionResult& result) {
    nlohmann::json value{{"success", result.success}, {"code", result.code}, {"message", result.message},
                         {"mode", replay::replayExecutionModeName(result.mode)},
                         {"appliedInputs", result.applied_inputs}};
    if (result.divergence) {
        value["divergence"] = {{"tick", result.divergence->first_mismatched_tick},
                               {"expectedHash", result.divergence->expected_hash},
                               {"actualHash", result.divergence->actual_hash}};
    }
    return value;
}

replay::ReplayExecutionResult resultFromJson(const nlohmann::json& value) {
    replay::ReplayExecutionResult result;
    result.success = value.value("success", false);
    result.code = value.value("code", "");
    result.message = value.value("message", "");
    result.mode = value.value("mode", "headless") == "interactive"
                      ? replay::ReplayExecutionMode::Interactive : replay::ReplayExecutionMode::Headless;
    result.applied_inputs = value.value("appliedInputs", size_t{0});
    if (const auto divergence = value.find("divergence"); divergence != value.end() && divergence->is_object()) {
        result.divergence = replay::ReplayComparison{
            false, divergence->value("tick", int64_t{-1}), divergence->value("expectedHash", ""),
            divergence->value("actualHash", "")};
    }
    return result;
}

} // namespace

bool PlaytestScenarioReplayBridge::publish(const LiveScenarioReplaySnapshot& snapshot,
                                           std::string* diagnostic) const {
    if (snapshot.revision == 0 || snapshot.session_id.empty() || snapshot.artifact.id.empty()) {
        if (diagnostic) *diagnostic = "scenario_replay_snapshot_invalid";
        return false;
    }
    const auto artifact = snapshot.artifact.toJson();
    if (!atomicWrite(session_directory_ / "replay.json", artifact, diagnostic)) return false;
    return atomicWrite(session_directory_ / "scenario_replay_snapshot.json",
                       {{"schema", kSnapshotSchema}, {"revision", snapshot.revision},
                        {"sessionId", snapshot.session_id}, {"capturing", snapshot.capturing},
                        {"artifact", artifact}, {"lastControlId", snapshot.last_control_id},
                        {"lastResult", resultToJson(snapshot.last_result)}}, diagnostic);
}

std::optional<LiveScenarioReplaySnapshot> PlaytestScenarioReplayBridge::readAfter(
    const uint64_t revision, std::string* diagnostic) const {
    std::ifstream input(session_directory_ / "scenario_replay_snapshot.json", std::ios::binary);
    if (!input) return std::nullopt;
    const auto value = nlohmann::json::parse(input, nullptr, false);
    if (!value.is_object() || value.value("schema", "") != kSnapshotSchema ||
        !value.value("artifact", nlohmann::json{}).is_object()) {
        if (diagnostic) *diagnostic = "scenario_replay_snapshot_malformed";
        return std::nullopt;
    }
    LiveScenarioReplaySnapshot snapshot;
    snapshot.revision = value.value("revision", uint64_t{0});
    snapshot.session_id = value.value("sessionId", "");
    snapshot.capturing = value.value("capturing", false);
    snapshot.artifact = replay::ReplayArtifact::fromJson(value.at("artifact"));
    snapshot.last_control_id = value.value("lastControlId", uint64_t{0});
    snapshot.last_result = resultFromJson(value.value("lastResult", nlohmann::json::object()));
    if (snapshot.revision == 0 || snapshot.session_id.empty() || snapshot.artifact.id.empty()) {
        if (diagnostic) *diagnostic = "scenario_replay_snapshot_invalid";
        return std::nullopt;
    }
    if (snapshot.revision <= revision) return std::nullopt;
    return snapshot;
}

bool PlaytestScenarioReplayBridge::appendControl(const ScenarioReplayControl& control,
                                                 std::string* diagnostic) const {
    if (control.control_id == 0 || control.expected_revision == 0) {
        if (diagnostic) *diagnostic = "scenario_replay_control_invalid";
        return false;
    }
    std::error_code error;
    std::filesystem::create_directories(session_directory_, error);
    if (error) {
        if (diagnostic) *diagnostic = "scenario_replay_control_directory_failed:" + error.message();
        return false;
    }
    std::ofstream output(session_directory_ / "scenario_replay_controls.jsonl", std::ios::binary | std::ios::app);
    if (!output) {
        if (diagnostic) *diagnostic = "scenario_replay_control_open_failed";
        return false;
    }
    output << nlohmann::json{{"schema", kControlSchema}, {"controlId", control.control_id},
                             {"expectedRevision", control.expected_revision},
                             {"mode", replay::replayExecutionModeName(control.mode)}}.dump() << '\n';
    return static_cast<bool>(output);
}

ScenarioReplayControlPollResult PlaytestScenarioReplayBridge::pollControls(
    const ControlHandler& handler, const size_t max_controls) {
    ScenarioReplayControlPollResult result;
    if (!handler || max_controls == 0) return result;
    std::ifstream input(session_directory_ / "scenario_replay_controls.jsonl", std::ios::binary);
    if (!input) return result;
    input.seekg(static_cast<std::streamoff>(consumed_control_bytes_));
    std::string line;
    while (result.processed < max_controls && std::getline(input, line)) {
        consumed_control_bytes_ += line.size() + 1;
        const auto value = nlohmann::json::parse(line, nullptr, false);
        ScenarioReplayControl control;
        const bool valid = value.is_object() && value.value("schema", "") == kControlSchema &&
                           value.value("controlId", uint64_t{0}) > 0 &&
                           value.value("expectedRevision", uint64_t{0}) > 0 &&
                           (value.value("mode", "") == "headless" || value.value("mode", "") == "interactive");
        if (valid) {
            control.control_id = value.value("controlId", uint64_t{0});
            control.expected_revision = value.value("expectedRevision", uint64_t{0});
            control.mode = value.value("mode", "") == "interactive"
                               ? replay::ReplayExecutionMode::Interactive : replay::ReplayExecutionMode::Headless;
        }
        ++result.processed;
        if (valid && handler(control)) ++result.applied;
        else ++result.rejected;
    }
    if (input.bad()) {
        result.io_error = true;
        result.error = "scenario_replay_control_read_failed";
    }
    return result;
}

} // namespace urpg::playtest
