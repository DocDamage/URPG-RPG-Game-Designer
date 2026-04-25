#include "editor/narrative/narrative_continuity_panel.h"

#include <utility>

namespace urpg::editor {

void NarrativeContinuityPanel::setDiagnostics(std::vector<std::string> diagnostics) {
    diagnostics_ = std::move(diagnostics);
}

void NarrativeContinuityPanel::render() {
    snapshot_ = {
        {"panel", "narrative_continuity"},
        {"diagnostic_count", diagnostics_.size()},
        {"diagnostics", diagnostics_},
    };
}

nlohmann::json NarrativeContinuityPanel::lastRenderSnapshot() const {
    return snapshot_;
}

} // namespace urpg::editor
