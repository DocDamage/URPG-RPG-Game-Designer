#pragma once

#include "engine/core/save/save_debugger.h"

#include <nlohmann/json.hpp>

namespace urpg::editor {

class SaveDebuggerPanel {
public:
    void setSlot(urpg::save::SaveDebugSlot slot);
    void render();
    nlohmann::json lastRenderSnapshot() const;

private:
    urpg::save::SaveDebugSlot slot_;
    nlohmann::json snapshot_;
};

} // namespace urpg::editor
