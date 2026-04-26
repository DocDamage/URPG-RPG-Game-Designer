#pragma once

#include "engine/core/message/dialogue_registry.h"

#include <filesystem>
#include <string>
#include <vector>

namespace urpg::message {

enum class DialogueLoadSeverity {
    Info,
    Warning,
    Error,
};

struct DialogueLoadDiagnostic {
    DialogueLoadSeverity severity = DialogueLoadSeverity::Info;
    std::string code;
    std::filesystem::path path;
    std::string message;
};

struct DialogueProjectLoadResult {
    bool loaded = false;
    size_t conversation_count = 0;
    size_t node_count = 0;
    std::filesystem::path source_path;
    std::vector<DialogueLoadDiagnostic> diagnostics;
};

class DialogueProjectLoader {
  public:
    static DialogueProjectLoadResult loadProject(DialogueRegistry& registry, const std::filesystem::path& project_root);
    static DialogueProjectLoadResult importDocument(DialogueRegistry& registry, const nlohmann::json& document,
                                                    const std::filesystem::path& source_path = {});
};

} // namespace urpg::message
