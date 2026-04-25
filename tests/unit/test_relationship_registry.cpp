#include "engine/core/relationship/relationship_registry.h"

#include <catch2/catch_test_macros.hpp>

#include <vector>

TEST_CASE("relationship registry persists affinity and lists reputation gated content", "[relationship][narrative][ffs10]") {
    urpg::relationship::RelationshipRegistry registry;
    registry.setAffinity("guide", 150);
    registry.addGate({"guide_sidequest", "guide", 20});
    registry.addGate({"guide_secret", "guide", 120});

    REQUIRE(registry.affinity("guide") == 100);
    REQUIRE(registry.availableContent() == std::vector<std::string>{"guide_sidequest"});

    const auto restored = urpg::relationship::RelationshipRegistry::deserialize(registry.serialize());
    REQUIRE(restored.affinity("guide") == 100);
    REQUIRE(restored.availableContent() == std::vector<std::string>{"guide_sidequest"});
}
