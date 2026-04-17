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
}
