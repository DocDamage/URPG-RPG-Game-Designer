#include "engine/core/diagnostics/startup_diagnostics.h"

#include <chrono>
#include <cstdint>
#include <fstream>
#include <nlohmann/json.hpp>

namespace urpg::diagnostics {

namespace {

std::filesystem::path fallbackLogRoot() {
    return std::filesystem::temp_directory_path() / "urpg" / "startup_diagnostics";
}

std::string nowUtcEpochSeconds() {
    const auto now = std::chrono::system_clock::now();
    const auto seconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
    const auto count = seconds.time_since_epoch().count();
    return std::to_string(count);
}

} // namespace

const char* toString(StartupDiagnosticSeverity severity) {
    switch (severity) {
    case StartupDiagnosticSeverity::Info:
        return "info";
    case StartupDiagnosticSeverity::Warning:
        return "warning";
    case StartupDiagnosticSeverity::Error:
        return "error";
    case StartupDiagnosticSeverity::Fatal:
        return "fatal";
    }
    return "unknown";
}

std::filesystem::path startupDiagnosticsLogPath(const std::string& app_id, const std::filesystem::path& project_root) {
    std::error_code ec;
    if (!project_root.empty() && std::filesystem::is_directory(project_root, ec)) {
        return project_root / ".urpg" / "logs" / (app_id + "_startup.jsonl");
    }

    return fallbackLogRoot() / (app_id + "_startup.jsonl");
}

StartupDiagnosticWriteResult writeStartupDiagnostic(const StartupDiagnosticRecord& record) {
    StartupDiagnosticWriteResult result;
    result.log_path = startupDiagnosticsLogPath(record.app_id, record.project_root);

    std::error_code ec;
    std::filesystem::create_directories(result.log_path.parent_path(), ec);
    if (ec) {
        result.error = "Unable to create startup diagnostics directory: " + ec.message();
        return result;
    }

    std::ofstream out(result.log_path, std::ios::app | std::ios::binary);
    if (!out) {
        result.error = "Unable to open startup diagnostics log.";
        return result;
    }

    nlohmann::json row = {
        {"schema", "urpg.startup_diagnostic.v1"},
        {"timestamp_utc_epoch_seconds", nowUtcEpochSeconds()},
        {"app", record.app_id},
        {"severity", toString(record.severity)},
        {"code", record.code},
        {"message", record.message},
        {"project_root", record.project_root.generic_string()},
        {"headless", record.headless},
    };
    out << row.dump() << '\n';
    result.written = static_cast<bool>(out);
    if (!result.written) {
        result.error = "Unable to write startup diagnostics log.";
    }
    return result;
}

std::optional<StartupDiagnosticRecord> validateStartupInputs(const std::string& app_id,
                                                             const std::filesystem::path& project_root,
                                                             std::uint32_t width,
                                                             std::uint32_t height,
                                                             bool headless) {
    std::error_code ec;
    if (!std::filesystem::is_directory(project_root, ec)) {
        return StartupDiagnosticRecord{
            app_id,
            StartupDiagnosticSeverity::Fatal,
            "project_root_missing",
            "Project root does not exist or is not a directory: " + project_root.string(),
            project_root,
            headless,
        };
    }

    if (width == 0 || height == 0) {
        return StartupDiagnosticRecord{
            app_id,
            StartupDiagnosticSeverity::Fatal,
            "invalid_window_size",
            "Startup width and height must both be greater than zero.",
            project_root,
            headless,
        };
    }

    return std::nullopt;
}

} // namespace urpg::diagnostics
