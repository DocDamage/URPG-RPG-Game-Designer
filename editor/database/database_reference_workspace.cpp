#include "editor/database/database_reference_workspace.h"

#include <algorithm>
#include <cctype>

namespace urpg::editor {

namespace {

std::string lower(std::string_view value) {
    std::string result(value);
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return result;
}

} // namespace

std::string_view databaseReferenceType(DatabaseTableKind kind) {
    switch (kind) {
    case DatabaseTableKind::Actors: return "actor";
    case DatabaseTableKind::Classes: return "class";
    case DatabaseTableKind::Skills: return "skill";
    case DatabaseTableKind::Items: return "item";
    case DatabaseTableKind::Equipment: return "equipment";
    case DatabaseTableKind::Enemies: return "enemy";
    case DatabaseTableKind::Encounters: return "encounter";
    case DatabaseTableKind::States: return "state";
    case DatabaseTableKind::Quests: return "quest";
    case DatabaseTableKind::Vendors: return "vendor";
    case DatabaseTableKind::Recipes: return "recipe";
    case DatabaseTableKind::Switches: return "switch";
    case DatabaseTableKind::Variables: return "variable";
    }
    return "unknown";
}

std::vector<DatabaseReferenceOption> DatabaseReferenceWorkspace::pickerOptions(DatabaseTableKind kind,
                                                                               std::string_view query,
                                                                               std::size_t limit) const {
    std::vector<DatabaseReferenceOption> options;
    const auto needle = lower(query);
    const auto object_type = std::string(databaseReferenceType(kind));
    for (const auto& row : tables_.table(kind).rows()) {
        if (!needle.empty() && lower(row.id).find(needle) == std::string::npos &&
            lower(row.name).find(needle) == std::string::npos) continue;
        options.push_back({kind, object_type, row.id, row.name, index_.inbound(object_type, row.id).empty()});
        if (options.size() == limit) break;
    }
    return options;
}

project::ProjectReferenceQueryResult DatabaseReferenceWorkspace::findUses(DatabaseTableKind kind,
                                                                          std::string_view id) const {
    return index_.findUses(databaseReferenceType(kind), id);
}

std::vector<std::string> DatabaseReferenceWorkspace::orphanIds(DatabaseTableKind kind) const {
    std::vector<std::string> orphans;
    const auto object_type = databaseReferenceType(kind);
    for (const auto& row : tables_.table(kind).rows()) {
        if (index_.inbound(object_type, row.id).empty()) orphans.push_back(row.id);
    }
    return orphans;
}

DatabaseRecordImpactPreview DatabaseReferenceWorkspace::previewDelete(DatabaseTableKind kind, std::string_view id,
                                                                      std::string operation_id) const {
    DatabaseRecordImpactPreview preview;
    preview.record_found = tables_.table(kind).find(id) != nullptr;
    if (!preview.record_found) {
        preview.code = "database_record_missing";
        return preview;
    }
    const auto object_type = std::string(databaseReferenceType(kind));
    preview.uses = index_.findUses(object_type, id).matches;
    preview.shared_plan = project::previewProjectReferenceChange(
        index_, {std::move(operation_id), project::ProjectReferenceChangeKind::Delete, object_type,
                 std::string(id), {}, {}});
    preview.safe = preview.shared_plan.success && preview.shared_plan.applicable;
    preview.code = preview.shared_plan.code;
    return preview;
}

DatabaseRecordImpactPreview DatabaseReferenceWorkspace::previewRename(DatabaseTableKind kind, std::string_view id,
                                                                      std::string_view renamed_id,
                                                                      std::string operation_id) const {
    DatabaseRecordImpactPreview preview;
    const auto& table = tables_.table(kind);
    preview.record_found = table.find(id) != nullptr;
    preview.replacement_found = table.find(renamed_id) != nullptr;
    if (!preview.record_found || renamed_id.empty() || id == renamed_id || preview.replacement_found) {
        preview.code = !preview.record_found ? "database_record_missing" :
            (renamed_id.empty() || id == renamed_id ? "database_rename_id_invalid" : "database_rename_conflict");
        return preview;
    }
    const auto object_type = std::string(databaseReferenceType(kind));
    preview.uses = index_.findUses(object_type, id).matches;
    preview.shared_plan = project::previewProjectReferenceChange(
        index_, {std::move(operation_id), project::ProjectReferenceChangeKind::Rename, object_type,
                 std::string(id), std::string(renamed_id), {}});
    preview.safe = preview.shared_plan.success && preview.shared_plan.applicable;
    preview.code = preview.shared_plan.code;
    return preview;
}

DatabaseRecordImpactPreview DatabaseReferenceWorkspace::previewReplace(DatabaseTableKind kind, std::string_view id,
                                                                       std::string_view replacement_id,
                                                                       std::string operation_id) const {
    DatabaseRecordImpactPreview preview;
    const auto& table = tables_.table(kind);
    preview.record_found = table.find(id) != nullptr;
    preview.replacement_found = table.find(replacement_id) != nullptr;
    if (!preview.record_found || !preview.replacement_found || id == replacement_id) {
        preview.code = !preview.record_found ? "database_record_missing" :
            (!preview.replacement_found ? "database_replacement_missing" : "database_replacement_same_id");
        return preview;
    }
    const auto object_type = std::string(databaseReferenceType(kind));
    preview.uses = index_.findUses(object_type, id).matches;
    preview.shared_plan = project::previewProjectReferenceChange(
        index_, {std::move(operation_id), project::ProjectReferenceChangeKind::Replace, object_type,
                 std::string(id), std::string(replacement_id), {}});
    preview.safe = preview.shared_plan.success && preview.shared_plan.applicable;
    preview.code = preview.shared_plan.code;
    return preview;
}

} // namespace urpg::editor
