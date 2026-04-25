#pragma once

#include "engine/core/npc/npc_schedule.h"

#include <string>

namespace urpg::editor::npc {

class NpcPanel {
public:
    [[nodiscard]] static std::string snapshot(const urpg::npc::NpcResolvedState& state);
};

} // namespace urpg::editor::npc
