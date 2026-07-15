#include "editor/playtest/playtest_session_controller.h"

#include "engine/core/save/save_journal.h"

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
                                      const std::string& perspective_2d_draft) {
    returnToEditor();
    message_.clear();
    exit_code_ = 0;
    diagnostics_.clear();
    diagnostics_offset_ = 0;
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
    const nlohmann::json manifest = {{"schema", "urpg.playtest_session.v1"},
                                     {"project_root", project_root.generic_string()},
                                     {"overlay_dir", session_directory_.generic_string()},
                                     {"map_id", map_id},
                                     {"spawn", spawn},
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
    state_ = PlaytestSessionState::Starting;
    message_ = "Playtest started with the current unsaved map overlay.";
    return true;
}

void PlaytestSessionController::update() {
    if (!isActive()) return;
    pollDiagnostics();
    if (process_.isRunning(&exit_code_)) {
        state_ = PlaytestSessionState::Running;
        return;
    }
    pollDiagnostics();
    state_ = exit_code_ == 0 ? PlaytestSessionState::Exited : PlaytestSessionState::Crashed;
    message_ = exit_code_ == 0 ? "Playtest runtime exited; the editor remains open." :
                                 "Playtest runtime exited with code " + std::to_string(exit_code_) + ".";
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
