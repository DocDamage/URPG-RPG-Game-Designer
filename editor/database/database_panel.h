#pragma once

#include "editor/database/database_table_model.h"
#include "editor/database/database_batch_edit.h"
#include "editor/database/database_record_operation_service.h"
#include "engine/core/database/rpg_database.h"

#include <optional>

namespace urpg::editor {

struct DatabasePanelSnapshot {
    std::size_t actor_count = 0;
    std::size_t item_count = 0;
    std::size_t diagnostic_count = 0;
    std::size_t table_count = 0;
    std::size_t reference_review_use_count = 0;
    std::size_t reference_review_package_impact_count = 0;
    bool reference_review_pending = false;
    bool reference_review_can_apply = false;
    std::string reference_review_code;
};

class DatabasePanel {
public:
    void setDatabase(database::RpgDatabase database);
    void bindDatabase(database::RpgDatabase& database);
    void refreshFromAuthority();
    bool applyBatch(DatabaseBatchEditPlan& plan, std::string* diagnostic = nullptr);
    bool undoBatch(DatabaseBatchEditPlan& plan, std::string* diagnostic = nullptr);
    bool redoBatch(DatabaseBatchEditPlan& plan, std::string* diagnostic = nullptr);
    void bindReferenceIndex(const project::ProjectReferenceIndex* index);
    DatabaseRecordOperationPreview previewRecordOperation(DatabaseRecordOperationRequest request);
    project::ProjectOperationResult applyReviewedRecordOperation(
        std::vector<project::ProjectOperationParticipant> participants);
    project::ProjectOperationResult undoRecordOperation();
    project::ProjectOperationResult redoRecordOperation();
    DatabasePanelSnapshot snapshot() const;
    void render();
    ProjectDatabaseTableModel& tables() { return tables_; }
    const ProjectDatabaseTableModel& tables() const { return tables_; }
    const DatabasePanelSnapshot& lastRenderSnapshot() const { return last_render_snapshot_; }

private:
    bool synchronizeRows(DatabaseTableKind kind, const std::vector<DatabaseTableRow>& rows,
                         database::RpgDatabase& output, std::string* diagnostic) const;
    database::RpgDatabase database_;
    database::RpgDatabase* authority_ = &database_;
    ProjectDatabaseTableModel tables_;
    const project::ProjectReferenceIndex* reference_index_ = nullptr;
    DatabaseRecordOperationService record_operations_;
    std::optional<DatabaseRecordOperationPreview> pending_record_operation_;
    DatabasePanelSnapshot last_render_snapshot_{};
};

} // namespace urpg::editor
