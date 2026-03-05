#include "editor/diagnostics/event_authority_diagnostics.h"

#include <nlohmann/json.hpp>

#include <sstream>

namespace urpg {

namespace {

std::string ReadString(const nlohmann::json& root, const char* key) {
    if (root.contains(key) && root[key].is_string()) {
        return root[key].get<std::string>();
    }
    return {};
}

} // namespace

std::vector<EventAuthorityDiagnosticEntry> EventAuthorityDiagnosticsIndex::ParseJsonl(std::string_view diagnostics_jsonl) {
    std::vector<EventAuthorityDiagnosticEntry> entries;

    std::istringstream stream{std::string(diagnostics_jsonl)};
    std::string line;
    while (std::getline(stream, line)) {
        if (line.empty()) {
            continue;
        }

        try {
            const nlohmann::json root = nlohmann::json::parse(line);
            if (!root.contains("subsystem") || !root["subsystem"].is_string() || root["subsystem"].get<std::string>() != "event_authority") {
                continue;
            }

            EventAuthorityDiagnosticEntry entry;
            entry.ts = ReadString(root, "ts");
            entry.level = ReadString(root, "level");
            entry.event_name = ReadString(root, "event");
            entry.event_id = ReadString(root, "event_id");
            entry.block_id = ReadString(root, "block_id");
            entry.mode = ReadString(root, "mode");
            entry.operation = ReadString(root, "operation");
            entry.error_code = ReadString(root, "error_code");
            entry.message = ReadString(root, "message");
            entries.push_back(std::move(entry));
        } catch (...) {
            // Ignore malformed lines in diagnostics stream.
        }
    }

    return entries;
}

std::optional<EventNavigationTarget> EventAuthorityDiagnosticsIndex::FindNavigationTarget(
    const std::vector<EventAuthorityDiagnosticEntry>& entries,
    std::string_view event_id
) {
    for (const auto& entry : entries) {
        if (entry.event_id == event_id) {
            return EventNavigationTarget{entry.event_id, entry.block_id};
        }
    }

    return std::nullopt;
}

} // namespace urpg
