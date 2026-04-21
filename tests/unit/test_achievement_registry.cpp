#include "engine/core/achievement/achievement_registry.h"

#include <catch2/catch_test_macros.hpp>

using namespace urpg::achievement;

TEST_CASE("AchievementRegistry register and retrieve", "[achievement]") {
    AchievementRegistry registry;

    AchievementDef def;
    def.id = "ach_001";
    def.title = "First Blood";
    def.description = "Defeat your first enemy.";
    def.secret = false;
    def.unlockCondition = "kill_count_1";
    def.iconId = "icon_sword";

    registry.registerAchievement(def);

    auto retrieved = registry.getAchievement("ach_001");
    REQUIRE(retrieved.has_value());
    REQUIRE(retrieved->id == "ach_001");
    REQUIRE(retrieved->title == "First Blood");

    auto all = registry.getAllAchievements();
    REQUIRE(all.size() == 1);
    REQUIRE(all[0].id == "ach_001");
}

TEST_CASE("AchievementRegistry progress increment unlocks at target", "[achievement]") {
    AchievementRegistry registry;

    AchievementDef def;
    def.id = "ach_010";
    def.title = "Decimator";
    def.description = "Defeat ten enemies.";
    def.secret = false;
    def.unlockCondition = "kill_count_10";
    def.iconId = "icon_axe";

    registry.registerAchievement(def);

    REQUIRE_FALSE(registry.reportProgress("ach_010", 5));
    auto progress = registry.getProgress("ach_010");
    REQUIRE(progress.has_value());
    REQUIRE(progress->current == 5);
    REQUIRE_FALSE(progress->unlocked);

    REQUIRE(registry.reportProgress("ach_010", 5));
    progress = registry.getProgress("ach_010");
    REQUIRE(progress->current == 10);
    REQUIRE(progress->unlocked);
    REQUIRE(progress->unlockTime.has_value());
    REQUIRE(progress->unlockTime.value() == "deterministic_timestamp");
}

TEST_CASE("AchievementRegistry double-unlock returns false second time", "[achievement]") {
    AchievementRegistry registry;

    AchievementDef def;
    def.id = "ach_001";
    def.title = "First Step";
    def.description = "Take a step.";
    def.secret = false;
    def.unlockCondition = "step_count_1";
    def.iconId = "icon_boot";

    registry.registerAchievement(def);

    REQUIRE(registry.reportProgress("ach_001", 1));
    REQUIRE_FALSE(registry.reportProgress("ach_001", 1));
}

TEST_CASE("AchievementRegistry reset clears progress", "[achievement]") {
    AchievementRegistry registry;

    AchievementDef def;
    def.id = "ach_005";
    def.title = "Walker";
    def.description = "Walk five steps.";
    def.secret = false;
    def.unlockCondition = "step_count_5";
    def.iconId = "icon_walk";

    registry.registerAchievement(def);
    registry.reportProgress("ach_005", 5);

    auto progress = registry.getProgress("ach_005");
    REQUIRE(progress->unlocked);

    registry.resetProgress("ach_005");
    progress = registry.getProgress("ach_005");
    REQUIRE(progress->current == 0);
    REQUIRE_FALSE(progress->unlocked);
    REQUIRE_FALSE(progress->unlockTime.has_value());
}

TEST_CASE("AchievementRegistry save/load round-trip", "[achievement]") {
    AchievementRegistry registry;

    AchievementDef def;
    def.id = "ach_003";
    def.title = "Collector";
    def.description = "Collect three items.";
    def.secret = false;
    def.unlockCondition = "collect_count_3";
    def.iconId = "icon_bag";

    registry.registerAchievement(def);
    registry.reportProgress("ach_003", 2);

    auto json = registry.saveToJson();
    REQUIRE(json["version"] == "1.0.0");
    REQUIRE(json["progress"].is_array());
    REQUIRE(json["progress"].size() == 1);

    AchievementRegistry loadedRegistry;
    loadedRegistry.registerAchievement(def);
    loadedRegistry.loadFromJson(json);

    auto progress = loadedRegistry.getProgress("ach_003");
    REQUIRE(progress.has_value());
    REQUIRE(progress->current == 2);
    REQUIRE(progress->target == 3);
    REQUIRE_FALSE(progress->unlocked);
}

TEST_CASE("AchievementRegistry unknown id in load is ignored", "[achievement]") {
    AchievementRegistry registry;

    AchievementDef def;
    def.id = "ach_known";
    def.title = "Known";
    def.description = "A known achievement.";
    def.secret = false;
    def.unlockCondition = "count_1";
    def.iconId = "icon_known";

    registry.registerAchievement(def);

    nlohmann::json json = {
        {"version", "1.0.0"},
        {"progress", nlohmann::json::array({
            {
                {"id", "ach_known"},
                {"current", 1},
                {"target", 1},
                {"unlocked", true},
                {"unlockTime", "deterministic_timestamp"}
            },
            {
                {"id", "ach_unknown"},
                {"current", 5},
                {"target", 10},
                {"unlocked", false}
            }
        })}
    };

    registry.loadFromJson(json);

    auto progress = registry.getProgress("ach_known");
    REQUIRE(progress.has_value());
    REQUIRE(progress->unlocked);

    auto unknown = registry.getProgress("ach_unknown");
    REQUIRE_FALSE(unknown.has_value());
}
