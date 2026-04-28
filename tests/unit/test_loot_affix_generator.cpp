#include "engine/core/items/loot_affix_generator.h"
#include "editor/items/loot_generator_panel.h"

#include <catch2/catch_test_macros.hpp>
#include <fstream>

namespace {

nlohmann::json loadLootGeneratorFixture() {
    std::ifstream stream(std::string(URPG_SOURCE_DIR) + "/content/fixtures/loot_generator_fixture.json");
    REQUIRE(stream.good());
    return nlohmann::json::parse(stream);
}

} // namespace

TEST_CASE("Loot affix generator uses seeded rolls and validates rarity pools", "[items][balance][ffs12]") {
    urpg::items::LootAffixGenerator generator;
    generator.addAffix({"sharp", "rare", 2, 5, 120});
    generator.addAffix({"bright", "rare", 1, 3, 110});

    const auto first = generator.roll("rare", 123);
    const auto second = generator.roll("rare", 123);

    REQUIRE(first.has_value());
    REQUIRE(second.has_value());
    REQUIRE(first->id == second->id);
    REQUIRE(first->value >= first->min_value);
    REQUIRE(first->value <= first->max_value);
    REQUIRE(generator.validate().empty());
    REQUIRE_FALSE(generator.roll("legendary", 1).has_value());
}

TEST_CASE("Loot generator creates deterministic affixed items and editor preview", "[items][loot][generator]") {
    const auto document = urpg::items::LootGeneratorDocument::fromJson(loadLootGeneratorFixture());

    REQUIRE(document.validate({"weapon", "armor"}).empty());

    const auto first = document.preview(77, 4);
    const auto second = document.preview(77, 4);
    REQUIRE(first.items.size() == 4);
    REQUIRE(urpg::items::lootGeneratorPreviewToJson(first) == urpg::items::lootGeneratorPreviewToJson(second));
    REQUIRE(first.items.front().affixes.size() == 2);
    REQUIRE(first.items.front().value > 0);

    urpg::editor::items::LootGeneratorPanel panel;
    panel.loadDocument(document);
    panel.setPreviewSeed(77, 4);
    panel.render();

    REQUIRE(panel.snapshot().item_count == 4);
    REQUIRE(panel.saveProjectData() == document.toJson());
}

TEST_CASE("Loot generator reports invalid items and missing rarity pools", "[items][loot][generator]") {
    urpg::items::LootGeneratorDocument document;
    document.id = "bad";
    document.affixes_per_item = 1;
    document.base_items.push_back({"broken", "unknown", "legendary", -1, {}});
    document.affixes.push_back({"bad_affix", "rare", 5, 1, 0});

    const auto diagnostics = document.validate({"weapon"});
    REQUIRE(diagnostics.size() >= 3);

    const auto preview = document.preview(3, 1);
    REQUIRE(preview.items.size() == 1);
    REQUIRE_FALSE(preview.diagnostics.empty());
}
