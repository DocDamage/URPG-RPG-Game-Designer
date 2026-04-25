#include "editor/codex/codex_panel.h"

namespace urpg::editor::codex {

std::string CodexPanel::snapshot(const urpg::codex::BestiaryRegistry& registry) {
    return registry.completionRatio() >= 1.0 ? "codex:complete" : "codex:in-progress";
}

} // namespace urpg::editor::codex
