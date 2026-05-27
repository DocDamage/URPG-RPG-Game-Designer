#pragma once

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

    nlohmann::json toJson() const;
};

std::set<std::string> SupportedMzEventCommands();
MzEventCommandCoverageReport AnalyzeMzEventCommandCoverage(const std::vector<std::string>& commands);

} // namespace urpg::compat
