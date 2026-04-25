#include "engine/core/project/project_template_generator.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>

TEST_CASE("project template generator emits valid starter projects", "[project][onboarding][ffs08]") {
    urpg::project::ProjectTemplateGenerator generator;

    for (const auto* template_id : {"jrpg", "visual_novel", "turn_based_rpg"}) {
        const std::string project_id = std::string(template_id) + "_starter";
        const auto result = generator.generate({template_id, project_id, std::string(template_id) + " Game"});

        REQUIRE(result.success);
        REQUIRE(result.errors.empty());
        REQUIRE(result.project["template_id"] == template_id);
        REQUIRE(result.project["subsystems"].contains("maps"));
        REQUIRE(result.project["subsystems"].contains("menu"));
        REQUIRE(result.project["subsystems"].contains("message"));
        REQUIRE(result.project["subsystems"].contains("battle"));
        REQUIRE(result.project["subsystems"].contains("save"));
        REQUIRE(result.project["subsystems"].contains("localization"));
        REQUIRE(result.project["subsystems"].contains("input"));
        REQUIRE(result.project["subsystems"].contains("export_profile"));
        REQUIRE(result.audit_report["status"] == "passed");
        REQUIRE(generator.validateProjectDocument(result.project).empty());
    }
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
