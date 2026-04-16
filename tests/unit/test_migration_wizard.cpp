#include <catch2/catch_test_macros.hpp>
#include "editor/diagnostics/migration_wizard_model.h"
#include <nlohmann/json.hpp>

using namespace urpg::editor;

TEST_CASE("MigrationWizardModel: Batch Orchestration", "[editor][diagnostics][wizard]") {
    MigrationWizardModel model;
    
    nlohmann::json project_data = {
        {"scenes", {
            {{"symbol", "item"}, {"name", "Items"}}
        }},
        {"troops", {
            {{"id", 1}, {"name", "Slime x2"}, {"members", {}}}
        }}
    };

    SECTION("Initial state") {
        auto report = model.getReport();
        REQUIRE(report.total_files_processed == 0);
        REQUIRE_FALSE(report.is_complete);
    }

    SECTION("Run full migration collects counts from subsystems") {
        model.runFullMigration(project_data);
        auto report = model.getReport();
        
        REQUIRE(report.is_complete);
        REQUIRE(report.total_files_processed == 2); // 1 for scenes/menus, 1 for troops
        REQUIRE(report.summary_logs.size() > 0);
    }
}
