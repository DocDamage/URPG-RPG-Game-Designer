#include "editor/playtest/playtest_session_controller.h"

#include "engine/core/save/save_journal.h"
#include "engine/core/security/sha256.h"
#include "engine/core/version.h"

#include <chrono>
#include <fstream>
#include <iterator>
#include <nlohmann/json.hpp>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace urpg::editor {
namespace {

std::filesystem::path currentExecutableDirectory() {
#ifdef _WIN32
    std::wstring buffer(MAX_PATH, L'\0');
    const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (length == 0 || length == buffer.size()) return {};
    buffer.resize(length);
    return std::filesystem::path(buffer).parent_path();
#else
    return {};
#endif
}

bool isSafeMapId(const std::string& map_id) {
    const std::filesystem::path mapPath(map_id);
    return !map_id.empty() && !mapPath.is_absolute() && mapPath.filename().string() == map_id && map_id != "." &&
           map_id != "..";
}

const char* severityName(const diagnostics::DiagnosticSeverity severity) {
    switch (severity) {
    case diagnostics::DiagnosticSeverity::Info: return "info";
    case diagnostics::DiagnosticSeverity::Warning: return "warning";
    case diagnostics::DiagnosticSeverity::Error: return "error";
    case diagnostics::DiagnosticSeverity::Fatal: return "fatal";
    }
    return "unknown";
}

} // namespace

PlaytestSessionController::PlaytestSessionController(std::filesystem::path runtime_executable)
    : runtime_executable_(std::move(runtime_executable)) {}

PlaytestSessionController::~PlaytestSessionController() {
    returnToEditor();
}

std::filesystem::path PlaytestSessionController::resolveRuntimeExecutable() const {
    if (!runtime_executable_.empty()) return runtime_executable_;
    const auto adjacent = currentExecutableDirectory() /
#ifdef _WIN32
                          "urpg_runtime.exe";
#else
                          "urpg_runtime";
#endif
    return adjacent;
}

bool PlaytestSessionController::start(const std::filesystem::path& project_root,
                                      const std::string& map_id,
                                      const std::string& spawn,
                                      const std::string& grid_draft,
                                      const std::string& perspective_2d_draft,
                                      const std::string& selected_object_id) {
    returnToEditor();
    message_.clear();
    exit_code_ = 0;
    map_id_.clear();
    spawn_.clear();
    selected_object_id_.clear();
    checkpoint_id_.clear();
    started_at_ = {};
    diagnostics_.clear();
    diagnostics_offset_ = 0;
    next_command_id_ = 1;
    map_reload_revision_ = 0;
    pending_map_reload_command_id_.reset();
    acknowledgement_offset_ = 0;
    if (!std::filesystem::is_directory(project_root) || !isSafeMapId(map_id)) {
        state_ = PlaytestSessionState::Crashed;
        message_ = "Choose a project and a valid map before starting playtest.";
        return false;
    }
    if (grid_draft.empty() && perspective_2d_draft.empty()) {
        state_ = PlaytestSessionState::Crashed;
        message_ = "There is no current map draft to playtest.";
        return false;
    }

    const auto nonce = std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count();
    session_directory_ = project_root / ".urpg" / "playtest" / ("session_" + std::to_string(nonce));
    cleanStaleOverlays(project_root);
    std::error_code filesystemError;
    std::filesystem::create_directories(session_directory_ / "content" / "maps", filesystemError);
    if (filesystemError) {
        state_ = PlaytestSessionState::Crashed;
        message_ = "Could not prepare the private playtest overlay: " + filesystemError.message();
        return false;
    }

    std::string writeError;
    const auto overlayMaps = session_directory_ / "content" / "maps";
    if ((!grid_draft.empty() && !SaveJournal::WriteAtomically(overlayMaps / (map_id + ".grid.json"), grid_draft, &writeError)) ||
        (!perspective_2d_draft.empty() &&
         !SaveJournal::WriteAtomically(overlayMaps / (map_id + ".p2d.json"), perspective_2d_draft, &writeError))) {
        state_ = PlaytestSessionState::Crashed;
        message_ = "Could not write the current map playtest overlay: " + writeError;
        return false;
    }
    const auto manifestPath = session_directory_ / "session.json";
    checkpoint_id_ = "launch";
    const std::string revision_material = project_root.generic_string() + "\n" + map_id + "\n" + spawn + "\n" +
                                          grid_draft + "\n" + perspective_2d_draft;
    const std::vector<uint8_t> revision_bytes(revision_material.begin(), revision_material.end());
    const auto project_revision = security::Sha256::toHex(security::Sha256::compute(revision_bytes));
    const nlohmann::json manifest = {{"schema", "urpg.playtest_session.v1"},
                                     {"project_root", project_root.generic_string()},
                                     {"overlay_dir", session_directory_.generic_string()},
                                     {"map_id", map_id},
                                     {"spawn", spawn},
                                     {"selected_object_id", selected_object_id},
                                     {"project_revision", project_revision},
                                     {"replay_seed", static_cast<uint64_t>(nonce)},
                                     {"checkpoint", {{"id", checkpoint_id_}, {"disposable_overlay", true}}},
                                     {"diagnostics_path", (session_directory_ / "diagnostics.jsonl").generic_string()}};
    if (!SaveJournal::WriteAtomically(manifestPath, manifest.dump(2) + "\n", &writeError)) {
        state_ = PlaytestSessionState::Crashed;
        message_ = "Could not write the playtest session manifest: " + writeError;
        return false;
    }

    const auto runtime = resolveRuntimeExecutable();
    if (!std::filesystem::is_regular_file(runtime)) {
        state_ = PlaytestSessionState::Crashed;
        message_ = "URPG Runtime was not found next to the editor.";
        return false;
    }
    platform::ProcessCommand command;
    command.executable = runtime;
    command.workingDirectory = project_root;
    command.arguments = {"--project-root", project_root.string(), "--map", map_id, "--spawn", spawn,
                         "--session-manifest", manifestPath.string()};
    if (!process_.launch(command)) {
        state_ = PlaytestSessionState::Crashed;
        message_ = "Could not launch URPG Runtime: " + process_.error();
        return false;
    }
    map_id_ = map_id;
    spawn_ = spawn;
    selected_object_id_ = selected_object_id;
    started_at_ = std::chrono::steady_clock::now();
    state_ = PlaytestSessionState::Starting;
    message_ = "Playtest started with the current unsaved map overlay.";
    return true;
}

bool PlaytestSessionController::startFromHere(const std::filesystem::path& project_root, const std::string& map_id,
                                              const int32_t tile_x, const int32_t tile_y,
                                              const std::string& grid_draft,
                                              const std::string& perspective_2d_draft,
                                              const std::string& selected_object_id) {
    if (tile_x < 0 || tile_y < 0) return false;
    return start(project_root, map_id, std::to_string(tile_x) + "," + std::to_string(tile_y), grid_draft,
                 perspective_2d_draft, selected_object_id);
}

bool PlaytestSessionController::teleportHere(const std::string& map_id, const int32_t tile_x, const int32_t tile_y,
                                             const std::string& selected_object_id) {
    if (!isActive() || !isSafeMapId(map_id) || tile_x < 0 || tile_y < 0 || session_directory_.empty()) return false;
    std::ofstream output(session_directory_ / "editor_commands.jsonl", std::ios::app);
    if (!output) return false;
    output << nlohmann::json{{"version",1},{"command_id",next_command_id_},{"command","teleport_here"},{"map_id",map_id},
                             {"tile_x",tile_x},{"tile_y",tile_y},{"selected_object_id",selected_object_id}}.dump() << '\n';
    if (!output) return false;
    ++next_command_id_;
    map_id_ = map_id; spawn_ = std::to_string(tile_x) + "," + std::to_string(tile_y);
    selected_object_id_ = selected_object_id; checkpoint_id_ = "teleport";
    message_ = "Playtest teleport was queued for the selected Map cell.";
    return true;
}

bool PlaytestSessionController::hotReloadCurrentMap(const std::string& grid_draft,
                                                    const std::string& perspective_2d_draft) {
    if (!isActive() || pending_map_reload_command_id_.has_value() || session_directory_.empty() ||
        !isSafeMapId(map_id_) ||
        (grid_draft.empty() && perspective_2d_draft.empty()) ||
        grid_draft.size() > 4U * 1024U * 1024U || perspective_2d_draft.size() > 4U * 1024U * 1024U) return false;
    const auto overlay_maps = session_directory_ / "content" / "maps";
    std::string error;
    if ((!grid_draft.empty() &&
         !SaveJournal::WriteAtomically(overlay_maps / (map_id_ + ".grid.json"), grid_draft, &error)) ||
        (!perspective_2d_draft.empty() &&
         !SaveJournal::WriteAtomically(overlay_maps / (map_id_ + ".p2d.json"), perspective_2d_draft, &error))) {
        message_ = "Playtest map reload staging failed: " + error;
        return false;
    }
    std::ofstream output(session_directory_ / "editor_commands.jsonl", std::ios::app | std::ios::binary);
    if (!output) return false;
    const auto command_id = next_command_id_;
    output << nlohmann::json{{"version", 1}, {"command_id", command_id}, {"command", "hot_reload_map"},
                             {"map_id", map_id_}, {"expected_revision", map_reload_revision_},
                             {"state_policy", "reset_affected"}, {"selected_object_id", selected_object_id_}}.dump()
           << '\n';
    if (!output) return false;
    ++next_command_id_;
    pending_map_reload_command_id_ = command_id;
    message_ = "Bounded Map hot reload was queued with reset-affected state policy.";
    return true;
}

void PlaytestSessionController::returnToDiagnostic(const diagnostics::RuntimeDiagnostic& diagnostic,
                                                   const int32_t tile_x, const int32_t tile_y) {
    if (!diagnostic.map_id.empty() && isSafeMapId(diagnostic.map_id)) map_id_ = diagnostic.map_id;
    if (tile_x >= 0 && tile_y >= 0) spawn_ = std::to_string(tile_x) + "," + std::to_string(tile_y);
    selected_object_id_ = diagnostic.object_id;
    checkpoint_id_ = "diagnostic:" + diagnostic.code;
    returnToEditor();
}

std::chrono::seconds PlaytestSessionController::elapsed() const {
    if (started_at_ == std::chrono::steady_clock::time_point{}) return {};
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - started_at_);
}

PlaytestSupportBundleResult PlaytestSessionController::writeRedactedSupportBundle() const {
    return writeApprovedSupportBundle(previewRedactedSupportBundle(), true);
}

diagnostics::RedactedSupportBundlePreview PlaytestSessionController::previewRedactedSupportBundle() const {
    diagnostics::RedactedSupportBundleInput input;
    input.logs = {capturedStdout(), capturedStderr()};
    input.diagnostics = nlohmann::json::array();
    for (const auto& diagnostic : diagnostics_) {
        input.diagnostics.push_back({{"severity", severityName(diagnostic.severity)},
                                     {"subsystem", diagnostic.subsystem},
                                     {"code", diagnostic.code},
                                     {"message", diagnostic.message},
                                     {"map_id", diagnostic.map_id},
                                     {"object_id", diagnostic.object_id},
                                     {"source_path", diagnostic.source_file}});
    }
    input.versions = {{"editor_runtime_contract", urpg::versionString()}, {"support_schema", 1}};
    input.platform_capabilities = {{"os", "windows"},
                                   {"runtime_child_process", true},
                                   {"disposable_overlay", true},
                                   {"automatic_upload", false}};
    const auto manifest_path = session_directory_ / "session.json";
    std::ifstream manifest_input(manifest_path, std::ios::binary);
    if (manifest_input) {
        const std::string contents{std::istreambuf_iterator<char>(manifest_input),
                                   std::istreambuf_iterator<char>()};
        const std::vector<uint8_t> bytes(contents.begin(), contents.end());
        input.project_manifest_hashes["playtest_session_manifest"] =
            security::Sha256::toHex(security::Sha256::compute(bytes));
    }
    const auto replay_path = session_directory_ / "replay.json";
    std::ifstream replay_input(replay_path, std::ios::binary);
    if (replay_input) input.replay = nlohmann::json::parse(replay_input, nullptr, false);
    input.include_replay = input.replay.is_object();
    input.include_selected_project_data = false;
    return diagnostics::RedactedSupportBundleBuilder{}.preview(input);
}

PlaytestSupportBundleResult PlaytestSessionController::writeApprovedSupportBundle(
    const diagnostics::RedactedSupportBundlePreview& preview, const bool approved) const {
    PlaytestSupportBundleResult result;
    if (session_directory_.empty() || !std::filesystem::is_directory(session_directory_)) {
        result.message = "Start a playtest session before writing a redacted support bundle.";
        return result;
    }
    const auto written = diagnostics::RedactedSupportBundleBuilder{}.writeApproved(
        preview, session_directory_ / "support", approved);
    if (!written.success) {
        result.message = written.message;
        return result;
    }
    result.success = true;
    result.path = written.path;
    result.diagnostic_count = diagnostics_.size();
    result.message = written.message;
    return result;
}

void PlaytestSessionController::update() {
    if (!isActive()) return;
    pollDiagnostics();
    pollCommandAcknowledgements();
    if (process_.isRunning(&exit_code_)) {
        state_ = PlaytestSessionState::Running;
        return;
    }
    pollDiagnostics();
    pollCommandAcknowledgements();
    state_ = exit_code_ == 0 ? PlaytestSessionState::Exited : PlaytestSessionState::Crashed;
    message_ = exit_code_ == 0 ? "Playtest runtime exited; the editor remains open." :
                                 "Playtest runtime exited with code " + std::to_string(exit_code_) + ".";
}

void PlaytestSessionController::pollCommandAcknowledgements() {
    const auto path = session_directory_ / "runtime_acknowledgements.jsonl";
    std::error_code error;
    if (!pending_map_reload_command_id_.has_value() || !std::filesystem::is_regular_file(path, error) || error) return;
    const auto file_size = std::filesystem::file_size(path, error);
    if (error) return;
    if (acknowledgement_offset_ < 0 || static_cast<uintmax_t>(acknowledgement_offset_) > file_size) {
        acknowledgement_offset_ = 0;
    }
    std::ifstream input(path, std::ios::binary);
    if (!input) return;
    input.seekg(acknowledgement_offset_);
    const std::string chunk{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
    std::size_t consumed = 0;
    std::size_t processed = 0;
    while (processed < 64) {
        const auto newline = chunk.find('\n', consumed);
        if (newline == std::string::npos) break;
        const auto json = nlohmann::json::parse(chunk.substr(consumed, newline - consumed), nullptr, false);
        consumed = newline + 1;
        ++processed;
        if (!json.is_object() || json.value("version", 0) != 1 ||
            json.value("command", "") != "hot_reload_map" ||
            json.value("command_id", uint64_t{0}) != *pending_map_reload_command_id_) continue;
        if (json.value("status", "") == "applied" &&
            json.value("expected_revision", uint64_t{0}) == map_reload_revision_) {
            ++map_reload_revision_;
            checkpoint_id_ = "hot_reload:" + std::to_string(map_reload_revision_);
            message_ = "Map hot reload was acknowledged; affected map state was reset.";
        } else {
            message_ = "Map hot reload was rejected; the prior runtime state remains active.";
        }
        pending_map_reload_command_id_.reset();
    }
    acknowledgement_offset_ += static_cast<std::streamoff>(consumed);
}

void PlaytestSessionController::pollDiagnostics() {
    const auto path = session_directory_ / "diagnostics.jsonl";
    std::error_code error;
    if (!std::filesystem::is_regular_file(path, error) || error) return;
    const auto fileSize = std::filesystem::file_size(path, error);
    if (error) return;
    if (diagnostics_offset_ < 0 || static_cast<uintmax_t>(diagnostics_offset_) > fileSize) diagnostics_offset_ = 0;
    std::ifstream input(path, std::ios::binary);
    if (!input) return;
    input.seekg(diagnostics_offset_);
    const std::string chunk{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
    size_t consumed = 0;
    while (true) {
        const auto newline = chunk.find('\n', consumed);
        if (newline == std::string::npos) break;
        const auto json = nlohmann::json::parse(chunk.substr(consumed, newline - consumed), nullptr, false);
        consumed = newline + 1;
        if (!json.is_object() || json.value("version", 0) != 1) continue;
        diagnostics::RuntimeDiagnostic diagnostic;
        const auto severity = json.value("severity", "info");
        if (severity == "warning") diagnostic.severity = diagnostics::DiagnosticSeverity::Warning;
        else if (severity == "error") diagnostic.severity = diagnostics::DiagnosticSeverity::Error;
        else if (severity == "fatal") diagnostic.severity = diagnostics::DiagnosticSeverity::Fatal;
        diagnostic.subsystem = json.value("subsystem", "");
        diagnostic.code = json.value("code", "");
        diagnostic.message = json.value("message", "");
        diagnostic.map_id = json.value("map_id", "");
        diagnostic.object_id = json.value("object_id", "");
        diagnostic.source_file = json.value("source_file", "");
        diagnostic.timestamp = json.value("timestamp", 0LL);
        if (!diagnostic.code.empty()) {
            diagnostics_.push_back(std::move(diagnostic));
            if (diagnostics_.size() > diagnostics::RuntimeDiagnostics::kMaxRetainedEntries) {
                diagnostics_.erase(diagnostics_.begin());
            }
        }
    }
    diagnostics_offset_ += static_cast<std::streamoff>(consumed);
}

void PlaytestSessionController::returnToEditor() {
    if (isActive()) {
        state_ = PlaytestSessionState::Stopping;
        process_.terminate();
        exit_code_ = -1;
        message_ = "Playtest stopped and returned to the editor.";
        state_ = PlaytestSessionState::Returned;
    }
    if (!session_directory_.empty() && !map_id_.empty()) {
        last_return_context_ = {true, map_id_, spawn_, selected_object_id_, checkpoint_id_};
    }
}

void PlaytestSessionController::cleanStaleOverlays(const std::filesystem::path& project_root) const {
    const auto root = project_root / ".urpg" / "playtest";
    std::error_code error;
    if (!std::filesystem::is_directory(root, error) || error) return;
    const auto staleBefore = std::filesystem::file_time_type::clock::now() - std::chrono::hours(24 * 7);
    for (const auto& entry : std::filesystem::directory_iterator(root, error)) {
        if (error) return;
        if (!entry.is_directory(error) || error || entry.is_symlink(error) || entry.path() == session_directory_) {
            error.clear();
            continue;
        }
        const auto modified = entry.last_write_time(error);
        if (!error && modified < staleBefore) {
            std::filesystem::remove_all(entry.path(), error);
        }
        error.clear();
    }
}

} // namespace urpg::editor
