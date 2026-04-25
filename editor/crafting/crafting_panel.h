#pragma once

#include "engine/core/crafting/crafting_registry.h"

#include <string>

namespace urpg::editor::crafting {

class CraftingPanel {
public:
    [[nodiscard]] static std::string snapshot(const urpg::crafting::CraftingRegistry& registry);
};

} // namespace urpg::editor::crafting
