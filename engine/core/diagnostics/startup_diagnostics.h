#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>

namespace urpg::diagnostics {

enum class StartupDiagnosticSeverity {
    Info,
    Warning,
    Error,
    Fatal,
};

struct StartupDiagnosticRecord {
    std::string app_id;
    StartupDiagnosticSeverity severity = StartupDiagnosticSeverity::Error;
    std::string code;
    std::string message;
    std::filesystem::path project_root;
    bool headless = false;
};

struct StartupDiagnosticWriteResult {
    bool written = false;
    std::filesystem::path log_path;
    std::string error;
};

const char* toString(StartupDiagnosticSeverity severity);
std::filesystem::path startupDiagnosticsLogPath(const std::string& app_id, const std::filesystem::path& project_root);
StartupDiagnosticWriteResult writeStartupDiagnostic(const StartupDiagnosticRecord& record);
std::optional<StartupDiagnosticRecord> validateStartupInputs(const std::string& app_id,
                                                             const std::filesystem::path& project_root,
                                                             std::uint32_t width,
                                                             std::uint32_t height,
                                                             bool headless);
std::optional<StartupDiagnosticRecord> validateRuntimeProjectPreflight(const std::string& app_id,
                                                                       const std::filesystem::path& project_root,
                                                                       bool headless);

} // namespace urpg::diagnostics
