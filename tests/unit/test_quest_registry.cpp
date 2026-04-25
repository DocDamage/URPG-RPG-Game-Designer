#include "engine/core/quest/quest_registry.h"
#include "engine/core/quest/quest_validator.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("quest objective advancement is deterministic and persists", "[quest][narrative][ffs10]") {
    urpg::quest::QuestRegistry registry;
    REQUIRE(registry.registerQuest({
        "intro",
        {{"meet_guide",
          urpg::quest::ObjectiveState::Locked,
          {{"switch", "intro_started", 1}, {"reputation", "guide", 10}},
          ""}},
    }));

    urpg::quest::QuestWorldState world;
    world.switches["intro_started"] = true;
    world.reputation["guide"] = 12;

    REQUIRE(registry.evaluateObjective("intro", "meet_guide", world, "2026-04-25T12:00:00Z"));
    const auto* quest = registry.findQuest("intro");
    REQUIRE(quest != nullptr);
    REQUIRE(quest->objectives[0].state == urpg::quest::ObjectiveState::Completed);

    const auto restored = urpg::quest::QuestRegistry::deserialize(registry.serialize());
    const auto* restored_quest = restored.findQuest("intro");
    REQUIRE(restored_quest != nullptr);
    REQUIRE(restored_quest->objectives[0].state == urpg::quest::ObjectiveState::Completed);
    REQUIRE(restored_quest->objectives[0].updated_at == "2026-04-25T12:00:00Z");
}

TEST_CASE("quest validator reports deleted item references", "[quest][narrative][ffs10]") {
    const urpg::quest::QuestDefinition quest{
        "fetch",
        {{"find_relic", urpg::quest::ObjectiveState::Locked, {{"item", "deleted_relic", 1}}, ""}},
    };

    const urpg::quest::QuestValidator validator;
    const auto diagnostics = validator.validate(quest, {"potion"});

    REQUIRE(diagnostics == std::vector<std::string>{"missing_item_reference:deleted_relic"});
}
