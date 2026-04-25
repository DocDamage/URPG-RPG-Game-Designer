#include "engine/core/items/loot_affix_generator.h"

#include <catch2/catch_test_macros.hpp>

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
