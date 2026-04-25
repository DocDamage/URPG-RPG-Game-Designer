#include "engine/core/crafting/crafting_registry.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Crafting validates missing ingredients and previews result", "[crafting][simulation][ffs13]") {
    urpg::crafting::CraftingRegistry registry;
    registry.addRecipe({"recipe.potion", {{"herb", 2}, {"water", 1}}, {{"potion", 1}}, "alchemy_unlocked"});

    const auto preview = registry.preview("recipe.potion", {{"herb", 1}, {"water", 1}}, {"alchemy_unlocked"});

    REQUIRE_FALSE(preview.canCraft);
    REQUIRE(preview.missingIngredients.at("herb") == 1);
    REQUIRE(preview.results.at("potion") == 1);
}

TEST_CASE("Crafting rejects recipes that consume their own result", "[crafting][simulation][ffs13]") {
    urpg::crafting::CraftingRegistry registry;
    registry.addRecipe({"recipe.loop", {{"potion", 1}}, {{"potion", 2}}, ""});

    const auto diagnostics = registry.validate();
    REQUIRE_FALSE(diagnostics.empty());
    REQUIRE(diagnostics.front().code == "recipe_consumes_result");
}
