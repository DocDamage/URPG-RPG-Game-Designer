#include "engine/core/crafting/crafting_economy_loop.h"

#include <algorithm>
#include <set>

namespace urpg::crafting {
namespace {

bool hasFlag(const std::vector<std::string>& flags, const std::string& flag) {
    return flag.empty() || std::find(flags.begin(), flags.end(), flag) != flags.end();
}

GatheringSource sourceFromJson(const nlohmann::json& json) {
    GatheringSource source;
    source.id = json.value("id", "");
    source.required_flag = json.value("required_flag", "");
    if (json.contains("yields") && json["yields"].is_object()) {
        for (auto it = json["yields"].begin(); it != json["yields"].end(); ++it) {
            source.yields[it.key()] = it.value().get<int>();
        }
    }
    return source;
}

nlohmann::json sourceToJson(const GatheringSource& source) {
    return {{"id", source.id}, {"required_flag", source.required_flag}, {"yields", source.yields}};
}

CraftingRecipe recipeFromJson(const nlohmann::json& json) {
    CraftingRecipe recipe;
    recipe.id = json.value("id", "");
    recipe.unlock_flag = json.value("unlock_flag", "");
    if (json.contains("ingredients") && json["ingredients"].is_object()) {
        for (auto it = json["ingredients"].begin(); it != json["ingredients"].end(); ++it) {
            recipe.ingredients[it.key()] = it.value().get<int>();
        }
    }
    if (json.contains("results") && json["results"].is_object()) {
        for (auto it = json["results"].begin(); it != json["results"].end(); ++it) {
            recipe.results[it.key()] = it.value().get<int>();
        }
    }
    return recipe;
}

nlohmann::json recipeToJson(const CraftingRecipe& recipe) {
    return {{"id", recipe.id},
            {"unlock_flag", recipe.unlock_flag},
            {"ingredients", recipe.ingredients},
            {"results", recipe.results}};
}

} // namespace

CraftingEconomyLoopDocument CraftingEconomyLoopDocument::fromJson(const nlohmann::json& json) {
    CraftingEconomyLoopDocument document;
    document.loop_id = json.value("loop_id", "");
    if (json.contains("gathering_sources") && json["gathering_sources"].is_array()) {
        for (const auto& source_json : json["gathering_sources"]) {
            document.gathering_sources.push_back(sourceFromJson(source_json));
        }
    }
    if (json.contains("recipes") && json["recipes"].is_array()) {
        for (const auto& recipe_json : json["recipes"]) {
            document.recipes.push_back(recipeFromJson(recipe_json));
        }
    }
    return document;
}

nlohmann::json CraftingEconomyLoopDocument::toJson() const {
    nlohmann::json sources = nlohmann::json::array();
    for (const auto& source : gathering_sources) {
        sources.push_back(sourceToJson(source));
    }
    nlohmann::json recipe_array = nlohmann::json::array();
    for (const auto& recipe : recipes) {
        recipe_array.push_back(recipeToJson(recipe));
    }
    return {{"schema_version", "urpg.crafting_economy_loop.v1"},
            {"loop_id", loop_id},
            {"gathering_sources", std::move(sources)},
            {"recipes", std::move(recipe_array)}};
}

std::vector<CraftingEconomyDiagnostic> CraftingEconomyLoopDocument::validate() const {
    std::vector<CraftingEconomyDiagnostic> diagnostics;
    if (loop_id.empty()) {
        diagnostics.push_back({"missing_loop_id", "Crafting economy loop requires a loop_id.", ""});
    }
    std::set<std::string> ids;
    for (const auto& source : gathering_sources) {
        if (source.id.empty() || source.yields.empty()) {
            diagnostics.push_back({"invalid_gathering_source", "Gathering source requires id and yields.", source.id});
        }
        if (!source.id.empty() && !ids.insert("source:" + source.id).second) {
            diagnostics.push_back({"duplicate_gathering_source", "Gathering source id is duplicated.", source.id});
        }
    }
    CraftingRegistry registry;
    for (const auto& recipe : recipes) {
        if (recipe.id.empty() || recipe.results.empty()) {
            diagnostics.push_back({"invalid_recipe", "Recipe requires id and results.", recipe.id});
        }
        registry.addRecipe(recipe);
    }
    for (const auto& diagnostic : registry.validate()) {
        diagnostics.push_back({diagnostic.code, diagnostic.message, ""});
    }
    return diagnostics;
}

CraftingEconomyPreview CraftingEconomyLoopDocument::preview(const std::string& source_id, const std::string& recipe_id,
                                                            const CraftingEconomyState& state) const {
    CraftingEconomyPreview preview;
    preview.recipe_id = recipe_id;
    preview.inventory_after_gather = state.inventory;
    preview.diagnostics = validate();
    if (!preview.diagnostics.empty()) {
        return preview;
    }

    const auto source = std::find_if(gathering_sources.begin(), gathering_sources.end(),
                                     [&](const GatheringSource& value) { return value.id == source_id; });
    if (source == gathering_sources.end()) {
        preview.diagnostics.push_back({"missing_gathering_source", "Gathering source does not exist.", source_id});
        return preview;
    }
    if (!hasFlag(state.flags, source->required_flag)) {
        preview.diagnostics.push_back({"gathering_source_locked", "Gathering source required flag is missing.",
                                       source_id});
        return preview;
    }
    for (const auto& [item, count] : source->yields) {
        preview.inventory_after_gather[item] += count;
    }

    CraftingRegistry registry;
    for (const auto& recipe : recipes) {
        registry.addRecipe(recipe);
    }
    const auto craft_preview = registry.preview(recipe_id, preview.inventory_after_gather, state.flags);
    preview.can_craft = craft_preview.canCraft;
    preview.missing_ingredients = craft_preview.missingIngredients;
    preview.inventory_after_craft = preview.inventory_after_gather;
    if (preview.can_craft) {
        const auto recipe = std::find_if(recipes.begin(), recipes.end(), [&](const CraftingRecipe& value) {
            return value.id == recipe_id;
        });
        if (recipe != recipes.end()) {
            for (const auto& [item, count] : recipe->ingredients) {
                preview.inventory_after_craft[item] -= count;
            }
            for (const auto& [item, count] : recipe->results) {
                preview.inventory_after_craft[item] += count;
            }
        }
    }
    return preview;
}

bool CraftingEconomyLoopDocument::apply(const std::string& source_id, const std::string& recipe_id,
                                        CraftingEconomyState& state) const {
    const auto result = preview(source_id, recipe_id, state);
    if (!result.can_craft || !result.diagnostics.empty()) {
        return false;
    }
    state.inventory = result.inventory_after_craft;
    return true;
}

nlohmann::json craftingEconomyPreviewToJson(const CraftingEconomyPreview& preview) {
    nlohmann::json diagnostics = nlohmann::json::array();
    for (const auto& diagnostic : preview.diagnostics) {
        diagnostics.push_back({{"code", diagnostic.code}, {"message", diagnostic.message}, {"id", diagnostic.id}});
    }
    return {{"can_craft", preview.can_craft},
            {"recipe_id", preview.recipe_id},
            {"inventory_after_gather", preview.inventory_after_gather},
            {"inventory_after_craft", preview.inventory_after_craft},
            {"missing_ingredients", preview.missing_ingredients},
            {"diagnostics", std::move(diagnostics)}};
}

} // namespace urpg::crafting
