#pragma once

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>
#include "../../engine/core/message/message_migration.h"
#include "../../engine/core/battle/battle_migration.h"
#include "../../engine/core/save/save_migration.h"
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

        if (project_data.contains("messages")) {
            auto result = runMessageMigration(project_data["messages"]);
            m_report.total_files_processed++;
            m_report.warning_count += result.warning_count;
            m_report.error_count += result.error_count;
            m_report.subsystem_results.push_back(result);
        }

        if (project_data.contains("scenes")) {
            auto result = runMenuMigration(project_data["scenes"]);
            m_report.total_files_processed++;
            m_report.warning_count += result.warning_count;
            m_report.error_count += result.error_count;
            m_report.subsystem_results.push_back(result);
        }

        if (project_data.contains("troops")) {
            auto result = runBattleMigration(project_data["troops"]);
            m_report.total_files_processed++;
            m_report.warning_count += result.warning_count;
            m_report.error_count += result.error_count;
            m_report.subsystem_results.push_back(result);
        }

        if (project_data.contains("save")) {
            auto result = runSaveMigration(project_data["save"]);
            m_report.total_files_processed++;
            m_report.warning_count += result.warning_count;
            m_report.error_count += result.error_count;
            m_report.subsystem_results.push_back(result);
        }

        m_report.is_complete = true;
        rebuildSummaryLogs();

        if (!m_report.subsystem_results.empty()) {
            selected_subsystem_id_ = m_report.subsystem_results.front().subsystem_id;
        }
    }

    bool clearSubsystemResult(std::string_view subsystem_id) {
        for (auto it = m_report.subsystem_results.begin(); it != m_report.subsystem_results.end(); ++it) {
            if (it->subsystem_id == subsystem_id) {
                m_report.total_files_processed = (m_report.total_files_processed > 0) ? m_report.total_files_processed - 1 : 0;
                m_report.warning_count = (m_report.warning_count >= it->warning_count) ? m_report.warning_count - it->warning_count : 0;
                m_report.error_count = (m_report.error_count >= it->error_count) ? m_report.error_count - it->error_count : 0;
                const bool was_selected = selected_subsystem_id_.has_value() && *selected_subsystem_id_ == subsystem_id;
                m_report.subsystem_results.erase(it);
                if (was_selected) {
                    selected_subsystem_id_.reset();
                    if (!m_report.subsystem_results.empty()) {
                        selected_subsystem_id_ = m_report.subsystem_results.front().subsystem_id;
                    }
                }

                if (m_report.subsystem_results.empty()) {
                    clear();
                    return true;
                }

                rebuildSummaryLogs();
                return true;
            }
        }
        return false;
    }

    bool clearSelectedSubsystemResult() {
        if (!selected_subsystem_id_.has_value()) {
            return false;
        }
        const std::string selected_subsystem_id = *selected_subsystem_id_;
        return clearSubsystemResult(selected_subsystem_id);
    }

    bool rerunSubsystem(std::string_view subsystem_id, const nlohmann::json& project_data) {
        for (auto it = m_report.subsystem_results.begin(); it != m_report.subsystem_results.end(); ++it) {
            if (it->subsystem_id == subsystem_id) {
                m_report.total_files_processed = (m_report.total_files_processed > 0) ? m_report.total_files_processed - 1 : 0;
                m_report.warning_count = (m_report.warning_count >= it->warning_count) ? m_report.warning_count - it->warning_count : 0;
                m_report.error_count = (m_report.error_count >= it->error_count) ? m_report.error_count - it->error_count : 0;
                m_report.subsystem_results.erase(it);
                break;
            }
        }

        SubsystemResult new_result;
        bool ran = false;
        if (subsystem_id == "message" && project_data.contains("messages")) {
            new_result = runMessageMigration(project_data["messages"]);
            ran = true;
        } else if (subsystem_id == "menu" && project_data.contains("scenes")) {
            new_result = runMenuMigration(project_data["scenes"]);
            ran = true;
        } else if (subsystem_id == "battle" && project_data.contains("troops")) {
            new_result = runBattleMigration(project_data["troops"]);
            ran = true;
        } else if (subsystem_id == "save" && project_data.contains("save")) {
            new_result = runSaveMigration(project_data["save"]);
            ran = true;
        }

        if (!ran) {
            rebuildSummaryLogs();
            return false;
        }

        m_report.total_files_processed++;
        m_report.warning_count += new_result.warning_count;
        m_report.error_count += new_result.error_count;
        m_report.subsystem_results.push_back(new_result);
        m_report.is_complete = true;
        rebuildSummaryLogs();

        if (!selected_subsystem_id_.has_value() && !m_report.subsystem_results.empty()) {
            selected_subsystem_id_ = m_report.subsystem_results.front().subsystem_id;
        }

        return true;
    }

    bool rerunSelectedSubsystem(const nlohmann::json& project_data) {
        if (!selected_subsystem_id_.has_value()) {
            return false;
        }
        const std::string selected_subsystem_id = *selected_subsystem_id_;
        return rerunSubsystem(selected_subsystem_id, project_data);
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

    bool selectNextSubsystemResult() {
        if (m_report.subsystem_results.empty()) {
            return false;
        }

        if (!selected_subsystem_id_.has_value()) {
            selected_subsystem_id_ = m_report.subsystem_results.front().subsystem_id;
            return true;
        }

        const auto current_index = selectedSubsystemIndex();
        if (!current_index.has_value() || *current_index + 1 >= m_report.subsystem_results.size()) {
            return false;
        }

        selected_subsystem_id_ = m_report.subsystem_results[*current_index + 1].subsystem_id;
        return true;
    }

    bool selectPreviousSubsystemResult() {
        if (m_report.subsystem_results.empty()) {
            return false;
        }

        if (!selected_subsystem_id_.has_value()) {
            selected_subsystem_id_ = m_report.subsystem_results.back().subsystem_id;
            return true;
        }

        const auto current_index = selectedSubsystemIndex();
        if (!current_index.has_value() || *current_index == 0) {
            return false;
        }

        selected_subsystem_id_ = m_report.subsystem_results[*current_index - 1].subsystem_id;
        return true;
    }

    bool canSelectNextSubsystemResult() const {
        if (m_report.subsystem_results.empty()) {
            return false;
        }

        if (!selected_subsystem_id_.has_value()) {
            return true;
        }

        const auto current_index = selectedSubsystemIndex();
        return current_index.has_value() && *current_index + 1 < m_report.subsystem_results.size();
    }

    bool canSelectPreviousSubsystemResult() const {
        if (m_report.subsystem_results.empty()) {
            return false;
        }

        if (!selected_subsystem_id_.has_value()) {
            return true;
        }

        const auto current_index = selectedSubsystemIndex();
        return current_index.has_value() && *current_index > 0;
    }

    bool selectNextIssueSubsystemResult() {
        const auto next_index = adjacentIssueSubsystemIndex(/*forward=*/true);
        if (!next_index.has_value()) {
            return false;
        }

        selected_subsystem_id_ = m_report.subsystem_results[*next_index].subsystem_id;
        return true;
    }

    bool selectPreviousIssueSubsystemResult() {
        const auto previous_index = adjacentIssueSubsystemIndex(/*forward=*/false);
        if (!previous_index.has_value()) {
            return false;
        }

        selected_subsystem_id_ = m_report.subsystem_results[*previous_index].subsystem_id;
        return true;
    }

    bool canSelectNextIssueSubsystemResult() const {
        return adjacentIssueSubsystemIndex(/*forward=*/true).has_value();
    }

    bool canSelectPreviousIssueSubsystemResult() const {
        return adjacentIssueSubsystemIndex(/*forward=*/false).has_value();
    }

    std::optional<std::string> selectedSubsystemId() const {
        return selected_subsystem_id_;
    }

    std::string getReportJson() const {
        nlohmann::json root;
        root["total_files_processed"] = m_report.total_files_processed;
        root["warning_count"] = m_report.warning_count;
        root["error_count"] = m_report.error_count;
        root["is_complete"] = m_report.is_complete;
        root["summary_logs"] = m_report.summary_logs;
        root["subsystem_results"] = nlohmann::json::array();
        for (const auto& result : m_report.subsystem_results) {
            nlohmann::json sub;
            sub["subsystem_id"] = result.subsystem_id;
            sub["display_name"] = result.display_name;
            sub["processed_count"] = result.processed_count;
            sub["warning_count"] = result.warning_count;
            sub["error_count"] = result.error_count;
            sub["completed"] = result.completed;
            sub["summary_line"] = result.summary_line;
            root["subsystem_results"].push_back(sub);
        }
        if (selected_subsystem_id_.has_value()) {
            root["selected_subsystem_id"] = *selected_subsystem_id_;
        } else {
            root["selected_subsystem_id"] = nullptr;
        }
        return root.dump();
    }

    bool saveReportToFile(const std::string& path) {
        std::ofstream ofs(path);
        if (!ofs) return false;
        ofs << getReportJson();
        return ofs.good();
    }

    bool loadReportFromFile(const std::string& path) {
        clear();
        std::ifstream ifs(path);
        if (!ifs) return false;
        try {
            nlohmann::json root;
            ifs >> root;

            if (!root.contains("total_files_processed") || !root["total_files_processed"].is_number_unsigned())
                return false;
            if (!root.contains("warning_count") || !root["warning_count"].is_number_unsigned())
                return false;
            if (!root.contains("error_count") || !root["error_count"].is_number_unsigned())
                return false;
            if (!root.contains("is_complete") || !root["is_complete"].is_boolean())
                return false;
            if (!root.contains("summary_logs") || !root["summary_logs"].is_array())
                return false;
            if (!root.contains("subsystem_results") || !root["subsystem_results"].is_array())
                return false;

            m_report.total_files_processed = root["total_files_processed"].get<size_t>();
            m_report.warning_count = root["warning_count"].get<size_t>();
            m_report.error_count = root["error_count"].get<size_t>();
            m_report.is_complete = root["is_complete"].get<bool>();
            m_report.summary_logs = root["summary_logs"].get<std::vector<std::string>>();

            for (const auto& item : root["subsystem_results"]) {
                if (!item.contains("subsystem_id") || !item["subsystem_id"].is_string() ||
                    !item.contains("display_name") || !item["display_name"].is_string() ||
                    !item.contains("processed_count") || !item["processed_count"].is_number_unsigned() ||
                    !item.contains("warning_count") || !item["warning_count"].is_number_unsigned() ||
                    !item.contains("error_count") || !item["error_count"].is_number_unsigned() ||
                    !item.contains("completed") || !item["completed"].is_boolean() ||
                    !item.contains("summary_line") || !item["summary_line"].is_string()) {
                    clear();
                    return false;
                }
                SubsystemResult sr;
                sr.subsystem_id = item["subsystem_id"].get<std::string>();
                sr.display_name = item["display_name"].get<std::string>();
                sr.processed_count = item["processed_count"].get<size_t>();
                sr.warning_count = item["warning_count"].get<size_t>();
                sr.error_count = item["error_count"].get<size_t>();
                sr.completed = item["completed"].get<bool>();
                sr.summary_line = item["summary_line"].get<std::string>();
                m_report.subsystem_results.push_back(std::move(sr));
            }

            if (root.contains("selected_subsystem_id")) {
                if (root["selected_subsystem_id"].is_string()) {
                    selected_subsystem_id_ = root["selected_subsystem_id"].get<std::string>();
                } else if (root["selected_subsystem_id"].is_null()) {
                    selected_subsystem_id_.reset();
                } else {
                    clear();
                    return false;
                }
            } else if (!m_report.subsystem_results.empty()) {
                selected_subsystem_id_ = m_report.subsystem_results.front().subsystem_id;
            }

            if (selected_subsystem_id_.has_value() && !selectedSubsystemResult().has_value()) {
                if (!m_report.subsystem_results.empty()) {
                    selected_subsystem_id_ = m_report.subsystem_results.front().subsystem_id;
                } else {
                    selected_subsystem_id_.reset();
                }
            }
            return true;
        } catch (...) {
            clear();
            return false;
        }
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
    SubsystemResult runMessageMigration(const nlohmann::json& messages_data) {
        const auto message_result = message::UpgradeCompatMessageDocument(messages_data);
        SubsystemResult summary{
            "message",
            "Message",
            1,
            0,
            0,
            true,
        };
        for (const auto& diagnostic : message_result.diagnostics) {
            if (diagnostic.severity == message::MessageMigrationSeverity::Error) {
                summary.error_count++;
            } else if (diagnostic.severity == message::MessageMigrationSeverity::Warning) {
                summary.warning_count++;
            }
        }
        summary.summary_line =
            "Message migration: " +
            std::to_string(message_result.dialogue_sequences.size()) +
            " dialogue sequence(s), " +
            std::to_string(message_result.diagnostics.size()) +
            " diagnostic(s).";
        return summary;
    }

    SubsystemResult runMenuMigration(const nlohmann::json& scenes_data) {
        ui::MenuMigration::Progress ui_progress;
        ui::MenuMigration::MigrateCommandPanel("main", scenes_data, ui_progress);
        SubsystemResult summary{
            "menu",
            "Menu",
            ui_progress.total_scenes,
            ui_progress.warnings.size(),
            ui_progress.errors.size(),
            true,
            "Menu migration: " + std::to_string(ui_progress.total_scenes) +
                " scene panel(s), " + std::to_string(ui_progress.total_commands) +
                " command(s).",
        };
        return summary;
    }

    SubsystemResult runBattleMigration(const nlohmann::json& troops_data) {
        battle::BattleMigration::Progress b_progress;
        for (const auto& troop : troops_data) {
            battle::BattleMigration::migrateTroop(troop, b_progress);
        }
        SubsystemResult summary{
            "battle",
            "Battle",
            b_progress.total_troops,
            b_progress.warnings.size(),
            b_progress.errors.size(),
            true,
            "Battle migration: " + std::to_string(b_progress.total_troops) +
                " troop(s), " + std::to_string(b_progress.total_actions) +
                " action(s).",
        };
        return summary;
    }

    SubsystemResult runSaveMigration(const nlohmann::json& save_data) {
        const auto save_result = save::UpgradeCompatSaveMetadataDocument(save_data);
        size_t warning_count = 0;
        size_t error_count = 0;
        for (const auto& diagnostic : save_result.diagnostics) {
            if (diagnostic.severity == save::SaveMigrationSeverity::Error) {
                error_count++;
            } else if (diagnostic.severity == save::SaveMigrationSeverity::Warning) {
                warning_count++;
            }
        }

        SubsystemResult summary{
            "save",
            "Save",
            1,
            warning_count,
            error_count,
            true,
            "Save migration: 1 metadata document, " + std::to_string(save_result.diagnostics.size()) +
                " diagnostic(s).",
        };
        return summary;
    }

    void rebuildSummaryLogs() {
        m_report.summary_logs.clear();
        for (const auto& result : m_report.subsystem_results) {
            m_report.summary_logs.push_back(result.summary_line);
        }
        if (m_report.is_complete || !m_report.subsystem_results.empty()) {
            m_report.summary_logs.push_back("Migration wizard complete.");
        }
    }

    std::optional<size_t> selectedSubsystemIndex() const {
        if (!selected_subsystem_id_.has_value()) {
            return std::nullopt;
        }

        for (size_t index = 0; index < m_report.subsystem_results.size(); ++index) {
            if (m_report.subsystem_results[index].subsystem_id == *selected_subsystem_id_) {
                return index;
            }
        }

        return std::nullopt;
    }

    static bool subsystemHasIssues(const SubsystemResult& result) {
        return result.warning_count > 0 || result.error_count > 0;
    }

    std::optional<size_t> adjacentIssueSubsystemIndex(bool forward) const {
        if (m_report.subsystem_results.empty()) {
            return std::nullopt;
        }

        if (!selected_subsystem_id_.has_value()) {
            const auto it = std::find_if(m_report.subsystem_results.begin(),
                                         m_report.subsystem_results.end(),
                                         subsystemHasIssues);
            if (it == m_report.subsystem_results.end()) {
                return std::nullopt;
            }
            return static_cast<size_t>(std::distance(m_report.subsystem_results.begin(), it));
        }

        const auto current_index = selectedSubsystemIndex();
        if (!current_index.has_value()) {
            return std::nullopt;
        }

        if (forward) {
            for (size_t index = *current_index + 1; index < m_report.subsystem_results.size(); ++index) {
                if (subsystemHasIssues(m_report.subsystem_results[index])) {
                    return index;
                }
            }
            return std::nullopt;
        }

        for (size_t index = *current_index; index > 0; --index) {
            if (subsystemHasIssues(m_report.subsystem_results[index - 1])) {
                return index - 1;
            }
        }

        return std::nullopt;
    }

    ProgressReport m_report;
    std::optional<std::string> selected_subsystem_id_;
};

} // namespace urpg::editor
