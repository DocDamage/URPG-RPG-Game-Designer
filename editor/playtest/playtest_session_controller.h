#pragma once

#include "engine/core/platform/process_runner.h"
#include "engine/core/diagnostics/runtime_diagnostics.h"

#include <filesystem>
#include <chrono>
#include <ios>
#include <string>
#include <vector>
#include <utility>

namespace urpg::editor {

enum class PlaytestSessionState {
    Inactive,
    Starting,
    Running,
    Stopping,
    Exited,
    Crashed,
    Returned
};

struct PlaytestReloadResource {
    std::string id;
    std::string kind; // "map", "ability", "dialogue"
    std::string path; // relative to project root
};

class PlaytestSessionController {
  public:
    explicit PlaytestSessionController(std::filesystem::path runtime_executable = {});
    ~PlaytestSessionController();

    bool startSession(const std::filesystem::path& project_root, const std::string& map_id, const std::string& spawn, bool headless,
                      const std::string& grid_json, const std::string& p2d_json);
    void stopSession();
    void update();
    void triggerReload(const std::vector<PlaytestReloadResource>& resources);

    PlaytestSessionState state() const { return m_state; }
    bool isActive() const {
        return m_state == PlaytestSessionState::Starting || m_state == PlaytestSessionState::Running ||
            m_state == PlaytestSessionState::Stopping;
    }
    int exitCode() const { return m_exitCode; }
    const std::vector<diagnostics::RuntimeDiagnostic>& diagnostics() const { return m_diagnostics; }
    void clearDiagnostics() { m_diagnostics.clear(); }
    void cleanStaleOverlays(const std::filesystem::path& project_root);

    const std::filesystem::path& sessionDir() const { return m_sessionDir; }
    const std::string& sessionId() const { return m_sessionId; }
    const std::string& targetMapId() const { return m_targetMapId; }
    const std::string& targetSpawn() const { return m_targetSpawn; }
    std::chrono::milliseconds elapsed() const;
    const std::string& capturedStdout() const { return m_process.getStdout(); }
    const std::string& capturedStderr() const { return m_process.getStderr(); }

  private:
    void changeState(PlaytestSessionState newState);
    void pollDiagnostics();

    PlaytestSessionState m_state = PlaytestSessionState::Inactive;
    int m_exitCode = 0;
    std::string m_sessionId;
    std::string m_targetMapId;
    std::string m_targetSpawn;
    std::filesystem::path m_sessionDir;
    std::chrono::steady_clock::time_point m_startedAt{};
    platform::Process m_process;
    std::filesystem::path m_runtimeExecutable;
    std::vector<diagnostics::RuntimeDiagnostic> m_diagnostics;
    std::streamoff m_diagnosticsReadOffset = 0;
    int m_reloadVersion = 0;
    bool m_reloadPending = false;
};

} // namespace urpg::editor
