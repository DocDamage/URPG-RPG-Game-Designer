#include "engine/core/events/event_edit_guard.h"
#include "engine/core/sync/source_authority.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Compat suite: raw commands are authoritative in compat mode", "[compat][authority]") {
    urpg::AuthorityTag tag;
    tag.project_mode = urpg::ProjectMode::Compat;
    tag.authoritative_kind = urpg::AuthorityKind::RawCommandList;

    const auto error = urpg::SourceAuthorityPolicy::Validate(tag, urpg::EditOperation::EditRawCommandList);
    REQUIRE_FALSE(error.has_value());
}

TEST_CASE("Compat suite: AST edits in compat mode emit authority diagnostics", "[compat][authority]") {
    const urpg::EventEditRequest request{
        .event_id = "evt_compat_001",
        .authority = urpg::AuthorityTag{urpg::ProjectMode::Compat, urpg::AuthorityKind::RawCommandList},
        .operation = urpg::EditOperation::EditUrpgAst,
        .timestamp_utc = "2026-03-04T00:00:00Z",
    };

    const auto result = urpg::EventEditGuard::ValidateAndDiagnose(request);
    REQUIRE_FALSE(result.allowed);
    REQUIRE(result.diagnostic_jsonl.find("\"event_authority\"") != std::string::npos);
    REQUIRE(result.diagnostic_jsonl.find("\"evt_compat_001\"") != std::string::npos);
    REQUIRE(result.diagnostic_jsonl.find("\"compat\"") != std::string::npos);
}
