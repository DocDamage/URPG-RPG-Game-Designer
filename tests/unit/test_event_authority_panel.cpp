#include "editor/diagnostics/event_authority_panel.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Event authority panel model builds visible rows from diagnostics", "[events][panel]") {
    const std::string jsonl =
        "{\"ts\":\"2026-03-04T00:00:00Z\",\"level\":\"warn\",\"subsystem\":\"event_authority\",\"event\":\"edit_rejected\",\"event_id\":\"evt_a\",\"block_id\":\"block_1\",\"mode\":\"compat\",\"operation\":\"edit_urpg_ast\",\"error_code\":\"read_only_derived_view\",\"message\":\"ast is read-only\"}\n"
        "{\"ts\":\"2026-03-04T00:00:01Z\",\"level\":\"warn\",\"subsystem\":\"event_authority\",\"event\":\"edit_rejected\",\"event_id\":\"evt_b\",\"mode\":\"mixed\",\"operation\":\"edit_raw_command_list\",\"error_code\":\"invalid_for_mode\",\"message\":\"raw rejected\"}";

    urpg::EventAuthorityPanelModel model;
    model.LoadFromJsonl(jsonl);

    const auto& rows = model.VisibleRows();
    REQUIRE(rows.size() == 2);
    REQUIRE(rows[0].event_id == "evt_a");
    REQUIRE(rows[0].block_id == "block_1");
    REQUIRE(rows[0].summary.find("read_only_derived_view") != std::string::npos);
}

TEST_CASE("Event authority panel model filters by event id", "[events][panel]") {
    const std::string jsonl =
        "{\"ts\":\"2026-03-04T00:00:00Z\",\"level\":\"warn\",\"subsystem\":\"event_authority\",\"event\":\"edit_rejected\",\"event_id\":\"evt_a\",\"block_id\":\"block_1\",\"mode\":\"compat\",\"operation\":\"edit_urpg_ast\",\"error_code\":\"read_only_derived_view\",\"message\":\"ast is read-only\"}\n"
        "{\"ts\":\"2026-03-04T00:00:01Z\",\"level\":\"warn\",\"subsystem\":\"event_authority\",\"event\":\"edit_rejected\",\"event_id\":\"evt_b\",\"mode\":\"mixed\",\"operation\":\"edit_raw_command_list\",\"error_code\":\"invalid_for_mode\",\"message\":\"raw rejected\"}";

    urpg::EventAuthorityPanelModel model;
    model.LoadFromJsonl(jsonl);
    model.SetFilter("evt_b");

    const auto& rows = model.VisibleRows();
    REQUIRE(rows.size() == 1);
    REQUIRE(rows[0].event_id == "evt_b");
    REQUIRE(rows[0].block_id.empty());
}

TEST_CASE("Event authority panel model filters by severity level", "[events][panel]") {
    const std::string jsonl =
        "{\"ts\":\"2026-03-04T00:00:00Z\",\"level\":\"warn\",\"subsystem\":\"event_authority\",\"event\":\"edit_rejected\",\"event_id\":\"evt_a\",\"block_id\":\"block_1\",\"mode\":\"compat\",\"operation\":\"edit_urpg_ast\",\"error_code\":\"read_only_derived_view\",\"message\":\"ast is read-only\"}\n"
        "{\"ts\":\"2026-03-04T00:00:01Z\",\"level\":\"error\",\"subsystem\":\"event_authority\",\"event\":\"edit_rejected\",\"event_id\":\"evt_b\",\"block_id\":\"block_2\",\"mode\":\"mixed\",\"operation\":\"edit_raw_command_list\",\"error_code\":\"invalid_for_mode\",\"message\":\"raw rejected\"}";

    urpg::EventAuthorityPanel panel;
    panel.ingestDiagnosticsJsonl(jsonl);
    panel.refresh();

    panel.setLevelFilter("error");
    panel.update();

    const auto& errorRows = panel.getModel().VisibleRows();
    REQUIRE(errorRows.size() == 1);
    REQUIRE(errorRows[0].event_id == "evt_b");
    REQUIRE(errorRows[0].level == "error");

    panel.setLevelFilter("warn");
    panel.update();

    const auto& warnRows = panel.getModel().VisibleRows();
    REQUIRE(warnRows.size() == 1);
    REQUIRE(warnRows[0].event_id == "evt_a");
    REQUIRE(warnRows[0].level == "warn");
}

TEST_CASE("Event authority panel model filters by mode", "[events][panel]") {
    const std::string jsonl =
        "{\"ts\":\"2026-03-04T00:00:00Z\",\"level\":\"warn\",\"subsystem\":\"event_authority\",\"event\":\"edit_rejected\",\"event_id\":\"evt_a\",\"block_id\":\"block_1\",\"mode\":\"compat\",\"operation\":\"edit_urpg_ast\",\"error_code\":\"read_only_derived_view\",\"message\":\"ast is read-only\"}\n"
        "{\"ts\":\"2026-03-04T00:00:01Z\",\"level\":\"error\",\"subsystem\":\"event_authority\",\"event\":\"edit_rejected\",\"event_id\":\"evt_b\",\"block_id\":\"block_2\",\"mode\":\"mixed\",\"operation\":\"edit_raw_command_list\",\"error_code\":\"invalid_for_mode\",\"message\":\"raw rejected\"}";

    urpg::EventAuthorityPanel panel;
    panel.ingestDiagnosticsJsonl(jsonl);
    panel.refresh();

    panel.setModeFilter("mixed");
    panel.update();

    const auto& mixedRows = panel.getModel().VisibleRows();
    REQUIRE(mixedRows.size() == 1);
    REQUIRE(mixedRows[0].event_id == "evt_b");
    REQUIRE(mixedRows[0].mode == "mixed");

    panel.setModeFilter("compat");
    panel.update();

    const auto& compatRows = panel.getModel().VisibleRows();
    REQUIRE(compatRows.size() == 1);
    REQUIRE(compatRows[0].event_id == "evt_a");
    REQUIRE(compatRows[0].mode == "compat");
}

TEST_CASE("Event authority panel model resolves one-click navigation target", "[events][panel]") {
    const std::string jsonl =
        "{\"ts\":\"2026-03-04T00:00:00Z\",\"level\":\"warn\",\"subsystem\":\"event_authority\",\"event\":\"edit_rejected\",\"event_id\":\"evt_jump\",\"block_id\":\"block_nav\",\"mode\":\"compat\",\"operation\":\"edit_urpg_ast\",\"error_code\":\"read_only_derived_view\",\"message\":\"ast is read-only\"}";

    urpg::EventAuthorityPanelModel model;
    model.LoadFromJsonl(jsonl);

    REQUIRE(model.SelectRow(0));
    const auto nav = model.SelectedNavigationTarget();
    REQUIRE(nav.has_value());
    REQUIRE(nav->event_id == "evt_jump");
    REQUIRE(nav->block_id == "block_nav");

    REQUIRE_FALSE(model.SelectRow(10));
    REQUIRE_FALSE(model.SelectedNavigationTarget().has_value());
}

TEST_CASE("Event authority panel controller ingests diagnostics and applies filter on refresh", "[events][panel][integration]") {
    const std::string jsonlA =
        "{\"ts\":\"2026-03-04T00:00:00Z\",\"level\":\"warn\",\"subsystem\":\"event_authority\",\"event\":\"edit_rejected\",\"event_id\":\"evt_a\",\"block_id\":\"block_1\",\"mode\":\"compat\",\"operation\":\"edit_urpg_ast\",\"error_code\":\"read_only_derived_view\",\"message\":\"ast is read-only\"}";
    const std::string jsonlB =
        "{\"ts\":\"2026-03-04T00:00:01Z\",\"level\":\"warn\",\"subsystem\":\"event_authority\",\"event\":\"edit_rejected\",\"event_id\":\"evt_b\",\"mode\":\"mixed\",\"operation\":\"edit_raw_command_list\",\"error_code\":\"invalid_for_mode\",\"message\":\"raw rejected\"}";

    urpg::EventAuthorityPanel panel;
    panel.ingestDiagnosticsJsonl(jsonlA);
    panel.ingestDiagnosticsJsonl(jsonlB);
    panel.refresh();

    const auto& rows = panel.getModel().VisibleRows();
    REQUIRE(rows.size() == 2);

    panel.setFilter("evt_b");
    panel.update();

    const auto& filteredRows = panel.getModel().VisibleRows();
    REQUIRE(filteredRows.size() == 1);
    REQUIRE(filteredRows[0].event_id == "evt_b");

    panel.clearDiagnostics();
    REQUIRE(panel.getModel().VisibleRows().empty());
}

TEST_CASE("Event authority panel visible render records snapshot", "[events][panel][render]") {
    const std::string jsonl =
        "{\"ts\":\"2026-03-04T00:00:00Z\",\"level\":\"warn\",\"subsystem\":\"event_authority\",\"event\":\"edit_rejected\",\"event_id\":\"evt_a\",\"block_id\":\"block_1\",\"mode\":\"compat\",\"operation\":\"edit_urpg_ast\",\"error_code\":\"read_only_derived_view\",\"message\":\"ast is read-only\"}\n"
        "{\"ts\":\"2026-03-04T00:00:01Z\",\"level\":\"error\",\"subsystem\":\"event_authority\",\"event\":\"edit_rejected\",\"event_id\":\"evt_b\",\"block_id\":\"block_2\",\"mode\":\"mixed\",\"operation\":\"edit_raw_command_list\",\"error_code\":\"invalid_for_mode\",\"message\":\"raw rejected\"}";

    urpg::EventAuthorityPanel panel;
    panel.ingestDiagnosticsJsonl(jsonl);
    panel.refresh();
    panel.setVisible(true);

    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE(panel.lastRenderSnapshot().visible_rows == 2);
    REQUIRE(panel.lastRenderSnapshot().warning_count == 1);
    REQUIRE(panel.lastRenderSnapshot().error_count == 1);
    REQUIRE(panel.lastRenderSnapshot().has_selection == false);
    REQUIRE(panel.lastRenderSnapshot().has_data);
    REQUIRE(panel.lastRenderSnapshot().visible_row_entries.size() == 2);
    REQUIRE(panel.lastRenderSnapshot().visible_row_entries[0].event_id == "evt_a");
    REQUIRE(panel.lastRenderSnapshot().visible_row_entries[1].event_id == "evt_b");
    REQUIRE(panel.lastRenderSnapshot().visible_row_entries[1].level == "error");
}

TEST_CASE("Event authority panel render snapshot exposes selection and filter state", "[events][panel][render]") {
    const std::string jsonl =
        "{\"ts\":\"2026-03-04T00:00:00Z\",\"level\":\"warn\",\"subsystem\":\"event_authority\",\"event\":\"edit_rejected\",\"event_id\":\"evt_a\",\"block_id\":\"block_1\",\"mode\":\"compat\",\"operation\":\"edit_urpg_ast\",\"error_code\":\"read_only_derived_view\",\"message\":\"ast is read-only\"}\n"
        "{\"ts\":\"2026-03-04T00:00:01Z\",\"level\":\"error\",\"subsystem\":\"event_authority\",\"event\":\"edit_rejected\",\"event_id\":\"evt_b\",\"block_id\":\"block_2\",\"mode\":\"mixed\",\"operation\":\"edit_raw_command_list\",\"error_code\":\"invalid_for_mode\",\"message\":\"raw rejected\"}";

    urpg::EventAuthorityPanel panel;
    panel.ingestDiagnosticsJsonl(jsonl);
    panel.setFilter("evt_b");
    panel.refresh();

    REQUIRE(panel.selectRow(0));
    REQUIRE(panel.selectedRowIndex().has_value());
    REQUIRE(panel.selectedRowIndex().value() == 0);

    const auto nav = panel.selectedNavigationTarget();
    REQUIRE(nav.has_value());
    REQUIRE(nav->event_id == "evt_b");
    REQUIRE(nav->block_id == "block_2");

    panel.render();

    REQUIRE(panel.lastRenderSnapshot().visible_rows == 1);
    REQUIRE(panel.lastRenderSnapshot().warning_count == 0);
    REQUIRE(panel.lastRenderSnapshot().error_count == 1);
    REQUIRE(panel.lastRenderSnapshot().has_selection);
    REQUIRE(panel.lastRenderSnapshot().selected_row_index.has_value());
    REQUIRE(panel.lastRenderSnapshot().selected_row_index.value() == 0);
    REQUIRE(panel.lastRenderSnapshot().selected_navigation_target.has_value());
    REQUIRE(panel.lastRenderSnapshot().selected_navigation_target->event_id == "evt_b");
    REQUIRE(panel.lastRenderSnapshot().selected_navigation_target->block_id == "block_2");
    REQUIRE(panel.lastRenderSnapshot().event_id_filter == "evt_b");
    REQUIRE(panel.lastRenderSnapshot().level_filter.empty());
    REQUIRE(panel.selectedRowIndex().has_value());
    REQUIRE(panel.selectedRowIndex().value() == 0);
    REQUIRE(panel.selectedNavigationTarget().has_value());
    REQUIRE(panel.selectedNavigationTarget()->event_id == "evt_b");
    REQUIRE(panel.selectedNavigationTarget()->block_id == "block_2");
}

TEST_CASE("Event authority panel render snapshot exposes level filter", "[events][panel][render][filter]") {
    const std::string jsonl =
        "{\"ts\":\"2026-03-04T00:00:00Z\",\"level\":\"warn\",\"subsystem\":\"event_authority\",\"event\":\"edit_rejected\",\"event_id\":\"evt_a\",\"block_id\":\"block_1\",\"mode\":\"compat\",\"operation\":\"edit_urpg_ast\",\"error_code\":\"read_only_derived_view\",\"message\":\"ast is read-only\"}\n"
        "{\"ts\":\"2026-03-04T00:00:01Z\",\"level\":\"error\",\"subsystem\":\"event_authority\",\"event\":\"edit_rejected\",\"event_id\":\"evt_b\",\"block_id\":\"block_2\",\"mode\":\"mixed\",\"operation\":\"edit_raw_command_list\",\"error_code\":\"invalid_for_mode\",\"message\":\"raw rejected\"}";

    urpg::EventAuthorityPanel panel;
    panel.ingestDiagnosticsJsonl(jsonl);
    panel.setLevelFilter("error");
    panel.refresh();
    panel.setVisible(true);
    panel.render();

    REQUIRE(panel.lastRenderSnapshot().visible_rows == 1);
    REQUIRE(panel.lastRenderSnapshot().warning_count == 0);
    REQUIRE(panel.lastRenderSnapshot().error_count == 1);
    REQUIRE(panel.lastRenderSnapshot().level_filter == "error");
    REQUIRE(panel.lastRenderSnapshot().visible_row_entries.size() == 1);
    REQUIRE(panel.lastRenderSnapshot().visible_row_entries[0].event_id == "evt_b");
}

TEST_CASE("Event authority panel render snapshot exposes mode filter", "[events][panel][render][filter]") {
    const std::string jsonl =
        "{\"ts\":\"2026-03-04T00:00:00Z\",\"level\":\"warn\",\"subsystem\":\"event_authority\",\"event\":\"edit_rejected\",\"event_id\":\"evt_a\",\"block_id\":\"block_1\",\"mode\":\"compat\",\"operation\":\"edit_urpg_ast\",\"error_code\":\"read_only_derived_view\",\"message\":\"ast is read-only\"}\n"
        "{\"ts\":\"2026-03-04T00:00:01Z\",\"level\":\"error\",\"subsystem\":\"event_authority\",\"event\":\"edit_rejected\",\"event_id\":\"evt_b\",\"block_id\":\"block_2\",\"mode\":\"mixed\",\"operation\":\"edit_raw_command_list\",\"error_code\":\"invalid_for_mode\",\"message\":\"raw rejected\"}";

    urpg::EventAuthorityPanel panel;
    panel.ingestDiagnosticsJsonl(jsonl);
    panel.setModeFilter("mixed");
    panel.refresh();
    panel.setVisible(true);
    panel.render();

    REQUIRE(panel.lastRenderSnapshot().visible_rows == 1);
    REQUIRE(panel.lastRenderSnapshot().mode_filter == "mixed");
    REQUIRE(panel.lastRenderSnapshot().visible_row_entries.size() == 1);
    REQUIRE(panel.lastRenderSnapshot().visible_row_entries[0].event_id == "evt_b");
    REQUIRE(panel.lastRenderSnapshot().visible_row_entries[0].mode == "mixed");
}

TEST_CASE("Event authority panel supports next/previous row navigation with selected-row detail", "[events][panel][render][navigation]") {
    const std::string jsonl =
        "{\"ts\":\"2026-03-04T00:00:00Z\",\"level\":\"warn\",\"subsystem\":\"event_authority\",\"event\":\"edit_rejected\",\"event_id\":\"evt_a\",\"block_id\":\"block_1\",\"mode\":\"compat\",\"operation\":\"edit_urpg_ast\",\"error_code\":\"read_only_derived_view\",\"message\":\"ast is read-only\"}\n"
        "{\"ts\":\"2026-03-04T00:00:01Z\",\"level\":\"error\",\"subsystem\":\"event_authority\",\"event\":\"edit_rejected\",\"event_id\":\"evt_b\",\"block_id\":\"block_2\",\"mode\":\"mixed\",\"operation\":\"edit_raw_command_list\",\"error_code\":\"invalid_for_mode\",\"message\":\"raw rejected\"}";

    urpg::EventAuthorityPanel panel;
    panel.ingestDiagnosticsJsonl(jsonl);
    panel.refresh();
    panel.setVisible(true);

    REQUIRE(panel.canSelectNextRow());
    REQUIRE(panel.canSelectPreviousRow());
    REQUIRE(panel.selectNextRow());
    REQUIRE(panel.selectedRowIndex().has_value());
    REQUIRE(panel.selectedRowIndex().value() == 0);
    REQUIRE(panel.canSelectNextRow());
    REQUIRE_FALSE(panel.canSelectPreviousRow());

    REQUIRE(panel.selectNextRow());
    REQUIRE(panel.selectedRowIndex().value() == 1);
    REQUIRE_FALSE(panel.canSelectNextRow());
    REQUIRE(panel.canSelectPreviousRow());

    panel.render();

    REQUIRE(panel.lastRenderSnapshot().selected_row.has_value());
    REQUIRE(panel.lastRenderSnapshot().selected_row->event_id == "evt_b");
    REQUIRE(panel.lastRenderSnapshot().selected_row->block_id == "block_2");
    REQUIRE(panel.lastRenderSnapshot().selected_row->level == "error");
    REQUIRE(panel.lastRenderSnapshot().selected_row->operation == "edit_raw_command_list");
    REQUIRE(panel.lastRenderSnapshot().selected_row->summary.find("invalid_for_mode") != std::string::npos);
    REQUIRE_FALSE(panel.lastRenderSnapshot().can_select_next_row);
    REQUIRE(panel.lastRenderSnapshot().can_select_previous_row);

    REQUIRE(panel.selectPreviousRow());
    REQUIRE(panel.selectedRowIndex().value() == 0);
}
