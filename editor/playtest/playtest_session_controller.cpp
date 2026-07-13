#include "editor/playtest/playtest_session_controller.h"
#include "engine/core/platform/process_runner.h"
#include "engine/core/save/save_journal.h"
#include "engine/core/version.h"

#include <nlohmann/json.hpp>
#include <chrono>
#include <fstream>
#include <iterator>
#include <algorithm>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace urpg::editor {

namespace {

std::filesystem::path currentExecutablePath() {
#ifdef _WIN32
    std::wstring buffer(MAX_PATH, L'\0');
    DWORD size = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    while (size == buffer.size()) {
        buffer.resize(buffer.size() * 2);
        size = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    }
    buffer.resize(size);
    return std::filesystem::path(buffer);
#else
    std::string buffer(4096, '\0');
    const ssize_t size = readlink("/proc/self/exe", buffer.data(), buffer.size() - 1);
    if (size <= 0) return {};
    buffer.resize(static_cast<size_t>(size));
    return std::filesystem::path(buffer);
#endif
}

} // namespace

PlaytestSessionController::PlaytestSessionController(std::filesystem::path runtime_executable)
    : m_runtimeExecutable(std::move(runtime_executable)) {}

PlaytestSessionController::~PlaytestSessionController() {
    stopSession();
}

std::chrono::milliseconds PlaytestSessionController::elapsed() const {
    if (m_startedAt == std::chrono::steady_clock::time_point{}) {
        return std::chrono::milliseconds{0};
    }
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_startedAt);
}

bool PlaytestSessionController::startSession(const std::filesystem::path& project_root, const std::string& map_id, const std::string& spawn, bool headless,
                                             const std::string& grid_json, const std::string& p2d_json) {
    stopSession();
    m_exitCode = 0;
    m_diagnostics.clear();
    m_diagnosticsReadOffset = 0;
    m_reloadVersion = 0;
    m_reloadPending = false;
    m_targetMapId = map_id;
    m_targetSpawn = spawn;
    m_startedAt = {};

    std::error_code ec;
    if (!std::filesystem::is_directory(project_root, ec) || ec || map_id.empty() ||
        std::filesystem::path(map_id).filename().string() != map_id) {
        changeState(PlaytestSessionState::Crashed);
        return false;
    }

    // Generate a collision-resistant session ID from the high-resolution clock.
    auto now = std::chrono::system_clock::now().time_since_epoch().count();
    m_sessionId = "session_" + std::to_string(now);
    m_sessionDir = project_root / ".urpg" / "playtest" / m_sessionId;
    cleanStaleOverlays(project_root);

    std::filesystem::create_directories(m_sessionDir / "content" / "maps", ec);
    if (ec) {
        changeState(PlaytestSessionState::Crashed);
        return false;
    }

    std::string writeError;
    if (!grid_json.empty()) {
        if (!urpg::SaveJournal::WriteAtomically(
                m_sessionDir / "content" / "maps" / (map_id + ".grid.json"), grid_json, &writeError)) {
            changeState(PlaytestSessionState::Crashed);
            return false;
        }
    }
    if (!p2d_json.empty()) {
        if (!urpg::SaveJournal::WriteAtomically(
                m_sessionDir / "content" / "maps" / (map_id + ".p2d.json"), p2d_json, &writeError)) {
            changeState(PlaytestSessionState::Crashed);
            return false;
        }
    }

    nlohmann::json manifest;
    manifest["schema_version"] = "urpg.playtest_session.v1";
    manifest["editor_version"] = urpg::versionString();
    manifest["project_root"] = project_root.generic_string();
    manifest["session_id"] = m_sessionId;
    manifest["overlay_dir"] = m_sessionDir.generic_string();
    manifest["map_id"] = map_id;
    manifest["spawn"] = spawn;
    manifest["diagnostics_path"] = (m_sessionDir / "diagnostics.jsonl").generic_string();

    const auto manifestPath = m_sessionDir / "session.json";
    if (!urpg::SaveJournal::WriteAtomically(manifestPath, manifest.dump(2) + "\n", &writeError)) {
        changeState(PlaytestSessionState::Crashed);
        return false;
    }

    // Locate Runtime Executable next to Editor unless a test harness supplied one explicitly.
    auto runtimePath = m_runtimeExecutable;
    auto exeDir = currentExecutablePath().parent_path();
#ifdef _WIN32
    if (runtimePath.empty()) runtimePath = exeDir / "urpg_runtime.exe";
    if (!std::filesystem::exists(runtimePath) && m_runtimeExecutable.empty()) {
        // Fallback for build trees where apps reside in build/ preset folder or parent
        if (std::filesystem::exists(exeDir / ".." / "runtime" / "urpg_runtime.exe")) {
            runtimePath = exeDir / ".." / "runtime" / "urpg_runtime.exe";
        } else {
            runtimePath = "urpg_runtime.exe";
        }
    }
#else
    if (runtimePath.empty()) runtimePath = exeDir / "urpg_runtime";
    if (!std::filesystem::exists(runtimePath) && m_runtimeExecutable.empty()) {
        if (std::filesystem::exists(exeDir / ".." / "runtime" / "urpg_runtime")) {
            runtimePath = exeDir / ".." / "runtime" / "urpg_runtime";
        } else {
            runtimePath = "urpg_runtime";
        }
    }
#endif

    platform::ProcessCommand command;
    command.executable = runtimePath;
    command.arguments = {
        "--project-root", project_root.string(),
        "--map", map_id,
        "--spawn", spawn,
        "--session-manifest", manifestPath.string()
    };
    if (headless) {
        command.arguments.push_back("--headless");
    }
    command.workingDirectory = project_root;
    command.captureStdout = true;
    command.captureStderr = true;

    if (!m_process.launch(command)) {
        changeState(PlaytestSessionState::Crashed);
        return false;
    }

    m_startedAt = std::chrono::steady_clock::now();
    changeState(PlaytestSessionState::Starting);
    return true;
}

void PlaytestSessionController::stopSession() {
    if (m_state == PlaytestSessionState::Starting || m_state == PlaytestSessionState::Running ||
        m_state == PlaytestSessionState::Stopping) {
        changeState(PlaytestSessionState::Stopping);
        m_process.terminate();
        m_exitCode = -1;
        changeState(PlaytestSessionState::Returned);
    }
}

void PlaytestSessionController::update() {
    if (m_state != PlaytestSessionState::Starting && m_state != PlaytestSessionState::Running && m_state != PlaytestSessionState::Stopping) {
        return;
    }

    bool running = m_process.isRunning(&m_exitCode);
    pollDiagnostics();

    // Poll for hot reload response
    if (m_state == PlaytestSessionState::Running) {
        std::filesystem::path responsePath = m_sessionDir / "reload_response.json";
        std::error_code ec;
        if (std::filesystem::exists(responsePath, ec)) {
            std::ifstream in(responsePath, std::ios::binary);
            nlohmann::json resp = nlohmann::json::parse(in, nullptr, false);
            in.close();
            std::filesystem::remove(responsePath, ec);

            if (resp.is_object() && resp.value("version", 0) == m_reloadVersion) {
                m_reloadPending = false;
                for (const auto& result : resp.value("results", nlohmann::json::array())) {
                    if (result.is_object()) {
                        std::string id = result.value("id", "");
                        std::string status = result.value("status", "");
                        std::string reason = result.value("reason", "");

                        diagnostics::RuntimeDiagnostic d;
                        d.severity = (status == "accepted") ? diagnostics::DiagnosticSeverity::Info : diagnostics::DiagnosticSeverity::Warning;
                        d.subsystem = "editor.reload";
                        d.code = "reload.result";
                        d.message = "Resource '" + id + "' reload " + status + (reason.empty() ? "" : ": " + reason);
                        d.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch()).count();
                        m_diagnostics.push_back(d);
                    }
                }
            }
        }
    }

    if (!running) {
        if (m_exitCode == 0) {
            changeState(PlaytestSessionState::Returned);
        } else {
            changeState(PlaytestSessionState::Crashed);
        }
    } else if (m_state == PlaytestSessionState::Starting) {
        changeState(PlaytestSessionState::Running);
    }
}

void PlaytestSessionController::triggerReload(const std::vector<PlaytestReloadResource>& resources) {
    if (m_state != PlaytestSessionState::Starting && m_state != PlaytestSessionState::Running) {
        return;
    }
    if (m_reloadPending) {
        diagnostics::RuntimeDiagnostic diagnostic;
        diagnostic.severity = diagnostics::DiagnosticSeverity::Warning;
        diagnostic.subsystem = "editor.reload";
        diagnostic.code = "reload.already_pending";
        diagnostic.message = "A reload request is already pending; wait for its accepted/rejected response.";
        diagnostic.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        m_diagnostics.push_back(std::move(diagnostic));
        if (m_diagnostics.size() > diagnostics::RuntimeDiagnostics::kMaxRetainedEntries) {
            m_diagnostics.erase(m_diagnostics.begin());
        }
        return;
    }

    m_reloadVersion++;
    nlohmann::json req;
    req["version"] = m_reloadVersion;
    req["resources"] = nlohmann::json::array();
    for (const auto& res : resources) {
        nlohmann::json resItem;
        resItem["id"] = res.id;
        resItem["kind"] = res.kind;
        resItem["path"] = res.path;
        req["resources"].push_back(resItem);
    }

    std::string writeError;
    if (urpg::SaveJournal::WriteAtomically(m_sessionDir / "reload_request.json", req.dump(2) + "\n",
                                           &writeError)) {
        m_reloadPending = true;
    }
}

void PlaytestSessionController::pollDiagnostics() {
    std::filesystem::path diagPath = m_sessionDir / "diagnostics.jsonl";
    std::error_code ec;
    if (!std::filesystem::exists(diagPath, ec)) {
        return;
    }

    const auto fileSize = std::filesystem::file_size(diagPath, ec);
    if (ec) {
        return;
    }
    if (m_diagnosticsReadOffset < 0 || static_cast<uintmax_t>(m_diagnosticsReadOffset) > fileSize) {
        m_diagnosticsReadOffset = 0;
    }

    std::ifstream in(diagPath, std::ios::binary);
    if (!in) {
        return;
    }
    in.seekg(m_diagnosticsReadOffset);

    const std::string chunk{std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
    size_t consumed = 0;
    while (true) {
        const auto newline = chunk.find('\n', consumed);
        if (newline == std::string::npos) break;
        const std::string line = chunk.substr(consumed, newline - consumed);
        consumed = newline + 1;
        nlohmann::json j = nlohmann::json::parse(line, nullptr, false);
        if (j.is_object() && j.value("version", 0) == 1 && j.contains("severity") &&
            j.contains("subsystem") && j.contains("code") && j.contains("message")) {
            diagnostics::RuntimeDiagnostic diag;
            const std::string sevStr = j.value("severity", "info");
            if (sevStr == "warning") diag.severity = diagnostics::DiagnosticSeverity::Warning;
            else if (sevStr == "error") diag.severity = diagnostics::DiagnosticSeverity::Error;
            else if (sevStr == "fatal") diag.severity = diagnostics::DiagnosticSeverity::Fatal;

            diag.subsystem = j.value("subsystem", "");
            diag.code = j.value("code", "");
            diag.message = j.value("message", "");
            diag.map_id = j.value("map_id", "");
            diag.object_id = j.value("object_id", "");
            diag.source_file = j.value("source_file", "");
            diag.timestamp = j.value("timestamp", 0LL);
            m_diagnostics.push_back(std::move(diag));
            if (m_diagnostics.size() > diagnostics::RuntimeDiagnostics::kMaxRetainedEntries) {
                m_diagnostics.erase(m_diagnostics.begin());
            }
        }
    }
    m_diagnosticsReadOffset += static_cast<std::streamoff>(consumed);
}

void PlaytestSessionController::cleanStaleOverlays(const std::filesystem::path& project_root) {
    std::filesystem::path playtestRoot = project_root / ".urpg" / "playtest";
    std::error_code ec;
    if (!std::filesystem::exists(playtestRoot, ec)) {
        return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(playtestRoot, ec)) {
        if (ec) {
            return;
        }
        const auto lastWrite = entry.last_write_time(ec);
        if (ec) {
            ec.clear();
            continue;
        }
        const bool stale = lastWrite < std::filesystem::file_time_type::clock::now() - std::chrono::hours(24 * 7);
        if (stale && entry.is_directory(ec) && !entry.is_symlink(ec) &&
            entry.path().filename().string() != m_sessionId) {
            std::filesystem::remove_all(entry.path(), ec);
            ec.clear();
        }
    }
}

void PlaytestSessionController::changeState(PlaytestSessionState newState) {
    m_state = newState;
}

} // namespace urpg::editor
