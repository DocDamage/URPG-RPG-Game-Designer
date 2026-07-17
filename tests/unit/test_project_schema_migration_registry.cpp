#include "engine/core/project/project_schema_migration_registry.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <set>

using urpg::project::ProjectSchemaMigrationRegistry;
using urpg::project::ProjectSchemaMigrationStep;

namespace {

ProjectSchemaMigrationRegistry registry() {
    ProjectSchemaMigrationRegistry result;
    REQUIRE(result.registerStep(ProjectSchemaMigrationStep{
        "dialogue", "schema_version", "dialogue.v1", "dialogue.v2",
        {{"from", "dialogue.v1"}, {"to", "dialogue.v2"},
         {"ops", {{{"op", "rename"}, {"fromPath", "/speaker"}, {"toPath", "/speaker_id"}}}}}}));
    REQUIRE(result.registerStep(ProjectSchemaMigrationStep{
        "dialogue", "schema_version", "dialogue.v2", "dialogue.v3",
        {{"from", "dialogue.v2"}, {"to", "dialogue.v3"},
         {"ops", {{{"op", "set"}, {"path", "/caption_enabled"}, {"value", true}}}}}}));
    return result;
}

} // namespace

TEST_CASE("Project schema migration dry run applies full forward chain only to copy",
          "[project][schema_migration]") {
    auto document = nlohmann::json{{"schema_version", "dialogue.v1"}, {"speaker", "guide"}};
    const auto original = document;
    const auto result = registry().migrate("dialogue", "dialogue.v3", document, true);
    REQUIRE(result.success);
    REQUIRE(result.dry_run);
    REQUIRE(result.changed);
    REQUIRE(result.applied_versions == std::vector<std::string>{"dialogue.v2", "dialogue.v3"});
    REQUIRE(document == original);
    REQUIRE(result.backup == original);
    REQUIRE(result.migrated["schema_version"] == "dialogue.v3");
    REQUIRE(result.migrated["speaker_id"] == "guide");
    REQUIRE(result.migrated["caption_enabled"] == true);
}

TEST_CASE("Project schema migration commits atomically is idempotent and restores backup",
          "[project][schema_migration]") {
    auto document = nlohmann::json{{"schema_version", "dialogue.v1"}, {"speaker", "guide"}};
    const auto original = document;
    const auto applied = registry().migrate("dialogue", "dialogue.v3", document, false);
    REQUIRE(applied.success);
    REQUIRE(document == applied.migrated);
    const auto current = registry().migrate("dialogue", "dialogue.v3", document, false);
    REQUIRE(current.success);
    REQUIRE_FALSE(current.changed);
    REQUIRE(current.code == "project_schema_migration_already_current");
    REQUIRE(registry().restoreBackup(applied, document));
    REQUIRE(document == original);
}

TEST_CASE("Project schema migration failure leaves source document unchanged", "[project][schema_migration]") {
    auto document = nlohmann::json{{"schema_version", "dialogue.v1"}, {"different", "guide"}};
    const auto original = document;
    const auto failed = registry().migrate("dialogue", "dialogue.v3", document, false);
    REQUIRE_FALSE(failed.success);
    REQUIRE(failed.code == "project_schema_migration_step_failed");
    REQUIRE(document == original);
    REQUIRE(failed.backup == original);
}

TEST_CASE("Built-in project migration corpus adopts versions without losing legacy fields",
          "[project][schema_migration][corpus]") {
    std::ifstream input(std::filesystem::path(URPG_SOURCE_DIR) / "content" / "fixtures" /
                        "project_document_migration_corpus.json");
    const auto corpus = nlohmann::json::parse(input);
    REQUIRE(corpus["documents"].size() == 14);

    ProjectSchemaMigrationRegistry builtIns;
    std::string diagnostic;
    REQUIRE(urpg::project::registerBuiltInProjectSchemaMigrations(builtIns, &diagnostic));
    REQUIRE(diagnostic.empty());
    const std::set<std::string> migratedTypes = {
        "ability", "character_creator", "database", "vendor_catalog", "menu_studio"};

    for (const auto& fixture : corpus["documents"]) {
        const auto type = fixture.at("type").get<std::string>();
        auto document = fixture.at("document");
        const auto original = document;
        if (!migratedTypes.contains(type)) {
            REQUIRE(fixture.at("oldest") == fixture.at("current"));
            REQUIRE(document.at(fixture.at("versionField").get<std::string>()) == fixture.at("current"));
            continue;
        }
        const auto dryRun = builtIns.migrate(type, fixture.at("current").get<std::string>(), document, true);
        REQUIRE(dryRun.success);
        REQUIRE(dryRun.changed);
        REQUIRE(document == original);
        const auto applied = builtIns.migrate(type, fixture.at("current").get<std::string>(), document, false);
        REQUIRE(applied.success);
        REQUIRE(document.at(fixture.at("versionField").get<std::string>()) == fixture.at("current"));
        REQUIRE(document.at("fixture_marker") == original.at("fixture_marker"));
        const auto idempotent = builtIns.migrate(type, fixture.at("current").get<std::string>(), document, false);
        REQUIRE(idempotent.success);
        REQUIRE_FALSE(idempotent.changed);
        REQUIRE(builtIns.restoreBackup(applied, document));
        REQUIRE(document == original);
    }
}
