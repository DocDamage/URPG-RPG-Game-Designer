#include "engine/core/project/project_template_generator.h"
#include "engine/core/project/template_runtime_profile.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>

TEST_CASE("project template generator emits valid starter projects", "[project][onboarding][ffs08]") {
    urpg::project::ProjectTemplateGenerator generator;

    for (const auto& profile : urpg::project::allTemplateRuntimeProfiles()) {
        const std::string project_id = profile.id + "_starter";
        const auto result = generator.generate({profile.id, project_id, profile.id + " Game"});

        REQUIRE(result.success);
        REQUIRE(result.errors.empty());
        REQUIRE(result.project["template_id"] == profile.id);
        REQUIRE(result.project["subsystems"].contains("maps"));
        REQUIRE(result.project["subsystems"].contains("menu"));
        REQUIRE(result.project["subsystems"].contains("message"));
        REQUIRE(result.project["subsystems"].contains("battle"));
        REQUIRE(result.project["subsystems"].contains("save"));
        REQUIRE(result.project["subsystems"].contains("localization"));
        REQUIRE(result.project["subsystems"].contains("input"));
        REQUIRE(result.project["subsystems"].contains("export_profile"));
        REQUIRE(result.project["subsystems"].contains("template_runtime"));
        REQUIRE(result.project["loops"].is_array());
        REQUIRE_FALSE(result.project["loops"].empty());
        REQUIRE(result.project["template_bars"]["accessibility"]["status"] == "READY");
        REQUIRE(result.project["template_bars"]["audio"]["status"] == "READY");
        REQUIRE(result.project["template_bars"]["input"]["status"] == "READY");
        REQUIRE(result.project["template_bars"]["localization"]["status"] == "READY");
        REQUIRE(result.project["template_bars"]["performance"]["status"] == "READY");
        REQUIRE(result.audit_report["status"] == "passed");
        REQUIRE(generator.validateProjectDocument(result.project).empty());
    }
}

TEST_CASE("template runtime profiles implement advanced template-specific systems",
          "[project][template_profile]") {
    for (const auto& profile : urpg::project::allTemplateRuntimeProfiles()) {
        REQUIRE(urpg::project::validateTemplateRuntimeProfile(profile).empty());
    }

    const auto tactics = urpg::project::findTemplateRuntimeProfile("tactics_rpg");
    REQUIRE(tactics.has_value());
    REQUIRE(tactics->systems.contains("scenarioAuthoring"));
    REQUIRE(tactics->systems["scenarioAuthoring"].contains("deploymentZones"));

    const auto monster = urpg::project::findTemplateRuntimeProfile("monster_collector_rpg");
    REQUIRE(monster.has_value());
    REQUIRE(monster->systems["collection"].contains("species"));
    REQUIRE(monster->systems["collection"].contains("capture"));

    const auto cozy = urpg::project::findTemplateRuntimeProfile("cozy_life_rpg");
    REQUIRE(cozy.has_value());
    REQUIRE(cozy->systems.contains("schedule"));
    REQUIRE(cozy->systems.contains("crafting"));
    REQUIRE(cozy->systems.contains("economy"));

    const auto metroidvania = urpg::project::findTemplateRuntimeProfile("metroidvania_lite");
    REQUIRE(metroidvania.has_value());
    REQUIRE(metroidvania->systems["traversal"].contains("abilities"));
    REQUIRE(metroidvania->systems["traversal"].contains("regions"));

    const auto spatial = urpg::project::findTemplateRuntimeProfile("2_5d_rpg");
    REQUIRE(spatial.has_value());
    REQUIRE(spatial->systems["raycast"].contains("authoringAdapter"));
    REQUIRE(spatial->systems["raycast"].contains("exportValidation"));

    const auto roguelite = urpg::project::findTemplateRuntimeProfile("roguelite_dungeon");
    REQUIRE(roguelite.has_value());
    REQUIRE(roguelite->systems.contains("proceduralDungeon"));
    REQUIRE(roguelite->systems["loot"].contains("rewardPity"));

    const auto horror = urpg::project::findTemplateRuntimeProfile("survival_horror_rpg");
    REQUIRE(horror.has_value());
    REQUIRE(horror->systems.contains("tension"));
    REQUIRE(horror->systems.contains("puzzles"));

    const auto farming = urpg::project::findTemplateRuntimeProfile("farming_adventure_rpg");
    REQUIRE(farming.has_value());
    REQUIRE(farming->systems.contains("farming"));
    REQUIRE(farming->systems.contains("adventure"));

    const auto cardBattler = urpg::project::findTemplateRuntimeProfile("card_battler_rpg");
    REQUIRE(cardBattler.has_value());
    REQUIRE(cardBattler->systems.contains("cards"));
    REQUIRE(cardBattler->systems["battle"].contains("visibleIntent"));

    const auto platformer = urpg::project::findTemplateRuntimeProfile("platformer_rpg");
    REQUIRE(platformer.has_value());
    REQUIRE(platformer->systems.contains("platformer"));
    REQUIRE(platformer->systems["combat"].contains("actions"));

    const auto gacha = urpg::project::findTemplateRuntimeProfile("gacha_hero_rpg");
    REQUIRE(gacha.has_value());
    REQUIRE(gacha->systems.contains("summon"));
    REQUIRE(gacha->systems["heroes"].contains("starterRoster"));

    const auto mystery = urpg::project::findTemplateRuntimeProfile("mystery_detective_rpg");
    REQUIRE(mystery.has_value());
    REQUIRE(mystery->systems.contains("casework"));
    REQUIRE(mystery->systems["dialogue"].contains("interrogationModes"));

    const auto world = urpg::project::findTemplateRuntimeProfile("world_exploration_rpg");
    REQUIRE(world.has_value());
    REQUIRE(world->systems.contains("world"));
    REQUIRE(world->systems["quests"].contains("objectiveRadar"));
}

TEST_CASE("project template generator rejects duplicate project ids", "[project][onboarding][ffs08]") {
    urpg::project::ProjectTemplateGenerator generator;

    REQUIRE(generator.generate({"jrpg", "duplicate_project", "First"}).success);
    const auto duplicate = generator.generate({"jrpg", "duplicate_project", "Second"});

    REQUIRE_FALSE(duplicate.success);
    REQUIRE(duplicate.errors == std::vector<std::string>{"duplicate_project_id:duplicate_project"});
}

TEST_CASE("project template generator reports missing subsystem schema violations", "[project][onboarding][ffs08]") {
    urpg::project::ProjectTemplateGenerator generator;
    auto result = generator.generate({"jrpg", "schema_check", "Schema Check"});
    REQUIRE(result.success);
    result.project["subsystems"].erase("save");

    const auto errors = generator.validateProjectDocument(result.project);

    REQUIRE(std::find(errors.begin(), errors.end(), "missing_subsystem:save") != errors.end());
}
