#include <catch2/catch_test_macros.hpp>

#include "engine/core/ability/pattern_field.h"
#include "engine/core/ability/pattern_field_serializer.h"

using namespace urpg;

TEST_CASE("Pattern Field JSON serialization", "[pattern]") {
    // Create a pattern
    PatternField original("Fireball AoE");
    original.addPoint(0, 0);
    original.addPoint(1, 0);
    original.addPoint(-1, 0);
    original.addPoint(0, 1);
    original.addPoint(0, -1);

    // Serialize to JSON
    nlohmann::json j = PatternFieldSerializer::toJson(original);

    // Deserialize from JSON
    PatternField restored = PatternFieldSerializer::fromJson(j);

    // Verify
    REQUIRE(restored.getName() == "Fireball AoE");
    REQUIRE(restored.getPoints().size() == 5);
    REQUIRE(restored.hasPoint(0, 0));
    REQUIRE(restored.hasPoint(1, 0));
    REQUIRE(restored.hasPoint(-1, 0));
    REQUIRE(restored.hasPoint(0, 1));
    REQUIRE(restored.hasPoint(0, -1));
    REQUIRE_FALSE(restored.hasPoint(1, 1));
}
