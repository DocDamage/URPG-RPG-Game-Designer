#pragma once

#include "editor/database/database_table_model.h"
#include "engine/core/project/project_reference_change_plan.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace urpg::editor {

std::string_view databaseReferenceType(DatabaseTableKind kind);

struct DatabaseReferenceOption {
    DatabaseTableKind kind = DatabaseTableKind::Actors;
    std::string object_type;
    std::string id;
    std::string name;
    bool orphan = false;
};

struct DatabaseRecordImpactPreview {
    bool record_found = false;
    bool replacement_found = false;
    bool safe = false;
    std::string code;
    std::vector<project::ProjectReferenceEdge> uses;
    project::ProjectReferenceChangePlan shared_plan;
};

// Read-only database-facing adapter over the shared project reference index and
// change-plan authorities. Typed document owners remain responsible for apply.
class DatabaseReferenceWorkspace {
public:
    DatabaseReferenceWorkspace(const ProjectDatabaseTableModel& tables, const project::ProjectReferenceIndex& index)
        : tables_(tables), index_(index) {}

    std::vector<DatabaseReferenceOption> pickerOptions(DatabaseTableKind kind, std::string_view query = {},
                                                       std::size_t limit = 100) const;
    project::ProjectReferenceQueryResult findUses(DatabaseTableKind kind, std::string_view id) const;
    std::vector<std::string> orphanIds(DatabaseTableKind kind) const;
    DatabaseRecordImpactPreview previewDelete(DatabaseTableKind kind, std::string_view id,
                                              std::string operation_id) const;
    DatabaseRecordImpactPreview previewRename(DatabaseTableKind kind, std::string_view id,
                                              std::string_view renamed_id, std::string operation_id) const;
    DatabaseRecordImpactPreview previewReplace(DatabaseTableKind kind, std::string_view id,
                                               std::string_view replacement_id, std::string operation_id) const;

private:
    const ProjectDatabaseTableModel& tables_;
    const project::ProjectReferenceIndex& index_;
};

} // namespace urpg::editor
