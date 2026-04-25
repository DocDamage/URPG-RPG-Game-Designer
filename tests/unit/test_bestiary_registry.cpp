#include "engine/core/codex/bestiary_registry.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Bestiary state persists and tracks completion", "[codex][simulation][ffs13]") {
    urpg::codex::BestiaryRegistry registry;
    registry.addEntry({"enemy.slime", "Slime", {"ice"}, {"gel"}, "codex.slime"});
    registry.markSeen("enemy.slime");
    registry.markScanned("enemy.slime");
    registry.markDefeated("enemy.slime");

    const auto restored = urpg::codex::BestiaryRegistry::fromJson(registry.toJson());

    REQUIRE(restored.stateFor("enemy.slime").seen);
    REQUIRE(restored.stateFor("enemy.slime").scanned);
    REQUIRE(restored.stateFor("enemy.slime").defeated);
    REQUIRE(restored.completionRatio() == 1.0);
}

TEST_CASE("Bestiary validates missing enemy references", "[codex][simulation][ffs13]") {
    urpg::codex::BestiaryRegistry registry;
    registry.addEntry({"enemy.missing", "Missing", {}, {}, ""});

    const auto diagnostics = registry.validate({"enemy.slime"});
    REQUIRE_FALSE(diagnostics.empty());
    REQUIRE(diagnostics.front().code == "missing_enemy_reference");
}
