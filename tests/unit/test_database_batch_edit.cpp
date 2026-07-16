#include "editor/database/database_batch_edit.h"

#include <catch2/catch_test_macros.hpp>

namespace {

urpg::editor::DatabaseTableModel sampleTable() {
    urpg::editor::DatabaseTableModel table(urpg::editor::DatabaseTableKind::Items);
    table.replaceRows({
        {"item.1", "Potion", "", {{"price", "10"}}, 0},
        {"item.2", "Ether", "", {{"price", "20"}}, 0},
        {"item.3", "Elixir", "", {{"price", "bad"}}, 0},
    });
    return table;
}

} // namespace

TEST_CASE("Database batch preview reports every invalid row and applies nothing", "[editor][database][batch][pcq451]") {
    auto table = sampleTable();
    urpg::editor::DatabaseBatchEditRequest request;
    request.operation = urpg::editor::DatabaseBatchOperation::NumericFormula;
    request.target_ids = {"item.3", "item.missing"};
    request.field = "price";
    request.multiplier = 2.0;
    auto plan = urpg::editor::DatabaseBatchEditService::preview(table, request);
    REQUIRE_FALSE(plan.valid());
    REQUIRE(plan.diagnostics.size() == 2);
    REQUIRE(plan.diagnostics[0].code == "target_missing");
    REQUIRE(plan.diagnostics[1].code == "numeric_value_invalid");
    REQUIRE_FALSE(urpg::editor::DatabaseBatchEditService::apply(table, plan));
    REQUIRE(table.find("item.3")->fields.at("price") == "bad");

    request.target_ids = {"item.1", "item.3"};
    plan = urpg::editor::DatabaseBatchEditService::preview(table, request);
    REQUIRE_FALSE(plan.valid());
    REQUIRE(plan.diagnostics.size() == 1);
    REQUIRE(plan.diagnostics.front().row_id == "item.3");
    REQUIRE(plan.changed_rows == 0);
    REQUIRE(table.find("item.1")->fields.at("price") == "10");
}

TEST_CASE("Fill formulas and curves are atomic undoable and stale-plan safe", "[editor][database][batch][pcq451]") {
    auto table = sampleTable();
    urpg::editor::DatabaseBatchEditRequest formula;
    formula.operation = urpg::editor::DatabaseBatchOperation::NumericFormula;
    formula.target_ids = {"item.1", "item.2"};
    formula.field = "price";
    formula.multiplier = 1.5;
    formula.offset = 5.0;
    auto plan = urpg::editor::DatabaseBatchEditService::preview(table, formula);
    REQUIRE(plan.valid());
    REQUIRE(plan.changed_rows == 2);
    REQUIRE(urpg::editor::DatabaseBatchEditService::apply(table, plan));
    REQUIRE(table.find("item.1")->fields.at("price") == "20");
    REQUIRE(table.find("item.2")->fields.at("price") == "35");
    REQUIRE(urpg::editor::DatabaseBatchEditService::undo(table, plan));
    REQUIRE(table.find("item.1")->fields.at("price") == "10");
    REQUIRE(urpg::editor::DatabaseBatchEditService::redo(table, plan));
    REQUIRE(table.find("item.2")->fields.at("price") == "35");

    urpg::editor::DatabaseBatchEditRequest curve;
    curve.operation = urpg::editor::DatabaseBatchOperation::LinearCurve;
    curve.target_ids = {"item.1", "item.2", "item.3"};
    curve.field = "price";
    curve.curve_start = 100.0;
    curve.curve_end = 200.0;
    auto stale = urpg::editor::DatabaseBatchEditService::preview(table, curve);
    REQUIRE(stale.valid());
    REQUIRE(table.editField("item.1", "name", "Changed elsewhere"));
    REQUIRE_FALSE(urpg::editor::DatabaseBatchEditService::apply(table, stale));
}

TEST_CASE("Duplicate and import preflight preserve stable identity", "[editor][database][batch][pcq451]") {
    auto table = sampleTable();
    urpg::editor::DatabaseBatchEditRequest duplicate;
    duplicate.operation = urpg::editor::DatabaseBatchOperation::Duplicate;
    duplicate.target_ids = {"item.1", "item.2"};
    duplicate.duplicate_ids = {{"item.1", "item.10"}, {"item.2", "item.20"}};
    auto plan = urpg::editor::DatabaseBatchEditService::preview(table, duplicate);
    REQUIRE(plan.valid());
    REQUIRE(plan.changed_rows == 2);
    REQUIRE(urpg::editor::DatabaseBatchEditService::apply(table, plan));
    REQUIRE(table.find("item.10")->name == "Potion");
    REQUIRE(table.find("item.20")->revision == 0);

    auto exported = urpg::editor::DatabaseBatchEditService::exportRows(table, {"item.10", "item.20"});
    REQUIRE(exported["rows"].size() == 2);
    auto imported = urpg::editor::DatabaseBatchEditService::previewImport(table, exported, true);
    REQUIRE(imported.valid());
    REQUIRE(imported.after.size() == 2);

    exported["rows"].push_back(exported["rows"].front());
    const auto invalid = urpg::editor::DatabaseBatchEditService::previewImport(table, exported, true);
    REQUIRE_FALSE(invalid.valid());
    REQUIRE(invalid.diagnostics.size() == 1);
    REQUIRE(invalid.diagnostics.front().code == "stable_id_duplicate");
}

TEST_CASE("Batch fill edits names but refuses identity", "[editor][database][batch][pcq451]") {
    auto table = sampleTable();
    urpg::editor::DatabaseBatchEditRequest fill;
    fill.target_ids = {"item.1", "item.2"};
    fill.field = "name";
    fill.value = "Consumable";
    auto plan = urpg::editor::DatabaseBatchEditService::preview(table, fill);
    REQUIRE(plan.valid());
    REQUIRE(urpg::editor::DatabaseBatchEditService::apply(table, plan));
    REQUIRE(table.find("item.1")->name == "Consumable");
    REQUIRE(table.find("item.2")->name == "Consumable");

    fill.field = "id";
    plan = urpg::editor::DatabaseBatchEditService::preview(table, fill);
    REQUIRE_FALSE(plan.valid());
    REQUIRE(plan.diagnostics.front().code == "field_not_editable");
}
