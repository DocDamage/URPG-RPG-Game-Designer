#pragma once

#include "engine/core/database/rpg_database.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace urpg::editor {

enum class DatabaseTableKind : uint8_t {
    Actors,
    Classes,
    Skills,
    Items,
    Equipment,
    Enemies,
    Encounters,
    States,
    Quests,
    Vendors,
    Recipes,
    Switches,
    Variables,
};

std::string_view databaseTableKindName(DatabaseTableKind kind);
const std::vector<DatabaseTableKind>& allDatabaseTableKinds();

struct DatabaseTableRow {
    std::string id;
    std::string name;
    std::string summary;
    std::map<std::string, std::string> fields;
    uint64_t revision = 0;
};

struct DatabaseTableWindow {
    DatabaseTableKind kind = DatabaseTableKind::Actors;
    std::size_t total_rows = 0;
    std::size_t matching_rows = 0;
    std::size_t first_row = 0;
    std::vector<DatabaseTableRow> rows;
};

class DatabaseTableModel {
public:
    explicit DatabaseTableModel(DatabaseTableKind kind = DatabaseTableKind::Actors);

    DatabaseTableKind kind() const { return kind_; }
    void replaceRows(std::vector<DatabaseTableRow> rows);
    void setSearch(std::string search);
    const std::string& search() const { return search_; }
    void setSort(std::string field, bool ascending = true);
    const std::string& sortField() const { return sort_field_; }
    bool sortAscending() const { return sort_ascending_; }

    DatabaseTableWindow visibleWindow(std::size_t first_row, std::size_t row_count) const;
    std::size_t rowCount() const { return rows_.size(); }
    std::size_t matchingRowCount() const { return projection_.size(); }
    const std::vector<DatabaseTableRow>& rows() const { return rows_; }
    uint64_t generation() const { return generation_; }

    bool select(std::string_view id);
    const std::optional<std::string>& selectedId() const { return selected_id_; }
    bool editField(std::string_view id, std::string_view field, std::string value);
    const DatabaseTableRow* find(std::string_view id) const;

private:
    void rebuildSearchDocuments();
    void rebuildProjection();

    DatabaseTableKind kind_;
    std::vector<DatabaseTableRow> rows_;
    std::vector<std::string> search_documents_;
    std::vector<std::size_t> projection_;
    std::string search_;
    std::string sort_field_ = "id";
    bool sort_ascending_ = true;
    std::optional<std::string> selected_id_;
    uint64_t generation_ = 0;
};

class ProjectDatabaseTableModel {
public:
    ProjectDatabaseTableModel();

    DatabaseTableModel& table(DatabaseTableKind kind);
    const DatabaseTableModel& table(DatabaseTableKind kind) const;
    void project(const database::RpgDatabase& database);
    std::size_t tableCount() const { return tables_.size(); }

private:
    std::map<DatabaseTableKind, DatabaseTableModel> tables_;
};

} // namespace urpg::editor
