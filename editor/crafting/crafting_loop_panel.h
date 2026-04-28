#pragma once

#include "engine/core/crafting/crafting_economy_loop.h"

#include <nlohmann/json.hpp>

namespace urpg::editor {

class CraftingLoopPanel {
public:
    void bindDocument(urpg::crafting::CraftingEconomyLoopDocument document);
    void setState(urpg::crafting::CraftingEconomyState state);
    void setSelection(std::string source_id, std::string recipe_id);
    bool applySelection();
    void render();
    nlohmann::json lastRenderSnapshot() const;

private:
    urpg::crafting::CraftingEconomyLoopDocument document_;
    urpg::crafting::CraftingEconomyState state_;
    std::string selected_source_id_;
    std::string selected_recipe_id_;
    nlohmann::json snapshot_;
};

} // namespace urpg::editor
