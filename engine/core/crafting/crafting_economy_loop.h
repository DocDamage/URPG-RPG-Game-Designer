#pragma once

#include "engine/core/crafting/crafting_registry.h"

#include <nlohmann/json.hpp>

#include <map>
#include <string>
#include <vector>

namespace urpg::crafting {

struct GatheringSource {
    std::string id;
    std::map<std::string, int> yields;
    std::string required_flag;
};

struct CraftingEconomyDiagnostic {
    std::string code;
    std::string message;
    std::string id;
};

struct CraftingEconomyState {
    std::map<std::string, int> inventory;
    std::vector<std::string> flags;
    int gold = 0;
};

struct CraftingEconomyPreview {
    bool can_craft = false;
    std::string recipe_id;
    std::map<std::string, int> inventory_after_gather;
    std::map<std::string, int> inventory_after_craft;
    std::map<std::string, int> missing_ingredients;
    std::vector<CraftingEconomyDiagnostic> diagnostics;
};

class CraftingEconomyLoopDocument {
public:
    std::string loop_id;
    std::vector<GatheringSource> gathering_sources;
    std::vector<CraftingRecipe> recipes;

    static CraftingEconomyLoopDocument fromJson(const nlohmann::json& json);
    nlohmann::json toJson() const;
    std::vector<CraftingEconomyDiagnostic> validate() const;
    CraftingEconomyPreview preview(const std::string& source_id, const std::string& recipe_id,
                                   const CraftingEconomyState& state) const;
    bool apply(const std::string& source_id, const std::string& recipe_id, CraftingEconomyState& state) const;
};

nlohmann::json craftingEconomyPreviewToJson(const CraftingEconomyPreview& preview);

} // namespace urpg::crafting
