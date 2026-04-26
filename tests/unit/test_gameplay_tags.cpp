#include "engine/core/ability/gameplay_tags.h"
#include <catch2/catch_test_macros.hpp>

using namespace urpg::ability;

TEST_CASE("GameplayTag: Basic Tag Matching", "[ability][tags]") {
    GameplayTag tag1("Effect.Burn");
    GameplayTag tag2("Effect.Burn.Visual");
    GameplayTag tagOther("Effect.Freeze");

    SECTION("Exact match works") {
        REQUIRE(tag1.matches(tag1));
    }

    SECTION("Hierarchical matching works") {
        // 子タグは親タグにマッチする。 "Effect.Burn.Visual" matches "Effect.Burn"
        REQUIRE(tag2.matches(tag1));

        // 親タグは子タグにマッチしない。 "Effect.Burn" does not match "Effect.Burn.Visual"
        REQUIRE_FALSE(tag1.matches(tag2));
    }

    SECTION("Distinct tags do not match") {
        REQUIRE_FALSE(tag1.matches(tagOther));
    }
}

TEST_CASE("GameplayTagContainer: Tag Collection Management", "[ability][tags]") {
    GameplayTagContainer container;

    container.addTag(GameplayTag("State.Invulnerable"));
    container.addTag(GameplayTag("Ability.Passive.Heal"));

    SECTION("HasTag respects container membership and hierarchy") {
        REQUIRE(container.hasTag(GameplayTag("State.Invulnerable")));
        REQUIRE(container.hasTag(GameplayTag("Ability.Passive")));
        REQUIRE_FALSE(container.hasTag(GameplayTag("State.Stunned")));
    }

    SECTION("Batch checking: Any/All tags") {
        std::vector<GameplayTag> anySet = {GameplayTag("State.Invulnerable"), GameplayTag("DUMMY")};
        std::vector<GameplayTag> allSet = {GameplayTag("State.Invulnerable"), GameplayTag("Ability.Passive.Heal")};

        REQUIRE(container.hasAnyTags(anySet));
        REQUIRE(container.hasAllTags(allSet));
    }
}
