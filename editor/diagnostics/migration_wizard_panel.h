#pragma once

#include "migration_wizard_model.h"
#include <memory>
#include <optional>

namespace urpg::editor {

/**
 * @brief GUI controller for the Project Migration Wizard.
 */
class MigrationWizardPanel {
public:
    struct RenderSnapshot {
        size_t total_files_processed = 0;
        size_t warning_count = 0;
        size_t error_count = 0;
        size_t summary_log_count = 0;
        bool is_complete = false;
        bool has_data = false;
        std::string headline;
        std::vector<std::string> summary_logs;
        std::vector<MigrationWizardModel::SubsystemResult> subsystem_results;
        std::optional<std::string> selected_subsystem_id;
        std::string selected_subsystem_display_name;
        size_t selected_subsystem_processed_count = 0;
        size_t selected_subsystem_warning_count = 0;
        size_t selected_subsystem_error_count = 0;
        bool selected_subsystem_completed = false;
        std::string selected_subsystem_summary_line;
        bool can_rerun_selected_subsystem = false;
        bool can_clear_selected_subsystem = false;
        std::string exported_report_json;
    };

    MigrationWizardPanel() : m_model(std::make_shared<MigrationWizardModel>()) {}

    void onProjectUpdateRequested(const nlohmann::json& project_data) {
        m_model->runFullMigration(project_data);
    }

    void clear() {
        m_model->clear();
        m_has_rendered_frame = false;
        m_last_render_snapshot = {};
    }

    bool selectSubsystemResult(std::string_view subsystem_id) {
        return m_model->selectSubsystemResult(subsystem_id);
    }

    bool rerunSubsystem(std::string_view subsystem_id, const nlohmann::json& project_data) {
        return m_model->rerunSubsystem(subsystem_id, project_data);
    }

    bool clearSubsystemResult(std::string_view subsystem_id) {
        return m_model->clearSubsystemResult(subsystem_id);
    }

    std::string exportReportJson() {
        return m_model->getReportJson();
    }

    std::shared_ptr<MigrationWizardModel> getModel() const { return m_model; }

    bool isVisible() const { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }

    void render() {
        if (!m_visible) {
            return;
        }

        const auto& report = m_model->getReport();
        const auto selected_result = m_model->selectedSubsystemResult();
        m_last_render_snapshot = {
            report.total_files_processed,
            report.warning_count,
            report.error_count,
            report.summary_logs.size(),
            report.is_complete,
            report.total_files_processed > 0 || report.warning_count > 0 ||
                report.error_count > 0 || report.is_complete,
            report.summary_logs.empty() ? std::string{} : report.summary_logs.back(),
            report.summary_logs,
            report.subsystem_results,
            m_model->selectedSubsystemId(),
            selected_result.has_value() ? selected_result->display_name : std::string{},
            selected_result.has_value() ? selected_result->processed_count : 0,
            selected_result.has_value() ? selected_result->warning_count : 0,
            selected_result.has_value() ? selected_result->error_count : 0,
            selected_result.has_value() ? selected_result->completed : false,
            selected_result.has_value() ? selected_result->summary_line : std::string{},
            m_model->selectedSubsystemId().has_value(),
            m_model->selectedSubsystemId().has_value(),
            exportReportJson()
        };
        m_has_rendered_frame = true;
    }

    void update() {
        render();
    }

    bool hasRenderedFrame() const { return m_has_rendered_frame; }
    const RenderSnapshot& lastRenderSnapshot() const { return m_last_render_snapshot; }

private:
    std::shared_ptr<MigrationWizardModel> m_model;
    bool m_visible = false;
    bool m_has_rendered_frame = false;
    RenderSnapshot m_last_render_snapshot;
};

} // namespace urpg::editor
