#pragma once

#include "engine/core/events/event_document.h"

#include <nlohmann/json.hpp>

#include <set>
#include <string>
#include <vector>

namespace urpg::compat {

struct MzEventCommandDiagnostic {
    std::string code;
    std::string message;
    std::string target;

    nlohmann::json toJson() const;
};

struct MzEventCommandCoverageReport {
    size_t total_command_count = 0;
    size_t supported_command_count = 0;
    size_t unsupported_command_count = 0;
    std::vector<std::string> supported_commands;
    std::vector<std::string> unsupported_commands;
    std::vector<MzEventCommandDiagnostic> diagnostics;
    std::vector<nlohmann::json> fallback_records;

    nlohmann::json toJson() const;
};

struct MzEventCommandCapability {
    uint32_t opcode = 0;
    std::string command;
    bool supported = false;
    std::string import_strategy;
    std::string execution_strategy;
    std::string unsupported_fallback;
    events::EventCommandKind native_kind = events::EventCommandKind::Unsupported;
};

struct MzEventCommandImportResult {
    bool supported = false;
    events::EventCommand command;
    std::vector<MzEventCommandDiagnostic> diagnostics;
};

std::set<std::string> SupportedMzEventCommands();
const std::vector<MzEventCommandCapability>& MzEventCommandCompatibilityMatrix();
MzEventCommandCoverageReport AnalyzeMzEventCommandCoverage(const std::vector<std::string>& commands);
MzEventCommandImportResult ImportMzEventCommand(std::string command_id, const std::string& command,
                                                const nlohmann::json& raw_command);

} // namespace urpg::compat
