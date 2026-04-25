#include "editor/crafting/crafting_panel.h"

namespace urpg::editor::crafting {

std::string CraftingPanel::snapshot(const urpg::crafting::CraftingRegistry& registry) {
    return registry.validate().empty() ? "crafting:ready" : "crafting:diagnostics";
}

} // namespace urpg::editor::crafting
