#include "editor/project/project_recovery_coordinator.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>

TEST_CASE("Project recovery fault injection covers every primary document and failure class",
          "[editor][recovery][pcq506]") {
    using namespace urpg::editor;
    std::vector<ProjectRecoveryFault> faults;
    for (const auto& document : primaryRecoveryDocumentTypes()) {
        faults.push_back({"document-" + document, ProjectRecoveryClass::PrimaryDocument, document,
                          true, true, true, "simulated_document_crash"});
    }
    faults.push_back({"asset-job", ProjectRecoveryClass::AssetJob, "thumbnail-job", true, true, false,
                      "simulated_worker_exit"});
    faults.push_back({"external-change", ProjectRecoveryClass::ExternalChangeConflict, "map", true, false, true,
                      "simulated_disk_edit"});
    faults.push_back({"layout", ProjectRecoveryClass::LayoutOrSettings, "workspace-layout", true, false, false,
                      "simulated_corrupt_layout"});
    faults.push_back({"migration", ProjectRecoveryClass::Migration, "quest-schema", true, true, false,
                      "simulated_migration_failure"});
    faults.push_back({"package", ProjectRecoveryClass::Package, "windows-package", true, false, false,
                      "simulated_packager_failure"});

    const auto coverage = ProjectRecoveryCoordinator{}.evaluate(faults);
    REQUIRE(coverage.complete);
    REQUIRE(coverage.code == "project_recovery_coverage_complete");
    REQUIRE(coverage.missing_primary_documents.empty());
    REQUIRE(coverage.actions.size() == primaryRecoveryDocumentTypes().size() + 5);
    for (const auto& action : coverage.actions) REQUIRE(action.project_data_preserved);

    const auto find = [&](const std::string& id) -> const ProjectRecoveryAction& {
        const auto row = std::find_if(coverage.actions.begin(), coverage.actions.end(),
                                      [&](const auto& action) { return action.fault_id == id; });
        REQUIRE(row != coverage.actions.end());
        return *row;
    };
    REQUIRE(find("asset-job").disposition == ProjectRecoveryDisposition::ResumeOrRetry);
    REQUIRE(find("external-change").disposition == ProjectRecoveryDisposition::RequireUserResolution);
    REQUIRE(find("layout").disposition == ProjectRecoveryDisposition::ResetSafeDefaults);
    REQUIRE(find("layout").automatic);
    REQUIRE(find("migration").disposition == ProjectRecoveryDisposition::RestoreMigrationBackup);
    REQUIRE(find("package").disposition == ProjectRecoveryDisposition::PreserveSourcesAndRetry);
}

TEST_CASE("Project recovery never silently overwrites documents or unresolved conflicts",
          "[editor][recovery][pcq506]") {
    using namespace urpg::editor;
    std::vector<ProjectRecoveryFault> faults;
    for (const auto& document : primaryRecoveryDocumentTypes()) {
        faults.push_back({"doc-" + document, ProjectRecoveryClass::PrimaryDocument, document, true, false, true, {}});
    }
    faults.push_back({"asset", ProjectRecoveryClass::AssetJob, "import", true, false, false, {}});
    faults.push_back({"conflict", ProjectRecoveryClass::ExternalChangeConflict, "dialogue", true, false, true, {}});
    faults.push_back({"settings", ProjectRecoveryClass::LayoutOrSettings, "settings", true, false, false, {}});
    faults.push_back({"migration", ProjectRecoveryClass::Migration, "save", true, false, false, {}});
    faults.push_back({"package", ProjectRecoveryClass::Package, "linux", true, false, false, {}});
    const auto coverage = ProjectRecoveryCoordinator{}.evaluate(faults);
    REQUIRE(coverage.complete);
    for (const auto& action : coverage.actions) {
        if (action.recovery_class == ProjectRecoveryClass::PrimaryDocument ||
            action.recovery_class == ProjectRecoveryClass::ExternalChangeConflict) {
            REQUIRE(action.requires_user_action);
            REQUIRE_FALSE(action.automatic);
        }
    }
    const auto migration = std::find_if(coverage.actions.begin(), coverage.actions.end(),
        [](const auto& action) { return action.fault_id == "migration"; });
    REQUIRE(migration->disposition == ProjectRecoveryDisposition::PreserveFailure);
    REQUIRE(migration->requires_user_action);
}

TEST_CASE("Project recovery reports incomplete or ambiguous fault matrices", "[editor][recovery][pcq506]") {
    using namespace urpg::editor;
    const auto coverage = ProjectRecoveryCoordinator{}.evaluate({
        {"duplicate", ProjectRecoveryClass::PrimaryDocument, "map", true, true, false, {}},
        {"duplicate", ProjectRecoveryClass::Package, "package", true, false, false, {}}});
    REQUIRE_FALSE(coverage.complete);
    REQUIRE(coverage.code == "project_recovery_coverage_incomplete");
    REQUIRE(coverage.missing_primary_documents.size() == primaryRecoveryDocumentTypes().size() - 1);
    REQUIRE(coverage.diagnostics.size() == 6);
    REQUIRE(coverage.actions.size() == 1);
}
