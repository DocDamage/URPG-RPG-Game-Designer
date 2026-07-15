#pragma once

#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <nlohmann/json.hpp>
#include <string>
#include <utility>
#include <vector>

namespace urpg::diagnostics {

enum class DiagnosticSeverity { Info, Warning, Error, Fatal };

struct RuntimeDiagnostic {
    DiagnosticSeverity severity = DiagnosticSeverity::Info;
    std::string subsystem;
    std::string code;
    std::string message;
    std::string map_id;
    std::string object_id;
    std::string source_file;
    long long timestamp = 0;
};

inline std::filesystem::path g_DiagnosticsFilePath;
inline std::string g_ActiveMapId;

class RuntimeDiagnostics {
  public:
    static constexpr size_t kMaxRetainedEntries = 512;

    static std::string severityToString(DiagnosticSeverity severity) {
        switch (severity) {
            case DiagnosticSeverity::Info: return "info";
            case DiagnosticSeverity::Warning: return "warning";
            case DiagnosticSeverity::Error: return "error";
            case DiagnosticSeverity::Fatal: return "fatal";
        }
        return "info";
    }

    static void emit(DiagnosticSeverity severity, std::string subsystem, std::string code, std::string message,
                     std::string map_id = "", std::string object_id = "", std::string source_file = "") {
        std::lock_guard<std::mutex> lock(mutex());
        const long long ts = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        std::string final_map_id = map_id.empty() ? g_ActiveMapId : std::move(map_id);

        entries().push_back({
            severity,
            std::move(subsystem),
            std::move(code),
            std::move(message),
            std::move(final_map_id),
            std::move(object_id),
            std::move(source_file),
            ts
        });

        if (entries().size() > kMaxRetainedEntries) {
            entries().erase(entries().begin(), entries().begin() +
                static_cast<std::ptrdiff_t>(entries().size() - kMaxRetainedEntries));
        }

        if (!g_DiagnosticsFilePath.empty()) {
            try {
                std::ofstream out(g_DiagnosticsFilePath, std::ios::app);
                if (out) {
                    const auto& entry = entries().back();
                    nlohmann::json j;
                    j["version"] = 1;
                    j["severity"] = severityToString(entry.severity);
                    j["subsystem"] = entry.subsystem;
                    j["code"] = entry.code;
                    j["message"] = entry.message;
                    j["map_id"] = entry.map_id;
                    j["object_id"] = entry.object_id;
                    j["source_file"] = entry.source_file;
                    j["timestamp"] = entry.timestamp;
                    out << j.dump() << "\n";
                }
            } catch (...) {}
        }
    }

    static void info(std::string subsystem, std::string code, std::string message,
                     std::string map_id = "", std::string object_id = "", std::string source_file = "") {
        emit(DiagnosticSeverity::Info, std::move(subsystem), std::move(code), std::move(message),
             std::move(map_id), std::move(object_id), std::move(source_file));
    }

    static void warning(std::string subsystem, std::string code, std::string message,
                        std::string map_id = "", std::string object_id = "", std::string source_file = "") {
        emit(DiagnosticSeverity::Warning, std::move(subsystem), std::move(code), std::move(message),
             std::move(map_id), std::move(object_id), std::move(source_file));
    }

    static void error(std::string subsystem, std::string code, std::string message,
                      std::string map_id = "", std::string object_id = "", std::string source_file = "") {
        emit(DiagnosticSeverity::Error, std::move(subsystem), std::move(code), std::move(message),
             std::move(map_id), std::move(object_id), std::move(source_file));
    }

    static void fatal(std::string subsystem, std::string code, std::string message,
                      std::string map_id = "", std::string object_id = "", std::string source_file = "") {
        emit(DiagnosticSeverity::Fatal, std::move(subsystem), std::move(code), std::move(message),
             std::move(map_id), std::move(object_id), std::move(source_file));
    }

    static std::vector<RuntimeDiagnostic> snapshot() {
        std::lock_guard<std::mutex> lock(mutex());
        return entries();
    }

    static void clear() {
        std::lock_guard<std::mutex> lock(mutex());
        entries().clear();
    }

  private:
    static std::vector<RuntimeDiagnostic>& entries() {
        static std::vector<RuntimeDiagnostic> diagnostics;
        return diagnostics;
    }

    static std::mutex& mutex() {
        static std::mutex diagnosticsMutex;
        return diagnosticsMutex;
    }
};

} // namespace urpg::diagnostics
