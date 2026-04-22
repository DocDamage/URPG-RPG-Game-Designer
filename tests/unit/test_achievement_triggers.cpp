#include "engine/core/achievement/achievement_trigger.h"
#include "engine/core/achievement/achievement_registry.h"

#include <catch2/catch_test_macros.hpp>

using namespace urpg::achievement;

TEST_CASE("AchievementTrigger parse legacy count format", "[achievement][trigger]") {
    auto trigger = AchievementTrigger::parse("kill_count_10");
    REQUIRE(trigger.eventType == "kill_count");
    REQUIRE(trigger.target == 10);
    REQUIRE(trigger.params.empty());
}

TEST_CASE("AchievementTrigger parse plain event without count", "[achievement][trigger]") {
    auto trigger = AchievementTrigger::parse("battle_victory");
    REQUIRE(trigger.eventType == "battle_victory");
    REQUIRE(trigger.target == 1);
    REQUIRE(trigger.params.empty());
}

TEST_CASE("AchievementTrigger parse parameterized format", "[achievement][trigger]") {
    auto trigger = AchievementTrigger::parse("item_collect:item_id=5:count=3");
    REQUIRE(trigger.eventType == "item_collect");
    REQUIRE(trigger.target == 3);
    REQUIRE(trigger.params.size() == 1);
    REQUIRE(trigger.params.at("item_id") == "5");
}

TEST_CASE("AchievementTrigger parse empty condition", "[achievement][trigger]") {
    auto trigger = AchievementTrigger::parse("");
    REQUIRE(trigger.eventType.empty());
    REQUIRE(trigger.target == 1);
}

TEST_CASE("AchievementTrigger matches identical event", "[achievement][trigger]") {
    auto trigger = AchievementTrigger::parse("kill_count_5");
    REQUIRE(trigger.matches("kill_count", {}));
}

TEST_CASE("AchievementTrigger does not match different event type", "[achievement][trigger]") {
    auto trigger = AchievementTrigger::parse("kill_count_5");
    REQUIRE_FALSE(trigger.matches("step_count", {}));
}

TEST_CASE("AchievementTrigger matches parameterized event", "[achievement][trigger]") {
    auto trigger = AchievementTrigger::parse("item_collect:item_id=5:count=3");
    std::unordered_map<std::string, std::string> params;
    params["item_id"] = "5";
    REQUIRE(trigger.matches("item_collect", params));
}

TEST_CASE("AchievementTrigger rejects event with wrong parameter", "[achievement][trigger]") {
    auto trigger = AchievementTrigger::parse("item_collect:item_id=5:count=3");
    std::unordered_map<std::string, std::string> params;
    params["item_id"] = "7";
    REQUIRE_FALSE(trigger.matches("item_collect", params));
}

TEST_CASE("AchievementTrigger rejects event missing required parameter", "[achievement][trigger]") {
    auto trigger = AchievementTrigger::parse("item_collect:item_id=5:count=3");
    REQUIRE_FALSE(trigger.matches("item_collect", {}));
}

TEST_CASE("AchievementEventBus emits to listeners", "[achievement][trigger]") {
    AchievementEventBus bus;
    int callCount = 0;
    std::string capturedType;

    bus.addListener([&](const AchievementEvent& event) {
        callCount++;
        capturedType = event.eventType;
    });

    bus.emit(AchievementEvent{"battle_victory", {}, 1});

    REQUIRE(callCount == 1);
    REQUIRE(capturedType == "battle_victory");
}

TEST_CASE("AchievementEventBus emits to multiple listeners", "[achievement][trigger]") {
    AchievementEventBus bus;
    int callCountA = 0;
    int callCountB = 0;

    bus.addListener([&](const AchievementEvent&) { callCountA++; });
    bus.addListener([&](const AchievementEvent&) { callCountB++; });

    bus.emit(AchievementEvent{"step_count", {}, 1});

    REQUIRE(callCountA == 1);
    REQUIRE(callCountB == 1);
}

TEST_CASE("AchievementTriggerResolver auto-reports progress on matching event", "[achievement][trigger]") {
    AchievementRegistry registry;
    AchievementEventBus bus;
    AchievementTriggerResolver resolver;

    AchievementDef def;
    def.id = "ach_kill_5";
    def.title = "Slayer";
    def.unlockCondition = "kill_count_5";
    def.iconId = "icon_sword";
    registry.registerAchievement(def);

    resolver.bindRegistry(&registry);
    resolver.bindEventBus(&bus);

    REQUIRE_FALSE(registry.getProgress("ach_kill_5")->unlocked);

    bus.emit(AchievementEvent{"kill_count", {}, 3});
    REQUIRE(registry.getProgress("ach_kill_5")->current == 3);
    REQUIRE_FALSE(registry.getProgress("ach_kill_5")->unlocked);

    bus.emit(AchievementEvent{"kill_count", {}, 2});
    REQUIRE(registry.getProgress("ach_kill_5")->current == 5);
    REQUIRE(registry.getProgress("ach_kill_5")->unlocked);
}

TEST_CASE("AchievementTriggerResolver ignores non-matching events", "[achievement][trigger]") {
    AchievementRegistry registry;
    AchievementEventBus bus;
    AchievementTriggerResolver resolver;

    AchievementDef def;
    def.id = "ach_step_10";
    def.title = "Walker";
    def.unlockCondition = "step_count_10";
    def.iconId = "icon_boot";
    registry.registerAchievement(def);

    resolver.bindRegistry(&registry);
    resolver.bindEventBus(&bus);

    bus.emit(AchievementEvent{"kill_count", {}, 5});
    REQUIRE(registry.getProgress("ach_step_10")->current == 0);
}

TEST_CASE("AchievementTriggerResolver handles parameterized item collect event", "[achievement][trigger]") {
    AchievementRegistry registry;
    AchievementEventBus bus;
    AchievementTriggerResolver resolver;

    AchievementDef def;
    def.id = "ach_collect_potion";
    def.title = "Potion Hoarder";
    def.unlockCondition = "item_collect:item_id=1:count=3";
    def.iconId = "icon_potion";
    registry.registerAchievement(def);

    resolver.bindRegistry(&registry);
    resolver.bindEventBus(&bus);

    // Wrong item ID
    std::unordered_map<std::string, std::string> wrongParams;
    wrongParams["item_id"] = "2";
    bus.emit(AchievementEvent{"item_collect", wrongParams, 1});
    REQUIRE(registry.getProgress("ach_collect_potion")->current == 0);

    // Correct item ID
    std::unordered_map<std::string, std::string> rightParams;
    rightParams["item_id"] = "1";
    bus.emit(AchievementEvent{"item_collect", rightParams, 3});
    REQUIRE(registry.getProgress("ach_collect_potion")->current == 3);
    REQUIRE(registry.getProgress("ach_collect_potion")->unlocked);
}

TEST_CASE("AchievementTriggerResolver does not unlock parameterized target early", "[achievement][trigger]") {
    AchievementRegistry registry;
    AchievementEventBus bus;
    AchievementTriggerResolver resolver;

    AchievementDef def;
    def.id = "ach_collect_potion";
    def.title = "Potion Hoarder";
    def.unlockCondition = "item_collect:item_id=1:count=3";
    def.iconId = "icon_potion";
    registry.registerAchievement(def);

    resolver.bindRegistry(&registry);
    resolver.bindEventBus(&bus);

    std::unordered_map<std::string, std::string> params;
    params["item_id"] = "1";

    bus.emit(AchievementEvent{"item_collect", params, 1});
    auto progress = registry.getProgress("ach_collect_potion");
    REQUIRE(progress.has_value());
    REQUIRE(progress->target == 3);
    REQUIRE(progress->current == 1);
    REQUIRE_FALSE(progress->unlocked);

    bus.emit(AchievementEvent{"item_collect", params, 2});
    progress = registry.getProgress("ach_collect_potion");
    REQUIRE(progress->current == 3);
    REQUIRE(progress->unlocked);
}

TEST_CASE("AchievementTriggerResolver handles multiple achievements", "[achievement][trigger]") {
    AchievementRegistry registry;
    AchievementEventBus bus;
    AchievementTriggerResolver resolver;

    AchievementDef def1;
    def1.id = "ach_kill_3";
    def1.title = "Killer";
    def1.unlockCondition = "kill_count_3";
    def1.iconId = "icon_sword";
    registry.registerAchievement(def1);

    AchievementDef def2;
    def2.id = "ach_kill_5";
    def2.title = "Slayer";
    def2.unlockCondition = "kill_count_5";
    def2.iconId = "icon_axe";
    registry.registerAchievement(def2);

    resolver.bindRegistry(&registry);
    resolver.bindEventBus(&bus);

    bus.emit(AchievementEvent{"kill_count", {}, 3});
    REQUIRE(registry.getProgress("ach_kill_3")->unlocked);
    REQUIRE_FALSE(registry.getProgress("ach_kill_5")->unlocked);

    bus.emit(AchievementEvent{"kill_count", {}, 2});
    REQUIRE(registry.getProgress("ach_kill_5")->unlocked);
}

TEST_CASE("AchievementTriggerResolver refreshAll fills short progress for unlocked achievements", "[achievement][trigger]") {
    AchievementRegistry registry;
    AchievementEventBus bus;
    AchievementTriggerResolver resolver;

    AchievementDef def;
    def.id = "ach_test";
    def.title = "Test";
    def.unlockCondition = "kill_count_5";
    def.iconId = "icon_test";
    registry.registerAchievement(def);

    resolver.bindRegistry(&registry);
    resolver.bindEventBus(&bus);

    // Simulate manual progress short of target
    registry.reportProgress("ach_test", 2);
    REQUIRE(registry.getProgress("ach_test")->current == 2);
    REQUIRE_FALSE(registry.getProgress("ach_test")->unlocked);

    // refreshAll should not change anything since not unlocked
    resolver.refreshAll();
    REQUIRE(registry.getProgress("ach_test")->current == 2);
}

TEST_CASE("AchievementTriggerResolver rebinding same bus does not double count events", "[achievement][trigger]") {
    AchievementRegistry registry;
    AchievementEventBus bus;
    AchievementTriggerResolver resolver;

    AchievementDef def;
    def.id = "ach_step_2";
    def.title = "Walker";
    def.unlockCondition = "step_count_2";
    def.iconId = "icon_boot";
    registry.registerAchievement(def);

    resolver.bindRegistry(&registry);
    resolver.bindEventBus(&bus);
    resolver.bindEventBus(&bus);

    bus.emit(AchievementEvent{"step_count", {}, 1});

    auto progress = registry.getProgress("ach_step_2");
    REQUIRE(progress.has_value());
    REQUIRE(progress->current == 1);
    REQUIRE_FALSE(progress->unlocked);
}

TEST_CASE("AchievementTriggerResolver ignores events after resolver destruction", "[achievement][trigger]") {
    AchievementRegistry registry;
    AchievementEventBus bus;

    AchievementDef def;
    def.id = "ach_kill_1";
    def.title = "First Blood";
    def.unlockCondition = "kill_count_1";
    def.iconId = "icon_sword";
    registry.registerAchievement(def);

    {
        AchievementTriggerResolver resolver;
        resolver.bindRegistry(&registry);
        resolver.bindEventBus(&bus);
    }

    bus.emit(AchievementEvent{"kill_count", {}, 1});

    auto progress = registry.getProgress("ach_kill_1");
    REQUIRE(progress.has_value());
    REQUIRE(progress->current == 0);
    REQUIRE_FALSE(progress->unlocked);
}

TEST_CASE("AchievementDef explicit target overrides parsed target", "[achievement][trigger]") {
    AchievementRegistry registry;

    AchievementDef def;
    def.id = "ach_override";
    def.title = "Override Test";
    def.unlockCondition = "kill_count_999"; // Would parse to 999
    def.target = 5; // Override to 5
    def.iconId = "icon_test";
    registry.registerAchievement(def);

    auto progress = registry.getProgress("ach_override");
    REQUIRE(progress.has_value());
    REQUIRE(progress->target == 5);
}
