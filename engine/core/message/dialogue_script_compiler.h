#pragma once

#include "engine/core/message/message_core.h"

#include <cstddef>
#include <string>
#include <vector>

namespace urpg::message {

enum class DialogueScriptDiagnosticSeverity : uint8_t {
    Warning = 0,
    Error = 1,
};

struct DialogueScriptDiagnostic {
    size_t line = 0;
    DialogueScriptDiagnosticSeverity severity = DialogueScriptDiagnosticSeverity::Error;
    std::string code;
    std::string message;
};

struct DialogueScriptCompileResult {
    std::vector<DialoguePage> pages;
    std::vector<DialogueScriptDiagnostic> diagnostics;

    [[nodiscard]] bool ok() const;
};

[[nodiscard]] DialogueScriptCompileResult compileDialogueScript(const std::string& source);

} // namespace urpg::message
