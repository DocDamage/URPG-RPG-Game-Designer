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
    struct WorkflowActionState {
        std::string id;
        std::string label;
        bool visible = false;
        bool enabled = false;
    };

    struct WorkflowPrimaryActions {
        WorkflowActionState run_migration;
        WorkflowActionState rerun_selected_subsystem;
        WorkflowActionState clear_selected_subsystem;
        WorkflowActionState next_subsystem;
        WorkflowActionState previous_subsystem;
    };

    struct WorkflowSubsystemCard {
        std::string subsystem_id;
        std::string display_name;
        size_t processed_count = 0;
        size_t warning_count = 0;
        size_t error_count = 0;
        bool completed = false;
        std::string summary_line;
        bool is_selected = false;
        bool can_rerun = false;
        bool can_clear = false;
    };

    struct WorkflowReportIoState {
        WorkflowActionState save;
        WorkflowActionState load;
        std::string exported_report_json;
    };

    struct WorkflowBoundRuntimeActions {
        bool has_bound_project_data = false;
        WorkflowActionState rerun_migration;
        WorkflowActionState rerun_selected_subsystem;
    };

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
        bool can_select_next_subsystem = false;
        bool can_select_previous_subsystem = false;
        bool has_bound_project_data = false;
        bool can_rerun_bound_migration = false;
        bool can_rerun_bound_selected_subsystem = false;
        std::string exported_report_json;
        bool can_save_report = false;
        bool can_load_report = false;
        std::vector<std::string> workflow_sections;
        WorkflowPrimaryActions primary_actions;
        std::vector<WorkflowSubsystemCard> subsystem_cards;
        std::optional<WorkflowSubsystemCard> selected_subsystem_card;
        WorkflowReportIoState report_io;
        WorkflowBoundRuntimeActions bound_runtime_actions;
    };

    MigrationWizardPanel() : m_model(std::make_shared<MigrationWizardModel>()) {}

    void onProjectUpdateRequested(const nlohmann::json& project_data) {
        bound_project_data_ = project_data;
        m_model->runFullMigration(project_data);
    }

    void clear() {
        m_model->clear();
        bound_project_data_.reset();
        m_has_rendered_frame = false;
        m_last_render_snapshot = {};
    }

    bool selectSubsystemResult(std::string_view subsystem_id) {
        return m_model->selectSubsystemResult(subsystem_id);
    }

    bool rerunSubsystem(std::string_view subsystem_id, const nlohmann::json& project_data) {
        return m_model->rerunSubsystem(subsystem_id, project_data);
    }

    bool rerunSelectedSubsystem(const nlohmann::json& project_data) {
        return m_model->rerunSelectedSubsystem(project_data);
    }

    bool rerunBoundProject() {
        if (!bound_project_data_.has_value()) {
            return false;
        }
        m_model->runFullMigration(*bound_project_data_);
        return true;
    }

    bool rerunBoundSelectedSubsystem() {
        if (!bound_project_data_.has_value()) {
            return false;
        }
        return m_model->rerunSelectedSubsystem(*bound_project_data_);
    }

    bool selectNextSubsystemResult() {
        return m_model->selectNextSubsystemResult();
    }

    bool selectPreviousSubsystemResult() {
        return m_model->selectPreviousSubsystemResult();
    }

    bool clearSubsystemResult(std::string_view subsystem_id) {
        return m_model->clearSubsystemResult(subsystem_id);
    }

    bool clearSelectedSubsystemResult() {
        return m_model->clearSelectedSubsystemResult();
    }

    std::string exportReportJson() {
        return m_model->getReportJson();
    }

    bool saveReportToFile(const std::string& path) {
        return m_model->saveReportToFile(path);
    }

    bool loadReportFromFile(const std::string& path) {
        bound_project_data_.reset();
        return m_model->loadReportFromFile(path);
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
        const bool has_bound_project_data = bound_project_data_.has_value();
        const bool has_data = report.total_files_processed > 0 || report.warning_count > 0 ||
            report.error_count > 0 || report.is_complete;
        RenderSnapshot snapshot = {
            report.total_files_processed,
            report.warning_count,
            report.error_count,
            report.summary_logs.size(),
            report.is_complete,
            has_data,
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
            m_model->canSelectNextSubsystemResult(),
            m_model->canSelectPreviousSubsystemResult(),
            has_bound_project_data,
            has_bound_project_data,
            has_bound_project_data && m_model->selectedSubsystemId().has_value(),
            exportReportJson(),
            has_data,
            m_visible
        };
        populateWorkflowSnapshot(snapshot, report, selected_result, has_data, has_bound_project_data);
        m_last_render_snapshot = std::move(snapshot);
        m_has_rendered_frame = true;
    }

    void update() {
        render();
    }

    bool hasRenderedFrame() const { return m_has_rendered_frame; }
    const RenderSnapshot& lastRenderSnapshot() const { return m_last_render_snapshot; }

private:
    static WorkflowActionState makeActionState(std::string id, std::string label, bool enabled) {
        return WorkflowActionState{std::move(id), std::move(label), true, enabled};
    }

    static WorkflowSubsystemCard makeSubsystemCard(const MigrationWizardModel::SubsystemResult& result, bool is_selected) {
        return WorkflowSubsystemCard{
            result.subsystem_id,
            result.display_name,
            result.processed_count,
            result.warning_count,
            result.error_count,
            result.completed,
            result.summary_line,
            is_selected,
            true,
            true
        };
    }

    void populateWorkflowSnapshot(RenderSnapshot& snapshot,
                                  const MigrationWizardModel::ProgressReport& report,
                                  const std::optional<MigrationWizardModel::SubsystemResult>& selected_result,
                                  bool has_data,
                                  bool has_bound_project_data) const {
        snapshot.workflow_sections = {"overview", "actions", "subsystems", "report_io", "bound_runtime"};
        snapshot.primary_actions = buildPrimaryActions(has_bound_project_data);
        snapshot.subsystem_cards = buildSubsystemCards(report, selected_result);
        for (const auto& card : snapshot.subsystem_cards) {
            if (card.is_selected) {
                snapshot.selected_subsystem_card = card;
                break;
            }
        }
        snapshot.report_io = buildReportIoState(has_data, snapshot.exported_report_json);
        snapshot.bound_runtime_actions = buildBoundRuntimeActions(has_bound_project_data);
    }

    WorkflowPrimaryActions buildPrimaryActions(bool has_bound_project_data) const {
        return {
            makeActionState("run_migration", "Run migration", has_bound_project_data),
            makeActionState("rerun_selected_subsystem", "Rerun selected subsystem", m_model->selectedSubsystemId().has_value()),
            makeActionState("clear_selected_subsystem", "Clear selected subsystem", m_model->selectedSubsystemId().has_value()),
            makeActionState("next_subsystem", "Next subsystem", m_model->canSelectNextSubsystemResult()),
            makeActionState("previous_subsystem", "Previous subsystem", m_model->canSelectPreviousSubsystemResult()),
        };
    }

    std::vector<WorkflowSubsystemCard> buildSubsystemCards(
        const MigrationWizardModel::ProgressReport& report,
        const std::optional<MigrationWizardModel::SubsystemResult>& selected_result) const {
        std::vector<WorkflowSubsystemCard> cards;
        cards.reserve(report.subsystem_results.size());
        for (const auto& result : report.subsystem_results) {
            const bool is_selected = selected_result.has_value() && selected_result->subsystem_id == result.subsystem_id;
            cards.push_back(makeSubsystemCard(result, is_selected));
        }
        return cards;
    }

    static WorkflowReportIoState buildReportIoState(bool has_data, const std::string& exported_report_json) {
        return {
            makeActionState("save_report", "Save report", has_data),
            makeActionState("load_report", "Load report", true),
            exported_report_json
        };
    }

    WorkflowBoundRuntimeActions buildBoundRuntimeActions(bool has_bound_project_data) const {
        return {
            has_bound_project_data,
            makeActionState("rerun_bound_migration", "Rerun bound migration", has_bound_project_data),
            makeActionState(
                "rerun_bound_selected_subsystem",
                "Rerun bound selected subsystem",
                has_bound_project_data && m_model->selectedSubsystemId().has_value())
        };
    }

    std::shared_ptr<MigrationWizardModel> m_model;
    std::optional<nlohmann::json> bound_project_data_;
    bool m_visible = false;
    bool m_has_rendered_frame = false;
    RenderSnapshot m_last_render_snapshot;
};

} // namespace urpg::editor
