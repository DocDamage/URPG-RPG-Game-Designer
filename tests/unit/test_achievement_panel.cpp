#include "editor/achievement/achievement_panel.h"
#include "engine/core/achievement/achievement_registry.h"

#include <catch2/catch_test_macros.hpp>

using namespace urpg::editor;
using namespace urpg::achievement;

TEST_CASE("AchievementPanel empty snapshot when no registry bound", "[achievement][editor][panel]") {
    AchievementPanel panel;
    panel.render();

    auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot.is_object());
    REQUIRE(snapshot.empty());
}

TEST_CASE("AchievementPanel snapshot reflects achievements after bind", "[achievement][editor][panel]") {
    AchievementRegistry registry;

    AchievementDef def1;
    def1.id = "ach_001";
    def1.title = "First Blood";
    def1.description = "Defeat one enemy.";
    def1.secret = false;
    def1.unlockCondition = "kill_count_1";
    def1.iconId = "icon_sword";

    AchievementDef def2;
    def2.id = "ach_010";
    def2.title = "Decimator";
    def2.description = "Defeat ten enemies.";
    def2.secret = true;
    def2.unlockCondition = "kill_count_10";
    def2.iconId = "icon_axe";

    registry.registerAchievement(def1);
    registry.registerAchievement(def2);

    AchievementPanel panel;
    panel.bindRegistry(&registry);
    panel.render();

    auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot.contains("total_count"));
    REQUIRE(snapshot["total_count"] == 2);
    REQUIRE(snapshot.contains("unlocked_count"));
    REQUIRE(snapshot.contains("trophy_export"));
    REQUIRE(snapshot["trophy_export"]["total"] == 2);
    REQUIRE(snapshot["trophy_export"]["unlocked"] == 0);
    REQUIRE(snapshot["trophy_export"]["secret"] == 1);
    REQUIRE(snapshot.contains("achievements"));
    REQUIRE(snapshot["achievements"].is_array());
    REQUIRE(snapshot["achievements"].size() == 2);

    REQUIRE(snapshot["achievements"][0]["title"] == "First Blood");
    REQUIRE(snapshot["achievements"][0]["unlocked"] == false);
    REQUIRE(snapshot["achievements"][1]["title"] == "Decimator");
    REQUIRE(snapshot["achievements"][1]["unlocked"] == false);
}

TEST_CASE("AchievementPanel unlocked count is correct", "[achievement][editor][panel]") {
    AchievementRegistry registry;

    AchievementDef def1;
    def1.id = "ach_001";
    def1.title = "First Step";
    def1.description = "Take one step.";
    def1.secret = false;
    def1.unlockCondition = "step_count_1";
    def1.iconId = "icon_boot";

    AchievementDef def2;
    def2.id = "ach_002";
    def2.title = "Second Step";
    def2.description = "Take two steps.";
    def2.secret = false;
    def2.unlockCondition = "step_count_2";
    def2.iconId = "icon_boots";

    registry.registerAchievement(def1);
    registry.registerAchievement(def2);

    registry.reportProgress("ach_001", 1);

    AchievementPanel panel;
    panel.bindRegistry(&registry);
    panel.render();

    auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["total_count"] == 2);
    REQUIRE(snapshot["unlocked_count"] == 1);
    REQUIRE(snapshot["trophy_export"]["total"] == 2);
    REQUIRE(snapshot["trophy_export"]["unlocked"] == 1);
    REQUIRE(snapshot["achievements"][0]["unlocked"] == true);
    REQUIRE(snapshot["achievements"][1]["unlocked"] == false);
}
