#pragma once

#include <mutex>
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
};

class RuntimeDiagnostics {
  public:
    static void emit(DiagnosticSeverity severity, std::string subsystem, std::string code, std::string message) {
        std::lock_guard<std::mutex> lock(mutex());
        entries().push_back({
            severity,
            std::move(subsystem),
            std::move(code),
            std::move(message),
        });
    }

    static void info(std::string subsystem, std::string code, std::string message) {
        emit(DiagnosticSeverity::Info, std::move(subsystem), std::move(code), std::move(message));
    }

    static void warning(std::string subsystem, std::string code, std::string message) {
        emit(DiagnosticSeverity::Warning, std::move(subsystem), std::move(code), std::move(message));
    }

    static void error(std::string subsystem, std::string code, std::string message) {
        emit(DiagnosticSeverity::Error, std::move(subsystem), std::move(code), std::move(message));
    }

    static void fatal(std::string subsystem, std::string code, std::string message) {
        emit(DiagnosticSeverity::Fatal, std::move(subsystem), std::move(code), std::move(message));
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
