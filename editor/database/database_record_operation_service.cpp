#include "editor/database/database_record_operation_service.h"

#include "editor/database/database_reference_workspace.h"

#include <algorithm>
#include <set>
#include <utility>

namespace urpg::editor {
namespace {

project::ProjectReferenceChangeKind referenceKind(const DatabaseRecordOperationKind kind) {
    switch (kind) {
    case DatabaseRecordOperationKind::Rename: return project::ProjectReferenceChangeKind::Rename;
    case DatabaseRecordOperationKind::Delete: return project::ProjectReferenceChangeKind::Delete;
    case DatabaseRecordOperationKind::Replace: return project::ProjectReferenceChangeKind::Replace;
    }
    return project::ProjectReferenceChangeKind::Replace;
}

std::string operationLabel(const DatabaseRecordOperationKind kind) {
    switch (kind) {
    case DatabaseRecordOperationKind::Rename: return "Rename Database Record";
    case DatabaseRecordOperationKind::Delete: return "Delete Database Record";
    case DatabaseRecordOperationKind::Replace: return "Replace Database Record";
    }
    return "Change Database Record";
}

} // namespace

const char* databaseRecordOperationName(const DatabaseRecordOperationKind kind) {
    switch (kind) {
    case DatabaseRecordOperationKind::Rename: return "rename";
    case DatabaseRecordOperationKind::Delete: return "delete";
    case DatabaseRecordOperationKind::Replace: return "replace";
    }
    return "replace";
}

DatabaseRecordOperationService::DatabaseRecordOperationService(std::filesystem::path journal_path)
    : journal_(std::make_unique<project::ProjectOperationJournal>(std::move(journal_path))),
      coordinator_(journal_.get()) {}

DatabaseRecordOperationPreview DatabaseRecordOperationService::preview(
    const ProjectDatabaseTableModel& tables, const project::ProjectReferenceIndex& index,
    const DatabaseRecordOperationRequest& request) const {
    DatabaseRecordOperationPreview result;
    result.request = request;
    result.label = operationLabel(request.kind);
    const auto& table = tables.table(request.table_kind);
    if (request.operation_id.empty() || request.source_id.empty() || request.record_document.empty()) {
        result.code = "database_record_operation_invalid";
        result.message = "Database record operations require stable operation, source, and owner-document identities.";
        return result;
    }
    if (table.find(request.source_id) == nullptr) {
        result.code = "database_record_operation_source_missing";
        result.message = "The source database record no longer exists.";
        return result;
    }
    if (request.kind != DatabaseRecordOperationKind::Delete) {
        if (request.replacement_id.empty() || request.replacement_id == request.source_id) {
            result.code = "database_record_operation_replacement_invalid";
            result.message = "Rename and replacement require a different stable destination ID.";
            return result;
        }
        const bool replacementExists = table.find(request.replacement_id) != nullptr;
        if (request.kind == DatabaseRecordOperationKind::Replace && !replacementExists) {
            result.code = "database_record_operation_replacement_missing";
            result.message = "Replacement requires an existing destination record.";
            return result;
        }
        if (request.kind == DatabaseRecordOperationKind::Rename && replacementExists) {
            result.code = "database_record_operation_rename_conflict";
            result.message = "Rename cannot overwrite an existing stable record ID.";
            return result;
        }
    }
    result.reference_plan = project::previewProjectReferenceChange(
        index, {request.operation_id, referenceKind(request.kind),
                std::string(databaseReferenceType(request.table_kind)), request.source_id,
                request.replacement_id, {}});
    result.success = result.reference_plan.success;
    result.applicable = result.reference_plan.applicable;
    result.code = result.reference_plan.code;
    result.message = result.reference_plan.message;
    return result;
}

project::ProjectOperationResult DatabaseRecordOperationService::execute(
    const DatabaseRecordOperationPreview& preview,
    std::vector<project::ProjectOperationParticipant> participants) {
    if (!preview.success || !preview.applicable) {
        return {false, false, "database_record_operation_preview_not_applicable",
                "The database record operation requires an applicable reviewed impact preview.", {}};
    }
    std::set<std::filesystem::path> requiredDocuments{
        preview.request.record_document.lexically_normal()};
    for (const auto& update : preview.reference_plan.updates) {
        requiredDocuments.insert(update.before.document_path.lexically_normal());
    }
    for (const auto& required : requiredDocuments) {
        const auto owner = std::find_if(participants.begin(), participants.end(), [&](const auto& participant) {
            return !participant.document_path.empty() &&
                   participant.document_path.lexically_normal() == required;
        });
        if (owner == participants.end()) {
            return {false, false, "database_record_operation_owner_coverage_missing",
                    "The reviewed operation names a document without a participating typed owner: " +
                        required.generic_string(),
                    {}};
        }
    }
    return coordinator_.execute(
        {preview.request.operation_id, preview.label, std::move(participants)});
}

project::ProjectOperationResult DatabaseRecordOperationService::undoLast() {
    return coordinator_.undoLast();
}

project::ProjectOperationResult DatabaseRecordOperationService::redoLast() {
    return coordinator_.redoLast();
}

} // namespace urpg::editor
