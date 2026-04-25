#pragma once

#include <map>
#include <string>
#include <vector>

namespace urpg::crafting {

struct CraftingDiagnostic {
    std::string code;
    std::string message;
};

struct CraftingRecipe {
    std::string id;
    std::map<std::string, int> ingredients;
    std::map<std::string, int> results;
    std::string unlock_flag;
};

struct CraftingPreview {
    bool canCraft{false};
    std::map<std::string, int> missingIngredients;
    std::map<std::string, int> results;
};

class CraftingRegistry {
public:
    void addRecipe(CraftingRecipe recipe);
    [[nodiscard]] CraftingPreview preview(const std::string& recipe_id,
                                          const std::map<std::string, int>& inventory,
                                          const std::vector<std::string>& flags) const;
    [[nodiscard]] std::vector<CraftingDiagnostic> validate() const;

private:
    std::vector<CraftingRecipe> recipes_;
};

} // namespace urpg::crafting
