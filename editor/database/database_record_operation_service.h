#pragma once

#include "editor/database/database_table_model.h"
#include "engine/core/project/project_operation_coordinator.h"
#include "engine/core/project/project_reference_change_plan.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace urpg::editor {

enum class DatabaseRecordOperationKind { Rename, Delete, Replace };

struct DatabaseRecordOperationRequest {
    std::string operation_id;
    DatabaseRecordOperationKind kind = DatabaseRecordOperationKind::Replace;
    DatabaseTableKind table_kind = DatabaseTableKind::Actors;
    std::string source_id;
    std::string replacement_id;
    std::filesystem::path record_document;
};

struct DatabaseRecordOperationPreview {
    bool success = false;
    bool applicable = false;
    std::string code;
    std::string message;
    std::string label;
    DatabaseRecordOperationRequest request;
    project::ProjectReferenceChangePlan reference_plan;
};

class DatabaseRecordOperationService {
public:
    DatabaseRecordOperationService() = default;
    explicit DatabaseRecordOperationService(std::filesystem::path journal_path);

    DatabaseRecordOperationPreview preview(const ProjectDatabaseTableModel& tables,
                                           const project::ProjectReferenceIndex& index,
                                           const DatabaseRecordOperationRequest& request) const;
    project::ProjectOperationResult execute(
        const DatabaseRecordOperationPreview& preview,
        std::vector<project::ProjectOperationParticipant> participants);
    project::ProjectOperationResult undoLast();
    project::ProjectOperationResult redoLast();
    std::string undoLabel() const { return coordinator_.undoLabel(); }
    std::string redoLabel() const { return coordinator_.redoLabel(); }

private:
    std::unique_ptr<project::ProjectOperationJournal> journal_;
    project::ProjectOperationCoordinator coordinator_;
};

const char* databaseRecordOperationName(DatabaseRecordOperationKind kind);

} // namespace urpg::editor
