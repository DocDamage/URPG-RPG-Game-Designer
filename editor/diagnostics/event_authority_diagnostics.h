#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace urpg {

struct EventAuthorityDiagnosticEntry {
    std::string ts;
    std::string level;
    std::string event_name;
    std::string event_id;
    std::string block_id;
    std::string mode;
    std::string operation;
    std::string error_code;
    std::string message;
};

struct EventNavigationTarget {
    std::string event_id;
    std::string block_id;
};

class EventAuthorityDiagnosticsIndex {
public:
    static std::vector<EventAuthorityDiagnosticEntry> ParseJsonl(std::string_view diagnostics_jsonl);

    static std::optional<EventNavigationTarget> FindNavigationTarget(
        const std::vector<EventAuthorityDiagnosticEntry>& entries,
        std::string_view event_id
    );
};

} // namespace urpg
