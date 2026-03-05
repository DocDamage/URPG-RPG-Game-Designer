#include "editor/diagnostics/event_authority_diagnostics.h"
#include "engine/core/events/event_edit_guard.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Event edit guard diagnostic includes block id for navigation", "[events][diagnostics]") {
    const urpg::EventEditRequest request{
        .event_id = "evt_100",
        .block_id = "block_7",
        .authority = urpg::AuthorityTag{urpg::ProjectMode::Compat, urpg::AuthorityKind::RawCommandList},
        .operation = urpg::EditOperation::EditUrpgAst,
        .timestamp_utc = "2026-03-04T00:00:00Z",
    };

    const auto result = urpg::EventEditGuard::ValidateAndDiagnose(request);
    REQUIRE_FALSE(result.allowed);
    REQUIRE(result.diagnostic_jsonl.find("\"event_id\":\"evt_100\"") != std::string::npos);
    REQUIRE(result.diagnostic_jsonl.find("\"block_id\":\"block_7\"") != std::string::npos);
}

TEST_CASE("Event authority diagnostics parser filters and maps navigation targets", "[events][diagnostics]") {
    const std::string jsonl =
        "{\"ts\":\"2026-03-04T00:00:00Z\",\"level\":\"warn\",\"subsystem\":\"event_authority\",\"event\":\"edit_rejected\",\"event_id\":\"evt_1\",\"block_id\":\"block_a\",\"mode\":\"compat\",\"operation\":\"edit_urpg_ast\",\"error_code\":\"read_only_derived_view\",\"message\":\"AST read-only in compat\"}\n"
        "{\"ts\":\"2026-03-04T00:00:01Z\",\"level\":\"info\",\"subsystem\":\"other\",\"event\":\"ignored\"}\n"
        "not-json\n"
        "{\"ts\":\"2026-03-04T00:00:02Z\",\"level\":\"warn\",\"subsystem\":\"event_authority\",\"event\":\"edit_rejected\",\"event_id\":\"evt_2\",\"mode\":\"mixed\",\"operation\":\"edit_raw_command_list\",\"error_code\":\"invalid_for_mode\",\"message\":\"raw rejected\"}";

    const auto entries = urpg::EventAuthorityDiagnosticsIndex::ParseJsonl(jsonl);
    REQUIRE(entries.size() == 2);
    REQUIRE(entries[0].event_id == "evt_1");
    REQUIRE(entries[0].block_id == "block_a");
    REQUIRE(entries[1].event_id == "evt_2");
    REQUIRE(entries[1].block_id.empty());

    const auto nav = urpg::EventAuthorityDiagnosticsIndex::FindNavigationTarget(entries, "evt_1");
    REQUIRE(nav.has_value());
    REQUIRE(nav->event_id == "evt_1");
    REQUIRE(nav->block_id == "block_a");
}
