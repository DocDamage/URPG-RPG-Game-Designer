#pragma once

#include "editor/database/database_table_model.h"

#include <nlohmann/json.hpp>

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace urpg::editor {

enum class DatabaseBatchOperation : uint8_t { Fill, NumericFormula, LinearCurve, Duplicate };

struct DatabaseBatchEditRequest {
    DatabaseBatchOperation operation = DatabaseBatchOperation::Fill;
    std::vector<std::string> target_ids;
    std::string field;
    std::string value;
    double multiplier = 1.0;
    double offset = 0.0;
    double curve_start = 0.0;
    double curve_end = 0.0;
    std::map<std::string, std::string> duplicate_ids;
};

struct DatabaseBatchDiagnostic {
    std::string code;
    std::string row_id;
    std::string message;
};

struct DatabaseBatchEditPlan {
    DatabaseTableKind kind = DatabaseTableKind::Actors;
    uint64_t base_generation = 0;
    uint64_t applied_generation = 0;
    uint64_t undone_generation = 0;
    std::vector<DatabaseTableRow> before;
    std::vector<DatabaseTableRow> after;
    std::vector<DatabaseBatchDiagnostic> diagnostics;
    std::size_t changed_rows = 0;

    bool valid() const { return diagnostics.empty() && changed_rows > 0; }
};

class DatabaseBatchEditService {
public:
    static DatabaseBatchEditPlan preview(const DatabaseTableModel& table, const DatabaseBatchEditRequest& request);
    static DatabaseBatchEditPlan previewImport(const DatabaseTableModel& table, const nlohmann::json& document,
                                               bool replace_all);
    static nlohmann::json exportRows(const DatabaseTableModel& table, const std::vector<std::string>& ids = {});

    static bool apply(DatabaseTableModel& table, DatabaseBatchEditPlan& plan);
    static bool undo(DatabaseTableModel& table, DatabaseBatchEditPlan& plan);
    static bool redo(DatabaseTableModel& table, DatabaseBatchEditPlan& plan);
};

} // namespace urpg::editor
