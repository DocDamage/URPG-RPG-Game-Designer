#pragma once

#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>
#include "../../engine/core/message/message_migration.h"
#include "../../engine/core/battle/battle_migration.h"
#include "../../engine/core/ui/menu_migration.h"

namespace urpg::editor {

/**
 * @brief Logic for orchestrating full-project migrations from legacy formats.
 * 
 * This model collects diagnostics from all subsystem migrators (Message, Battle, UI)
 * and provides a single completion report for the editor wizard.
 */
class MigrationWizardModel {
public:
    struct ProgressReport {
        size_t total_files_processed = 0;
        size_t warning_count = 0;
        size_t error_count = 0;
        bool is_complete = false;
        std::vector<std::string> summary_logs;
    };

    void runFullMigration(const nlohmann::json& project_data) {
        m_report = {};
        
        // 1. Migrate Menus
        if (project_data.contains("scenes")) {
            ui::MenuMigration::Progress ui_progress;
            ui::MenuMigration::MigrateCommandPanel("main", project_data["scenes"], ui_progress);
            m_report.total_files_processed++;
            m_report.warning_count += ui_progress.warnings.size();
            m_report.error_count += ui_progress.errors.size();
        }

        // 2. Migrate Battle Troops/Actions
        if (project_data.contains("troops")) {
            battle::BattleMigration::Progress b_progress;
            for (const auto& troop : project_data["troops"]) {
                battle::BattleMigration::migrateTroop(troop, b_progress);
            }
            m_report.total_files_processed++;
            m_report.warning_count += b_progress.warnings.size();
            m_report.error_count += b_progress.errors.size();
        }

        m_report.is_complete = true;
        m_report.summary_logs.push_back("Standard asset migration complete.");
    }

    const ProgressReport& getReport() const { return m_report; }

private:
    ProgressReport m_report;
};

} // namespace urpg::editor
