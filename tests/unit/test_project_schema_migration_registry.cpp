#include "engine/core/project/project_schema_migration_registry.h"

#include <catch2/catch_test_macros.hpp>

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
