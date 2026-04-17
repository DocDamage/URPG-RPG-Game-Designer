#pragma once

#include <string>
#include <vector>
#include <map>
#include <optional>
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
    struct SubsystemResult {
        std::string subsystem_id;
        std::string display_name;
        size_t processed_count = 0;
        size_t warning_count = 0;
        size_t error_count = 0;
        bool completed = false;
        std::string summary_line;
    };

    struct ProgressReport {
        size_t total_files_processed = 0;
        size_t warning_count = 0;
        size_t error_count = 0;
        bool is_complete = false;
        std::vector<std::string> summary_logs;
        std::vector<SubsystemResult> subsystem_results;
    };

    void runFullMigration(const nlohmann::json& project_data) {
        m_report = {};
        selected_subsystem_id_.reset();

        // 1. Migrate Messages
        if (project_data.contains("messages")) {
            const auto message_result =
                message::UpgradeCompatMessageDocument(project_data["messages"]);
            m_report.total_files_processed++;
            SubsystemResult message_summary{
                "message",
                "Message",
                1,
                0,
                0,
                true,
            };
            for (const auto& diagnostic : message_result.diagnostics) {
                if (diagnostic.severity == message::MessageMigrationSeverity::Error) {
                    m_report.error_count++;
                    message_summary.error_count++;
                } else if (diagnostic.severity == message::MessageMigrationSeverity::Warning) {
                    m_report.warning_count++;
                    message_summary.warning_count++;
                }
            }
            m_report.subsystem_results.push_back(message_summary);
            m_report.subsystem_results.back().summary_line =
                "Message migration: " +
                std::to_string(message_result.dialogue_sequences.size()) +
                " dialogue sequence(s), " +
                std::to_string(message_result.diagnostics.size()) +
                " diagnostic(s).";
            m_report.summary_logs.push_back(
                "Message migration: " +
                std::to_string(message_result.dialogue_sequences.size()) +
                " dialogue sequence(s), " +
                std::to_string(message_result.diagnostics.size()) +
                " diagnostic(s).");
        }

        // 1. Migrate Menus
        if (project_data.contains("scenes")) {
            ui::MenuMigration::Progress ui_progress;
            ui::MenuMigration::MigrateCommandPanel("main", project_data["scenes"], ui_progress);
            m_report.total_files_processed++;
            m_report.warning_count += ui_progress.warnings.size();
            m_report.error_count += ui_progress.errors.size();
            m_report.subsystem_results.push_back({
                "menu",
                "Menu",
                ui_progress.total_scenes,
                ui_progress.warnings.size(),
                ui_progress.errors.size(),
                true,
                "Menu migration: " + std::to_string(ui_progress.total_scenes) +
                    " scene panel(s), " + std::to_string(ui_progress.total_commands) +
                    " command(s).",
            });
            m_report.summary_logs.push_back(
                "Menu migration: " + std::to_string(ui_progress.total_scenes) +
                " scene panel(s), " + std::to_string(ui_progress.total_commands) +
                " command(s).");
        }

        // 3. Migrate Battle Troops/Actions
        if (project_data.contains("troops")) {
            battle::BattleMigration::Progress b_progress;
            for (const auto& troop : project_data["troops"]) {
                battle::BattleMigration::migrateTroop(troop, b_progress);
            }
            m_report.total_files_processed++;
            m_report.warning_count += b_progress.warnings.size();
            m_report.error_count += b_progress.errors.size();
            m_report.subsystem_results.push_back({
                "battle",
                "Battle",
                b_progress.total_troops,
                b_progress.warnings.size(),
                b_progress.errors.size(),
                true,
                "Battle migration: " + std::to_string(b_progress.total_troops) +
                    " troop(s), " + std::to_string(b_progress.total_actions) +
                    " action(s).",
            });
            m_report.summary_logs.push_back(
                "Battle migration: " + std::to_string(b_progress.total_troops) +
                " troop(s), " + std::to_string(b_progress.total_actions) +
                " action(s).");
        }

        m_report.is_complete = true;
        m_report.summary_logs.push_back("Migration wizard complete.");

        if (!m_report.subsystem_results.empty()) {
            selected_subsystem_id_ = m_report.subsystem_results.front().subsystem_id;
        }
    }

    void clear() {
        m_report = {};
        selected_subsystem_id_.reset();
    }

    const ProgressReport& getReport() const { return m_report; }

    bool selectSubsystemResult(std::string_view subsystem_id) {
        for (const auto& result : m_report.subsystem_results) {
            if (result.subsystem_id == subsystem_id) {
                selected_subsystem_id_ = result.subsystem_id;
                return true;
            }
        }

        selected_subsystem_id_.reset();
        return false;
    }

    std::optional<std::string> selectedSubsystemId() const {
        return selected_subsystem_id_;
    }

    std::optional<SubsystemResult> selectedSubsystemResult() const {
        if (!selected_subsystem_id_.has_value()) {
            return std::nullopt;
        }

        for (const auto& result : m_report.subsystem_results) {
            if (result.subsystem_id == *selected_subsystem_id_) {
                return result;
            }
        }

        return std::nullopt;
    }

private:
    ProgressReport m_report;
    std::optional<std::string> selected_subsystem_id_;
};

} // namespace urpg::editor
