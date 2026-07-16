#include "editor/database/database_batch_edit.h"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <iomanip>
#include <set>
#include <sstream>

namespace urpg::editor {

namespace {

std::string* editableField(DatabaseTableRow& row, const std::string& field) {
    if (field == "name") return &row.name;
    if (field == "summary") return &row.summary;
    if (field.empty() || field == "id") return nullptr;
    return &row.fields[field];
}

const std::string* existingField(const DatabaseTableRow& row, const std::string& field) {
    if (field == "name") return &row.name;
    if (field == "summary") return &row.summary;
    const auto found = row.fields.find(field);
    return found == row.fields.end() ? nullptr : &found->second;
}

bool parseNumber(const std::string& value, double& result) {
    const auto* begin = value.data();
    const auto* end = begin + value.size();
    const auto parsed = std::from_chars(begin, end, result);
    return parsed.ec == std::errc{} && parsed.ptr == end && std::isfinite(result);
}

std::string formatNumber(double value) {
    std::ostringstream stream;
    stream << std::setprecision(12) << value;
    return stream.str();
}

DatabaseTableRow* findRow(std::vector<DatabaseTableRow>& rows, const std::string& id) {
    const auto found = std::find_if(rows.begin(), rows.end(), [&id](const auto& row) { return row.id == id; });
    return found == rows.end() ? nullptr : &*found;
}

void validateRows(const std::vector<DatabaseTableRow>& rows, std::vector<DatabaseBatchDiagnostic>& diagnostics) {
    std::set<std::string> ids;
    for (const auto& row : rows) {
        if (row.id.empty()) diagnostics.push_back({"stable_id_missing", row.id, "Imported rows require a stable id."});
        else if (!ids.insert(row.id).second)
            diagnostics.push_back({"stable_id_duplicate", row.id, "Imported rows contain a duplicate stable id."});
    }
}

nlohmann::json rowToJson(const DatabaseTableRow& row) {
    return {{"id", row.id}, {"name", row.name}, {"summary", row.summary}, {"fields", row.fields}, {"revision", row.revision}};
}

} // namespace

DatabaseBatchEditPlan DatabaseBatchEditService::preview(const DatabaseTableModel& table,
                                                        const DatabaseBatchEditRequest& request) {
    DatabaseBatchEditPlan plan{table.kind(), table.generation(), 0, 0, table.rows(), table.rows(), {}, 0};
    if (request.operation != DatabaseBatchOperation::Duplicate && (request.field.empty() || request.field == "id")) {
        plan.diagnostics.push_back({"field_not_editable", request.field, "Batch edits cannot change stable identity."});
        return plan;
    }
    for (const auto& id : request.target_ids) {
        if (table.find(id) == nullptr) plan.diagnostics.push_back({"target_missing", id, "Batch target does not exist."});
    }
    if (request.operation == DatabaseBatchOperation::Duplicate) {
        for (const auto& id : request.target_ids) {
            const auto* source = table.find(id);
            if (source == nullptr) continue;
            const auto mapping = request.duplicate_ids.find(id);
            if (mapping == request.duplicate_ids.end() || mapping->second.empty()) {
                plan.diagnostics.push_back({"duplicate_id_missing", id, "Duplicate operation requires a new stable id."});
                continue;
            }
            auto copy = *source;
            copy.id = mapping->second;
            copy.revision = 0;
            plan.after.push_back(std::move(copy));
        }
        validateRows(plan.after, plan.diagnostics);
        if (plan.diagnostics.empty()) plan.changed_rows = request.target_ids.size();
        return plan;
    }

    for (std::size_t index = 0; index < request.target_ids.size(); ++index) {
        auto* row = findRow(plan.after, request.target_ids[index]);
        if (row == nullptr) continue;
        auto* destination = editableField(*row, request.field);
        if (request.operation == DatabaseBatchOperation::Fill) {
            *destination = request.value;
        } else if (request.operation == DatabaseBatchOperation::NumericFormula) {
            const auto* source = existingField(*row, request.field);
            double number = 0.0;
            if (source == nullptr || !parseNumber(*source, number)) {
                plan.diagnostics.push_back({"numeric_value_invalid", row->id, "Formula target is not a finite number."});
                continue;
            }
            *destination = formatNumber(number * request.multiplier + request.offset);
        } else {
            const auto ratio = request.target_ids.size() <= 1 ? 0.0 :
                static_cast<double>(index) / static_cast<double>(request.target_ids.size() - 1);
            *destination = formatNumber(request.curve_start + (request.curve_end - request.curve_start) * ratio);
        }
        ++row->revision;
        ++plan.changed_rows;
    }
    if (!plan.diagnostics.empty()) plan.changed_rows = 0;
    return plan;
}

nlohmann::json DatabaseBatchEditService::exportRows(const DatabaseTableModel& table,
                                                    const std::vector<std::string>& ids) {
    nlohmann::json rows = nlohmann::json::array();
    const std::set<std::string> selected(ids.begin(), ids.end());
    for (const auto& row : table.rows()) {
        if (selected.empty() || selected.contains(row.id)) rows.push_back(rowToJson(row));
    }
    return {{"schema", "urpg.database_table_rows.v1"}, {"kind", databaseTableKindName(table.kind())}, {"rows", rows}};
}

DatabaseBatchEditPlan DatabaseBatchEditService::previewImport(const DatabaseTableModel& table,
                                                              const nlohmann::json& document, bool replace_all) {
    DatabaseBatchEditPlan plan{table.kind(), table.generation(), 0, 0, table.rows(), replace_all ?
        std::vector<DatabaseTableRow>{} : table.rows(), {}, 0};
    if (!document.is_object() || document.value("schema", "") != "urpg.database_table_rows.v1" ||
        !document.contains("rows") || !document["rows"].is_array()) {
        plan.diagnostics.push_back({"import_schema_invalid", "", "Import document has an unsupported schema."});
        return plan;
    }
    for (const auto& value : document["rows"]) {
        if (!value.is_object()) {
            plan.diagnostics.push_back({"import_row_invalid", "", "Import row must be an object."});
            continue;
        }
        DatabaseTableRow row;
        row.id = value.value("id", "");
        row.name = value.value("name", "");
        row.summary = value.value("summary", "");
        row.revision = value.value("revision", uint64_t{0});
        if (value.contains("fields") && value["fields"].is_object())
            row.fields = value["fields"].get<std::map<std::string, std::string>>();
        if (!replace_all) {
            if (auto* existing = findRow(plan.after, row.id)) *existing = row;
            else plan.after.push_back(row);
        } else plan.after.push_back(row);
        ++plan.changed_rows;
    }
    validateRows(plan.after, plan.diagnostics);
    if (!plan.diagnostics.empty()) plan.changed_rows = 0;
    return plan;
}

bool DatabaseBatchEditService::apply(DatabaseTableModel& table, DatabaseBatchEditPlan& plan) {
    if (!plan.valid() || table.kind() != plan.kind || table.generation() != plan.base_generation) return false;
    table.replaceRows(plan.after);
    plan.applied_generation = table.generation();
    return true;
}

bool DatabaseBatchEditService::undo(DatabaseTableModel& table, DatabaseBatchEditPlan& plan) {
    if (plan.applied_generation == 0 || table.generation() != plan.applied_generation) return false;
    table.replaceRows(plan.before);
    plan.undone_generation = table.generation();
    return true;
}

bool DatabaseBatchEditService::redo(DatabaseTableModel& table, DatabaseBatchEditPlan& plan) {
    if (plan.undone_generation == 0 || table.generation() != plan.undone_generation) return false;
    table.replaceRows(plan.after);
    plan.applied_generation = table.generation();
    plan.undone_generation = 0;
    return true;
}

} // namespace urpg::editor
