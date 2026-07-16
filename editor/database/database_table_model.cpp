#include "editor/database/database_table_model.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <utility>

namespace urpg::editor {

namespace {

std::string lower(std::string_view value) {
    std::string result(value);
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return result;
}

std::string_view valueForField(const DatabaseTableRow& row, std::string_view field) {
    if (field == "id") return row.id;
    if (field == "name") return row.name;
    if (field == "summary") return row.summary;
    const auto found = row.fields.find(std::string(field));
    return found == row.fields.end() ? std::string_view{} : std::string_view(found->second);
}

std::string searchDocument(const DatabaseTableRow& row) {
    std::string document;
    const auto append = [&document](std::string_view value) {
        if (!document.empty()) document.push_back('\n');
        document += lower(value);
    };
    append(row.id);
    append(row.name);
    append(row.summary);
    for (const auto& [key, value] : row.fields) {
        append(key);
        append(value);
    }
    return document;
}

} // namespace

std::string_view databaseTableKindName(DatabaseTableKind kind) {
    switch (kind) {
    case DatabaseTableKind::Actors: return "Actors";
    case DatabaseTableKind::Classes: return "Classes";
    case DatabaseTableKind::Skills: return "Skills";
    case DatabaseTableKind::Items: return "Items";
    case DatabaseTableKind::Equipment: return "Equipment";
    case DatabaseTableKind::Enemies: return "Enemies";
    case DatabaseTableKind::Encounters: return "Encounters";
    case DatabaseTableKind::States: return "States";
    case DatabaseTableKind::Quests: return "Quests";
    case DatabaseTableKind::Vendors: return "Vendors";
    case DatabaseTableKind::Recipes: return "Recipes";
    case DatabaseTableKind::Switches: return "Switches";
    case DatabaseTableKind::Variables: return "Variables";
    }
    return "Unknown";
}

const std::vector<DatabaseTableKind>& allDatabaseTableKinds() {
    static const std::vector<DatabaseTableKind> kinds{
        DatabaseTableKind::Actors, DatabaseTableKind::Classes, DatabaseTableKind::Skills,
        DatabaseTableKind::Items, DatabaseTableKind::Equipment, DatabaseTableKind::Enemies,
        DatabaseTableKind::Encounters, DatabaseTableKind::States, DatabaseTableKind::Quests,
        DatabaseTableKind::Vendors, DatabaseTableKind::Recipes, DatabaseTableKind::Switches,
        DatabaseTableKind::Variables,
    };
    return kinds;
}

DatabaseTableModel::DatabaseTableModel(DatabaseTableKind kind) : kind_(kind) {}

void DatabaseTableModel::replaceRows(std::vector<DatabaseTableRow> rows) {
    std::sort(rows.begin(), rows.end(), [](const auto& left, const auto& right) { return left.id < right.id; });
    const auto invalid = std::adjacent_find(rows.begin(), rows.end(), [](const auto& left, const auto& right) {
        return left.id.empty() || left.id == right.id;
    });
    if ((!rows.empty() && rows.back().id.empty()) || invalid != rows.end()) {
        throw std::invalid_argument("Database table rows require unique, non-empty stable ids.");
    }
    rows_ = std::move(rows);
    ++generation_;
    rebuildSearchDocuments();
    if (selected_id_ && find(*selected_id_) == nullptr) selected_id_.reset();
    rebuildProjection();
}

void DatabaseTableModel::setSearch(std::string search) {
    search_ = std::move(search);
    rebuildProjection();
}

void DatabaseTableModel::setSort(std::string field, bool ascending) {
    sort_field_ = field.empty() ? "id" : std::move(field);
    sort_ascending_ = ascending;
    rebuildProjection();
}

DatabaseTableWindow DatabaseTableModel::visibleWindow(std::size_t first_row, std::size_t row_count) const {
    DatabaseTableWindow window{kind_, rows_.size(), projection_.size(), std::min(first_row, projection_.size()), {}};
    const auto end = std::min(window.first_row + row_count, projection_.size());
    window.rows.reserve(end - window.first_row);
    for (auto index = window.first_row; index < end; ++index) {
        window.rows.push_back(rows_[projection_[index]]);
    }
    return window;
}

bool DatabaseTableModel::select(std::string_view id) {
    if (find(id) == nullptr) return false;
    selected_id_ = std::string(id);
    return true;
}

bool DatabaseTableModel::editField(std::string_view id, std::string_view field, std::string value) {
    const auto found = std::find_if(rows_.begin(), rows_.end(), [id](const auto& row) { return row.id == id; });
    if (found == rows_.end() || field.empty() || field == "id") return false;
    if (field == "name") found->name = std::move(value);
    else if (field == "summary") found->summary = std::move(value);
    else found->fields[std::string(field)] = std::move(value);
    ++found->revision;
    ++generation_;
    search_documents_[static_cast<std::size_t>(std::distance(rows_.begin(), found))] = searchDocument(*found);
    rebuildProjection();
    return true;
}

const DatabaseTableRow* DatabaseTableModel::find(std::string_view id) const {
    const auto found = std::lower_bound(rows_.begin(), rows_.end(), id, [](const auto& row, std::string_view value) {
        return row.id < value;
    });
    return found != rows_.end() && found->id == id ? &*found : nullptr;
}

void DatabaseTableModel::rebuildSearchDocuments() {
    search_documents_.clear();
    search_documents_.reserve(rows_.size());
    for (const auto& row : rows_) search_documents_.push_back(searchDocument(row));
}

void DatabaseTableModel::rebuildProjection() {
    projection_.clear();
    const auto needle = lower(search_);
    for (std::size_t index = 0; index < rows_.size(); ++index) {
        if (needle.empty() || search_documents_[index].find(needle) != std::string::npos) projection_.push_back(index);
    }
    std::vector<std::string> sort_values(rows_.size());
    for (const auto index : projection_) sort_values[index] = lower(valueForField(rows_[index], sort_field_));
    std::stable_sort(projection_.begin(), projection_.end(), [this, &sort_values](std::size_t left, std::size_t right) {
        const auto& left_value = sort_values[left];
        const auto& right_value = sort_values[right];
        if (left_value == right_value) return rows_[left].id < rows_[right].id;
        return sort_ascending_ ? left_value < right_value : left_value > right_value;
    });
}

ProjectDatabaseTableModel::ProjectDatabaseTableModel() {
    for (const auto kind : allDatabaseTableKinds()) tables_.emplace(kind, DatabaseTableModel(kind));
}

DatabaseTableModel& ProjectDatabaseTableModel::table(DatabaseTableKind kind) {
    return tables_.at(kind);
}

const DatabaseTableModel& ProjectDatabaseTableModel::table(DatabaseTableKind kind) const {
    return tables_.at(kind);
}

void ProjectDatabaseTableModel::project(const database::RpgDatabase& database) {
    std::vector<DatabaseTableRow> actors;
    actors.reserve(database.actors().size());
    for (const auto& [id, actor] : database.actors()) {
        actors.push_back({id, actor.name, actor.class_id,
                          {{"class_id", actor.class_id}, {"max_hp", std::to_string(actor.max_hp)},
                           {"attack", std::to_string(actor.attack)}}, 0});
    }
    table(DatabaseTableKind::Actors).replaceRows(std::move(actors));

    std::vector<DatabaseTableRow> items;
    items.reserve(database.items().size());
    for (const auto& [id, item] : database.items()) {
        items.push_back({id, item.name, "", {{"price", std::to_string(item.price)}}, 0});
    }
    table(DatabaseTableKind::Items).replaceRows(std::move(items));
}

} // namespace urpg::editor
