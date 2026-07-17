#include "engine/core/playtest/playtest_runtime_state_bridge.h"

#include <fstream>

namespace urpg::playtest {
namespace {

constexpr std::string_view kSnapshotSchema = "urpg.playtest_runtime_state_snapshot.v1";
constexpr std::string_view kControlSchema = "urpg.playtest_runtime_state_control.v1";
constexpr size_t kMaxMutations = 4096;

const char* actionName(const RuntimeStateControlAction action) {
    return action == RuntimeStateControlAction::TemporaryEdit ? "temporary_edit" : "reset_checkpoint";
}

std::optional<RuntimeStateControlAction> parseAction(const std::string& value) {
    if (value == "temporary_edit") return RuntimeStateControlAction::TemporaryEdit;
    if (value == "reset_checkpoint") return RuntimeStateControlAction::ResetCheckpoint;
    return std::nullopt;
}

bool atomicWrite(const std::filesystem::path& target, const nlohmann::json& value,
                 std::string* diagnostic) {
    std::error_code error;
    std::filesystem::create_directories(target.parent_path(), error);
    if (error) {
        if (diagnostic) *diagnostic = "runtime_state_directory_failed:" + error.message();
        return false;
    }
    auto temporary = target;
    temporary += ".tmp";
    std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
    if (!output) {
        if (diagnostic) *diagnostic = "runtime_state_temporary_open_failed";
        return false;
    }
    output << value.dump() << '\n';
    output.close();
    if (!output) {
        std::filesystem::remove(temporary, error);
        if (diagnostic) *diagnostic = "runtime_state_temporary_flush_failed";
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
            if (diagnostic) *diagnostic = "runtime_state_backup_failed:" + error.message();
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
        if (diagnostic) *diagnostic = "runtime_state_publish_failed:" + error.message();
        return false;
    }
    if (replacing) std::filesystem::remove(backup, error);
    return true;
}

bool validSnapshot(const LiveRuntimeStateSnapshot& snapshot) {
    return snapshot.revision > 0 && !snapshot.session_id.empty() && !snapshot.checkpoint_id.empty() &&
           snapshot.disposable && !snapshot.packaged && snapshot.state.is_object() &&
           snapshot.package_state.is_object() && snapshot.mutations.is_array() &&
           snapshot.mutations.size() <= kMaxMutations;
}

} // namespace

bool PlaytestRuntimeStateBridge::publish(const LiveRuntimeStateSnapshot& snapshot,
                                         std::string* diagnostic) const {
    if (!validSnapshot(snapshot)) {
        if (diagnostic) *diagnostic = "runtime_state_snapshot_invalid";
        return false;
    }
    return atomicWrite(session_directory_ / "runtime_state_snapshot.json",
                       {{"schema", kSnapshotSchema}, {"revision", snapshot.revision},
                        {"sessionId", snapshot.session_id}, {"checkpointId", snapshot.checkpoint_id},
                        {"disposable", true}, {"packaged", false}, {"state", snapshot.state},
                        {"packageState", snapshot.package_state}, {"mutations", snapshot.mutations},
                        {"lastControlId", snapshot.last_control_id},
                        {"lastControlCode", snapshot.last_control_code}},
                       diagnostic);
}

std::optional<LiveRuntimeStateSnapshot> PlaytestRuntimeStateBridge::readAfter(
    const uint64_t revision, std::string* diagnostic) const {
    std::ifstream input(session_directory_ / "runtime_state_snapshot.json", std::ios::binary);
    if (!input) return std::nullopt;
    const auto value = nlohmann::json::parse(input, nullptr, false);
    if (!value.is_object() || value.value("schema", "") != kSnapshotSchema) {
        if (diagnostic) *diagnostic = "runtime_state_snapshot_malformed";
        return std::nullopt;
    }
    LiveRuntimeStateSnapshot snapshot;
    snapshot.revision = value.value("revision", uint64_t{0});
    snapshot.session_id = value.value("sessionId", "");
    snapshot.checkpoint_id = value.value("checkpointId", "");
    snapshot.disposable = value.value("disposable", false);
    snapshot.packaged = value.value("packaged", true);
    snapshot.state = value.value("state", nlohmann::json::object());
    snapshot.package_state = value.value("packageState", nlohmann::json::object());
    snapshot.mutations = value.value("mutations", nlohmann::json::array());
    snapshot.last_control_id = value.value("lastControlId", uint64_t{0});
    snapshot.last_control_code = value.value("lastControlCode", "");
    if (!validSnapshot(snapshot)) {
        if (diagnostic) *diagnostic = "runtime_state_snapshot_invalid";
        return std::nullopt;
    }
    if (snapshot.revision <= revision) return std::nullopt;
    return snapshot;
}

bool PlaytestRuntimeStateBridge::appendControl(const RuntimeStateControl& control,
                                               std::string* diagnostic) const {
    if (control.control_id == 0 || control.expected_revision == 0 ||
        (control.action == RuntimeStateControlAction::TemporaryEdit &&
         (control.mutation_id.empty() || control.kind.empty() || control.id.empty())) ||
        (control.action == RuntimeStateControlAction::ResetCheckpoint && control.checkpoint_id.empty())) {
        if (diagnostic) *diagnostic = "runtime_state_control_invalid";
        return false;
    }
    std::error_code error;
    std::filesystem::create_directories(session_directory_, error);
    if (error) {
        if (diagnostic) *diagnostic = "runtime_state_control_directory_failed:" + error.message();
        return false;
    }
    std::ofstream output(session_directory_ / "runtime_state_controls.jsonl", std::ios::binary | std::ios::app);
    if (!output) {
        if (diagnostic) *diagnostic = "runtime_state_control_open_failed";
        return false;
    }
    output << nlohmann::json{{"schema", kControlSchema}, {"controlId", control.control_id},
                             {"expectedRevision", control.expected_revision},
                             {"action", actionName(control.action)}, {"mutationId", control.mutation_id},
                             {"kind", control.kind}, {"id", control.id}, {"field", control.field},
                             {"value", control.value}, {"checkpointId", control.checkpoint_id}}.dump() << '\n';
    return static_cast<bool>(output);
}

RuntimeStateControlPollResult PlaytestRuntimeStateBridge::pollControls(
    const ControlHandler& handler, const size_t max_controls) {
    RuntimeStateControlPollResult result;
    if (!handler || max_controls == 0) return result;
    std::ifstream input(session_directory_ / "runtime_state_controls.jsonl", std::ios::binary);
    if (!input) return result;
    input.seekg(static_cast<std::streamoff>(consumed_control_bytes_));
    std::string line;
    while (result.processed < max_controls && std::getline(input, line)) {
        consumed_control_bytes_ += line.size() + 1;
        const auto value = nlohmann::json::parse(line, nullptr, false);
        const auto action = value.is_object() && value.value("schema", "") == kControlSchema
                                ? parseAction(value.value("action", "")) : std::nullopt;
        RuntimeStateControl control;
        if (action) {
            control.control_id = value.value("controlId", uint64_t{0});
            control.expected_revision = value.value("expectedRevision", uint64_t{0});
            control.action = *action;
            control.mutation_id = value.value("mutationId", "");
            control.kind = value.value("kind", "");
            control.id = value.value("id", "");
            control.field = value.value("field", "");
            control.value = value.value("value", nlohmann::json{});
            control.checkpoint_id = value.value("checkpointId", "");
        }
        ++result.processed;
        const bool valid = action && control.control_id > 0 && control.expected_revision > 0 &&
            ((control.action == RuntimeStateControlAction::TemporaryEdit && !control.mutation_id.empty() &&
              !control.kind.empty() && !control.id.empty()) ||
             (control.action == RuntimeStateControlAction::ResetCheckpoint && !control.checkpoint_id.empty()));
        if (valid && handler(control)) ++result.applied;
        else ++result.rejected;
    }
    if (input.bad()) {
        result.io_error = true;
        result.error = "runtime_state_control_read_failed";
    }
    return result;
}

} // namespace urpg::playtest
