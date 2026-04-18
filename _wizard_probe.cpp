#include <iostream>
#include <nlohmann/json.hpp>
#include "editor/diagnostics/migration_wizard_model.h"

int main() {
    urpg::editor::MigrationWizardModel model;
    nlohmann::json project_data = {
        {"scenes", {
            {{"symbol", "item"}, {"name", "Items"}}
        }}
    };
    model.runFullMigration(project_data);
    std::cout << "before complete=" << model.getReport().is_complete
              << " count=" << model.getReport().subsystem_results.size()
              << " logs=" << model.getReport().summary_logs.size() << "\n";
    std::cout << "clear result=" << model.clearSubsystemResult("menu") << "\n";
    std::cout << "after complete=" << model.getReport().is_complete
              << " count=" << model.getReport().subsystem_results.size()
              << " logs=" << model.getReport().summary_logs.size()
              << " total=" << model.getReport().total_files_processed << "\n";
}
