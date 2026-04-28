#include "editor/crafting/crafting_loop_panel.h"
#include "engine/core/crafting/crafting_economy_loop.h"
#include "engine/core/crafting/crafting_registry.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

namespace {

std::filesystem::path sourceRootFromMacro() {
#ifdef URPG_SOURCE_DIR
    std::string sourceRoot = URPG_SOURCE_DIR;
    if (sourceRoot.size() >= 2 && sourceRoot.front() == '"' && sourceRoot.back() == '"') {
        sourceRoot = sourceRoot.substr(1, sourceRoot.size() - 2);
    }
    return std::filesystem::path(sourceRoot);
#else
    return {};
#endif
}

} // namespace

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

TEST_CASE("Crafting economy loop gathers crafts and exposes editor preview", "[crafting][economy][loop]") {
    const auto fixturePath = sourceRootFromMacro() / "content" / "fixtures" / "crafting_economy_loop_fixture.json";
    std::ifstream input(fixturePath);
    REQUIRE(input.is_open());
    nlohmann::json fixture;
    input >> fixture;

    const auto document = urpg::crafting::CraftingEconomyLoopDocument::fromJson(fixture);
    REQUIRE(document.validate().empty());

    urpg::crafting::CraftingEconomyState state;
    state.flags = {"alchemy_unlocked"};
    state.inventory = {{"herb", 1}, {"water", 1}};

    const auto preview = document.preview("forest_patch", "recipe.potion", state);
    REQUIRE(preview.can_craft);
    REQUIRE(preview.inventory_after_gather.at("herb") == 3);
    REQUIRE(preview.inventory_after_craft.at("potion") == 1);
    REQUIRE(preview.inventory_after_craft.at("herb") == 1);

    urpg::editor::CraftingLoopPanel panel;
    panel.bindDocument(document);
    panel.setState(state);
    panel.setSelection("forest_patch", "recipe.potion");
    REQUIRE(panel.applySelection());
    panel.render();

    const auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["panel"] == "crafting_loop");
    REQUIRE(snapshot["preview"]["can_craft"] == false);
    REQUIRE(snapshot["state"]["inventory"]["potion"] == 1);
}

TEST_CASE("Crafting economy loop reports locked gathering source", "[crafting][economy][loop]") {
    const auto document = urpg::crafting::CraftingEconomyLoopDocument::fromJson(nlohmann::json{
        {"loop_id", "locked"},
        {"gathering_sources", {{{"id", "mine"}, {"required_flag", "pickaxe"}, {"yields", {{"ore", 1}}}}}},
        {"recipes", {{{"id", "recipe.ingot"}, {"ingredients", {{"ore", 1}}}, {"results", {{"ingot", 1}}}}}},
    });

    const auto preview = document.preview("mine", "recipe.ingot", {});
    REQUIRE_FALSE(preview.can_craft);
    REQUIRE(preview.diagnostics.size() == 1);
    REQUIRE(preview.diagnostics[0].code == "gathering_source_locked");
}
