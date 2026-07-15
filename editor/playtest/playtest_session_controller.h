#pragma once

#include "engine/core/platform/process_runner.h"
#include "engine/core/diagnostics/runtime_diagnostics.h"

#include <filesystem>
#include <ios>
#include <string>

namespace urpg::editor {

enum class PlaytestSessionState { Inactive, Starting, Running, Stopping, Exited, Crashed, Returned };

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
               const std::string& perspective_2d_draft);
    void update();
    void returnToEditor();

    PlaytestSessionState state() const { return state_; }
    bool isActive() const {
        return state_ == PlaytestSessionState::Starting || state_ == PlaytestSessionState::Running ||
               state_ == PlaytestSessionState::Stopping;
    }
    int exitCode() const { return exit_code_; }
    const std::filesystem::path& sessionDirectory() const { return session_directory_; }
    const std::string& message() const { return message_; }
    const std::vector<diagnostics::RuntimeDiagnostic>& diagnostics() const { return diagnostics_; }
    const std::string& capturedStdout() const { return process_.stdoutText(); }
    const std::string& capturedStderr() const { return process_.stderrText(); }

  private:
    std::filesystem::path resolveRuntimeExecutable() const;
    void pollDiagnostics();
    void cleanStaleOverlays(const std::filesystem::path& project_root) const;

    std::filesystem::path runtime_executable_;
    std::filesystem::path session_directory_;
    platform::Process process_;
    PlaytestSessionState state_ = PlaytestSessionState::Inactive;
    int exit_code_ = 0;
    std::string message_;
    std::vector<diagnostics::RuntimeDiagnostic> diagnostics_;
    std::streamoff diagnostics_offset_ = 0;
};

} // namespace urpg::editor
