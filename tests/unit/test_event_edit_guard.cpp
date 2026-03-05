#include "engine/core/events/event_edit_guard.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

TEST_CASE("Event edit guard allows valid edits without diagnostics", "[event][authority]") {
    urpg::EventEditRequest request;
    request.event_id = "evt_001";
    request.authority = {
        .project_mode = urpg::ProjectMode::Native,
        .authoritative_kind = std::nullopt
    };
    request.operation = urpg::EditOperation::EditUrpgAst;
    request.timestamp_utc = "2026-03-04T10:00:00Z";

    const auto result = urpg::EventEditGuard::ValidateAndDiagnose(request);
    REQUIRE(result.allowed);
    REQUIRE(result.diagnostic_jsonl.empty());
}

TEST_CASE("Event edit guard emits structured diagnostics on rejected edits", "[event][authority]") {
    urpg::EventEditRequest request;
    request.event_id = "evt_compat_1";
    request.authority = {
        .project_mode = urpg::ProjectMode::Compat,
        .authoritative_kind = std::nullopt
    };
    request.operation = urpg::EditOperation::EditUrpgAst;
    request.timestamp_utc = "2026-03-04T11:00:00Z";

    const auto result = urpg::EventEditGuard::ValidateAndDiagnose(request);
    REQUIRE_FALSE(result.allowed);
    REQUIRE_FALSE(result.diagnostic_jsonl.empty());

    const auto json = nlohmann::json::parse(result.diagnostic_jsonl);
    REQUIRE(json["event"] == "edit_rejected");
    REQUIRE(json["event_id"] == "evt_compat_1");
    REQUIRE(json["mode"] == "compat");
    REQUIRE(json["operation"] == "edit_urpg_ast");
    REQUIRE(json["error_code"] == "read_only_derived_view");
}
