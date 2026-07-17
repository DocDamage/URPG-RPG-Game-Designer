#include "engine/core/project/project_document_migration_service.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace {

class MigrationProjectRoot {
public:
    MigrationProjectRoot() {
        root_ = std::filesystem::temp_directory_path() /
                ("urpg_document_migration_" +
                 std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        std::filesystem::create_directories(root_);
    }
    ~MigrationProjectRoot() {
        std::error_code error;
        std::filesystem::remove_all(root_, error);
    }
    const std::filesystem::path& root() const { return root_; }
    void write(const std::filesystem::path& relative, const nlohmann::json& value) const {
        std::filesystem::create_directories((root_ / relative).parent_path());
        std::ofstream output(root_ / relative, std::ios::binary);
        output << value.dump(2) << '\n';
    }
    nlohmann::json read(const std::filesystem::path& relative) const {
        std::ifstream input(root_ / relative, std::ios::binary);
        return nlohmann::json::parse(input);
    }
private:
    std::filesystem::path root_;
};

void writeLegacyDocuments(const MigrationProjectRoot& project) {
    project.write("content/abilities/fire.json", {{"ability_id", "fire"}});
    project.write("content/characters/hero.json", {{"character_id", "hero"}});
    project.write("content/database.json", {{"records", nlohmann::json::array()}});
    project.write("content/vendors/shop.json", {{"entries", nlohmann::json::array()}});
    project.write("content/ui/menus.json", {{"scenes", nlohmann::json::array()}});
}

} // namespace

TEST_CASE("Project document migrations dry run then publish with durable originals",
          "[project][migration][disk]") {
    MigrationProjectRoot project;
    writeLegacyDocuments(project);

    const auto dryRun = urpg::project::migrateProjectDocumentsOnDisk(project.root(), true);
    REQUIRE(dryRun.success);
    REQUIRE(dryRun.code == "project_document_migration_dry_run_ready");
    REQUIRE(dryRun.records.size() == 5);
    REQUIRE_FALSE(project.read("content/abilities/fire.json").contains("schema"));

    const auto applied = urpg::project::migrateProjectDocumentsOnDisk(project.root());
    REQUIRE(applied.success);
    REQUIRE(applied.code == "project_document_migration_applied");
    REQUIRE(project.read("content/abilities/fire.json")["schema"] == "urpg.ability.v1");
    REQUIRE(project.read("content/characters/hero.json")["schemaVersion"] == "1.0.0");
    REQUIRE(project.read("content/database.json")["schema"] == "urpg.database.v1");
    REQUIRE(project.read("content/vendors/shop.json")["schema"] == "urpg.vendor_catalog.v1");
    REQUIRE(project.read("content/ui/menus.json")["schema"] == "urpg.menu_graph.v1");
    for (const auto& record : applied.records) {
        if (!record.changed) continue;
        REQUIRE_FALSE(record.backup_path.empty());
        REQUIRE(std::filesystem::is_regular_file(record.backup_path));
        std::ifstream backup(record.backup_path, std::ios::binary);
        const auto original = nlohmann::json::parse(backup);
        REQUIRE_FALSE(original.contains(record.document_type == "character_creator" ? "schemaVersion" : "schema"));
    }

    const auto reopened = urpg::project::migrateProjectDocumentsOnDisk(project.root());
    REQUIRE(reopened.success);
    REQUIRE(reopened.code == "project_document_migration_already_current");
}

TEST_CASE("Project document migration publication fault restores earlier owners for retry",
          "[project][migration][disk][fault][pcq506]") {
    MigrationProjectRoot project;
    writeLegacyDocuments(project);
    size_t publications = 0;
    const auto failed = urpg::project::migrateProjectDocumentsOnDisk(
        project.root(), false, [&](const auto&) { return ++publications != 2; });
    REQUIRE_FALSE(failed.success);
    REQUIRE(failed.code == "project_document_migration_publish_failed_rolled_back");
    REQUIRE_FALSE(project.read("content/abilities/fire.json").contains("schema"));
    REQUIRE_FALSE(project.read("content/characters/hero.json").contains("schemaVersion"));
    REQUIRE(urpg::project::migrateProjectDocumentsOnDisk(project.root()).success);
}
