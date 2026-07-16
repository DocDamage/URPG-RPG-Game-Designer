#pragma once

#include "engine/core/platform/process_runner.h"
#include "engine/core/diagnostics/runtime_diagnostics.h"

#include <chrono>
#include <cstddef>
#include <filesystem>
#include <ios>
#include <string>
#include <vector>

namespace urpg::editor {

enum class PlaytestSessionState { Inactive, Starting, Running, Stopping, Exited, Crashed, Returned };

struct PlaytestSupportBundleResult {
    bool success = false;
    std::filesystem::path path;
    std::string message;
    size_t diagnostic_count = 0;
};

struct PlaytestReturnContext {
    bool valid = false;
    std::string map_id;
    std::string spawn;
    std::string selected_object_id;
    std::string checkpoint_id;
};

// Owns exactly one runtime child process started by the editor. Each launch
// receives a private, disposable overlay containing the current unsaved map
// drafts, so normal project content is never mutated merely to playtest it.
class PlaytestSessionController {
  public:
    explicit PlaytestSessionController(std::filesystem::path runtime_executable = {});
    ~PlaytestSessionController();

    bool start(const std::filesystem::path& project_root,
               const std::string& map_id,
               const std::string& spawn,
               const std::string& grid_draft,
               const std::string& perspective_2d_draft,
               const std::string& selected_object_id = {});
    bool startFromHere(const std::filesystem::path& project_root, const std::string& map_id,
                       int32_t tile_x, int32_t tile_y, const std::string& grid_draft,
                       const std::string& perspective_2d_draft, const std::string& selected_object_id = {});
    bool teleportHere(const std::string& map_id, int32_t tile_x, int32_t tile_y,
                      const std::string& selected_object_id = {});
    void returnToDiagnostic(const diagnostics::RuntimeDiagnostic& diagnostic, int32_t tile_x, int32_t tile_y);
    void update();
    void returnToEditor();

    PlaytestSessionState state() const { return state_; }
    bool isActive() const {
        return state_ == PlaytestSessionState::Starting || state_ == PlaytestSessionState::Running ||
               state_ == PlaytestSessionState::Stopping;
    }
    int exitCode() const { return exit_code_; }
    const std::filesystem::path& sessionDirectory() const { return session_directory_; }
    const std::string& mapId() const { return map_id_; }
    const std::string& spawn() const { return spawn_; }
    const std::string& selectedObjectId() const { return selected_object_id_; }
    const std::string& checkpointId() const { return checkpoint_id_; }
    const PlaytestReturnContext& lastReturnContext() const { return last_return_context_; }
    std::chrono::seconds elapsed() const;
    const std::string& message() const { return message_; }
    const std::vector<diagnostics::RuntimeDiagnostic>& diagnostics() const { return diagnostics_; }
    const std::string& capturedStdout() const { return process_.stdoutText(); }
    const std::string& capturedStderr() const { return process_.stderrText(); }
    // Writes bounded support evidence from this disposable session. It omits
    // project/session paths, process output, diagnostic messages/source paths,
    // and runtime object IDs.
    PlaytestSupportBundleResult writeRedactedSupportBundle() const;

  private:
    std::filesystem::path resolveRuntimeExecutable() const;
    void pollDiagnostics();
    void cleanStaleOverlays(const std::filesystem::path& project_root) const;

    std::filesystem::path runtime_executable_;
    std::filesystem::path session_directory_;
    platform::Process process_;
    PlaytestSessionState state_ = PlaytestSessionState::Inactive;
    int exit_code_ = 0;
    std::string map_id_;
    std::string spawn_;
    std::string selected_object_id_;
    std::string checkpoint_id_;
    PlaytestReturnContext last_return_context_;
    std::chrono::steady_clock::time_point started_at_{};
    std::string message_;
    std::vector<diagnostics::RuntimeDiagnostic> diagnostics_;
    std::streamoff diagnostics_offset_ = 0;
};

} // namespace urpg::editor
