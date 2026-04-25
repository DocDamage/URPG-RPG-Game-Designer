#include "engine/core/crafting/crafting_registry.h"

#include <algorithm>

namespace urpg::crafting {

void CraftingRegistry::addRecipe(CraftingRecipe recipe) {
    recipes_.push_back(std::move(recipe));
}

CraftingPreview CraftingRegistry::preview(const std::string& recipe_id,
                                          const std::map<std::string, int>& inventory,
                                          const std::vector<std::string>& flags) const {
    CraftingPreview preview;
    const auto it = std::ranges::find_if(recipes_, [&](const CraftingRecipe& recipe) { return recipe.id == recipe_id; });
    if (it == recipes_.end()) {
        return preview;
    }

    preview.results = it->results;
    const bool unlocked = it->unlock_flag.empty() || std::ranges::find(flags, it->unlock_flag) != flags.end();
    preview.canCraft = unlocked;
    for (const auto& [item, needed] : it->ingredients) {
        const int owned = inventory.contains(item) ? inventory.at(item) : 0;
        if (owned < needed) {
            preview.missingIngredients[item] = needed - owned;
            preview.canCraft = false;
        }
    }
    return preview;
}

std::vector<CraftingDiagnostic> CraftingRegistry::validate() const {
    std::vector<CraftingDiagnostic> diagnostics;
    for (const auto& recipe : recipes_) {
        for (const auto& [item, count] : recipe.ingredients) {
            (void)count;
            if (recipe.results.contains(item)) {
                diagnostics.push_back({"recipe_consumes_result", "recipe consumes an item that it also produces"});
            }
        }
    }
    return diagnostics;
}

} // namespace urpg::crafting
