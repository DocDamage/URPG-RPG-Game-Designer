#include "editor/gameplay/gameplay_recipe_panel.h"
#include "engine/core/gameplay/gameplay_recipe_document.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("gameplay recipe template previews, applies idempotently, and reverts its owned target",
          "[gameplay][recipe]") {
    const auto recipes = urpg::gameplay::builtInGameplayRecipeTemplates();
    REQUIRE(recipes.size() == 2);

    urpg::gameplay::GameplayRecipeProjectDocument project;
    urpg::gameplay::GameplayRecipeService service;
    const auto preview = service.preview(project, recipes.front(), {}, "confirm_interact");
    REQUIRE(preview.can_apply);
    REQUIRE(preview.runtime_preview.active_rules.size() == 1);

    const auto applied = service.apply(project, recipes.front());
    REQUIRE(applied.success);
    REQUIRE(applied.code == "applied");
    REQUIRE(project.features.size() == 1);
    REQUIRE(project.recipe_receipts.size() == 1);

    const auto repeated = service.apply(project, recipes.front());
    REQUIRE(repeated.success);
    REQUIRE(repeated.already_applied);
    REQUIRE(project.features.size() == 1);

    project.features.emplace("unrelated", recipes.front().target);
    project.features.at("unrelated").id = "unrelated";
    const auto reverted = service.revert(project, recipes.front().id);
    REQUIRE(reverted.success);
    REQUIRE_FALSE(project.features.contains(recipes.front().target.id));
    REQUIRE(project.features.contains("unrelated"));
    REQUIRE(project.recipe_receipts.empty());
}

TEST_CASE("gameplay recipe rejects collisions and preserves targets changed after apply", "[gameplay][recipe]") {
    const auto recipe = urpg::gameplay::builtInGameplayRecipeTemplates().front();
    urpg::gameplay::GameplayRecipeService service;
    urpg::gameplay::GameplayRecipeProjectDocument project;
    project.features.emplace(recipe.target.id, recipe.target);
    project.features.at(recipe.target.id).display_name = "Existing authored feature";

    const auto collision = service.apply(project, recipe);
    REQUIRE_FALSE(collision.success);
    REQUIRE(collision.code == "recipe_rejected");
    REQUIRE(collision.diagnostics.size() == 1);
    REQUIRE(collision.diagnostics.front().code == "target_conflict");

    project.features.clear();
    REQUIRE(service.apply(project, recipe).success);
    project.features.at(recipe.target.id).display_name = "Edited after recipe apply";
    const auto revert = service.revert(project, recipe.id);
    REQUIRE_FALSE(revert.success);
    REQUIRE(revert.code == "target_modified");
    REQUIRE(project.features.contains(recipe.target.id));
    REQUIRE(project.recipe_receipts.contains(recipe.id));
}

TEST_CASE("gameplay recipe panel projects preview and command state", "[gameplay][recipe][editor]") {
    urpg::editor::gameplay::GameplayRecipePanel panel;
    panel.selectRecipe(urpg::gameplay::builtInGameplayRecipeTemplates().front());
    panel.render();
    REQUIRE(panel.snapshot().can_apply);
    REQUIRE(panel.snapshot().status == "ready");
    REQUIRE(panel.applySelectedRecipe());
    REQUIRE(panel.snapshot().already_applied);
    REQUIRE(panel.snapshot().can_revert);
    REQUIRE(panel.revertSelectedRecipe());
    REQUIRE_FALSE(panel.snapshot().can_revert);
}
