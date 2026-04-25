#include "editor/save/save_debugger_panel.h"

#include <utility>

namespace urpg::editor {

void SaveDebuggerPanel::setSlot(urpg::save::SaveDebugSlot slot) {
    slot_ = std::move(slot);
}

void SaveDebuggerPanel::render() {
    urpg::save::SaveDebugger debugger;
    snapshot_ = {
        {"panel", "save_debugger"},
        {"slot", debugger.exportDiagnostics(slot_)},
    };
}

nlohmann::json SaveDebuggerPanel::lastRenderSnapshot() const {
    return snapshot_;
}

} // namespace urpg::editor
