#include "engine/core/tutorial/runtime_hint_system.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Runtime hint dismissal persists and prevents repeated display", "[tutorial][simulation][ffs13]") {
    urpg::tutorial::RuntimeHintSystem hints;
    hints.registerHint({"hint.save", "tutorial.save", "opened_menu", true, true});

    REQUIRE(hints.nextHint({"opened_menu"}).id == "hint.save");
    hints.dismiss("hint.save");
    REQUIRE(hints.nextHint({"opened_menu"}).id.empty());

    const auto restored = urpg::tutorial::RuntimeHintSystem::fromJson(hints.toJson());
    REQUIRE(restored.nextHint({"opened_menu"}).id.empty());
}
