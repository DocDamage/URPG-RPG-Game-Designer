#pragma once

#include "engine/core/codex/bestiary_registry.h"

#include <string>

namespace urpg::editor::codex {

class CodexPanel {
public:
    [[nodiscard]] static std::string snapshot(const urpg::codex::BestiaryRegistry& registry);
};

} // namespace urpg::editor::codex
