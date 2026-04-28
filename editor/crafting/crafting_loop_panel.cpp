#include "editor/crafting/crafting_loop_panel.h"

#include <utility>

namespace urpg::editor {

void CraftingLoopPanel::bindDocument(urpg::crafting::CraftingEconomyLoopDocument document) {
    document_ = std::move(document);
}

void CraftingLoopPanel::setState(urpg::crafting::CraftingEconomyState state) {
    state_ = std::move(state);
}

void CraftingLoopPanel::setSelection(std::string source_id, std::string recipe_id) {
    selected_source_id_ = std::move(source_id);
    selected_recipe_id_ = std::move(recipe_id);
}

bool CraftingLoopPanel::applySelection() {
    return document_.apply(selected_source_id_, selected_recipe_id_, state_);
}

void CraftingLoopPanel::render() {
    snapshot_ = {{"panel", "crafting_loop"},
                 {"loop_id", document_.loop_id},
                 {"selected_source_id", selected_source_id_},
                 {"selected_recipe_id", selected_recipe_id_},
                 {"state", {{"inventory", state_.inventory}, {"flags", state_.flags}, {"gold", state_.gold}}},
                 {"document", document_.toJson()},
                 {"preview", urpg::crafting::craftingEconomyPreviewToJson(
                                 document_.preview(selected_source_id_, selected_recipe_id_, state_))}};
}

nlohmann::json CraftingLoopPanel::lastRenderSnapshot() const {
    return snapshot_;
}

} // namespace urpg::editor
