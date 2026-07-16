#include "editor/database/database_panel.h"
#include "editor/database/database_table_model.h"

#include <catch2/catch_test_macros.hpp>

#include <iomanip>
#include <sstream>

namespace {

std::string paddedId(int value) {
    std::ostringstream stream;
    stream << "actor." << std::setw(6) << std::setfill('0') << value;
    return stream.str();
}

} // namespace

TEST_CASE("Project database exposes every product table through bounded virtual windows",
          "[editor][database][virtualized][pcq450]") {
    urpg::editor::ProjectDatabaseTableModel project;
    const auto& kinds = urpg::editor::allDatabaseTableKinds();
    REQUIRE(kinds.size() == 13);
    REQUIRE(project.tableCount() == kinds.size());

    for (const auto kind : kinds) {
        auto& table = project.table(kind);
        table.replaceRows({{"record.one", std::string(urpg::editor::databaseTableKindName(kind)), "summary", {}, 0}});
        const auto window = table.visibleWindow(0, 1);
        REQUIRE(window.kind == kind);
        REQUIRE(window.total_rows == 1);
        REQUIRE(window.matching_rows == 1);
        REQUIRE(window.rows.front().id == "record.one");
    }
}

TEST_CASE("Large database tables keep filtering sorting selection and editing deterministic",
          "[editor][database][virtualized][pcq450]") {
    urpg::editor::DatabaseTableModel table(urpg::editor::DatabaseTableKind::Actors);
    std::vector<urpg::editor::DatabaseTableRow> rows;
    rows.reserve(50000);
    for (int index = 0; index < 50000; ++index) {
        rows.push_back({paddedId(index), "Actor " + std::to_string(index), index % 1000 == 0 ? "boss" : "town",
                        {{"level", std::to_string(index % 99)}, {"role", index % 2 == 0 ? "guard" : "healer"}}, 0});
    }
    table.replaceRows(std::move(rows));
    table.setSearch("boss");
    table.setSort("id", false);

    const auto window = table.visibleWindow(10, 12);
    REQUIRE(window.total_rows == 50000);
    REQUIRE(window.matching_rows == 50);
    REQUIRE(window.first_row == 10);
    REQUIRE(window.rows.size() == 12);
    REQUIRE(window.rows.front().name == "Actor 39000");

    const auto selected = window.rows[3].id;
    REQUIRE(table.select(selected));
    REQUIRE(table.editField(selected, "name", "Edited Boss"));
    REQUIRE(table.selectedId() == selected);
    REQUIRE(table.find(selected)->name == "Edited Boss");
    REQUIRE(table.find(selected)->revision == 1);
    REQUIRE(table.visibleWindow(0, 50000).rows.size() == 50);
    REQUIRE_FALSE(table.editField(selected, "id", "unstable"));
    REQUIRE_FALSE(table.select("actor.missing"));
}

TEST_CASE("Database panel projects native actor and item authorities into table views",
          "[editor][database][virtualized][pcq450]") {
    urpg::database::RpgDatabase database;
    database.upsertActor({"actor.hero", "Hero", "class.hero", 100, 20});
    database.upsertItem({"item.potion", "Potion", 50, {"healing"}});

    urpg::editor::DatabasePanel panel;
    panel.setDatabase(database);
    panel.render();

    REQUIRE(panel.lastRenderSnapshot().table_count == 13);
    REQUIRE(panel.tables().table(urpg::editor::DatabaseTableKind::Actors).find("actor.hero")->name == "Hero");
    const auto* item = panel.tables().table(urpg::editor::DatabaseTableKind::Items).find("item.potion");
    REQUIRE(item->fields.at("price") == "50");
}

TEST_CASE("Database table rejects rows without unique stable ids", "[editor][database][virtualized][pcq450]") {
    urpg::editor::DatabaseTableModel table;
    REQUIRE_THROWS_AS(table.replaceRows({{"same", "One", "", {}, 0}, {"same", "Two", "", {}, 0}}),
                      std::invalid_argument);
    REQUIRE_THROWS_AS(table.replaceRows({{"", "Missing", "", {}, 0}}), std::invalid_argument);
}
