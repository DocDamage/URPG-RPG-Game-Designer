#include "engine/core/sync/source_authority.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Compat mode allows raw edits and rejects AST edits", "[authority]") {
    const urpg::AuthorityTag tag{.project_mode = urpg::ProjectMode::Compat, .authoritative_kind = std::nullopt};

    REQUIRE_FALSE(urpg::SourceAuthorityPolicy::Validate(tag, urpg::EditOperation::EditRawCommandList).has_value());

    const auto error = urpg::SourceAuthorityPolicy::Validate(tag, urpg::EditOperation::EditUrpgAst);
    REQUIRE(error.has_value());
    REQUIRE(error->code == urpg::AuthorityErrorCode::ReadOnlyDerivedView);
}

TEST_CASE("Native mode allows AST edits and rejects raw edits", "[authority]") {
    const urpg::AuthorityTag tag{.project_mode = urpg::ProjectMode::Native, .authoritative_kind = std::nullopt};

    REQUIRE_FALSE(urpg::SourceAuthorityPolicy::Validate(tag, urpg::EditOperation::EditUrpgAst).has_value());

    const auto error = urpg::SourceAuthorityPolicy::Validate(tag, urpg::EditOperation::EditRawCommandList);
    REQUIRE(error.has_value());
    REQUIRE(error->code == urpg::AuthorityErrorCode::InvalidForMode);
}

TEST_CASE("Mixed mode requires explicit authority tag", "[authority]") {
    const urpg::AuthorityTag tag{.project_mode = urpg::ProjectMode::Mixed, .authoritative_kind = std::nullopt};

    const auto error = urpg::SourceAuthorityPolicy::Validate(tag, urpg::EditOperation::EditRawCommandList);
    REQUIRE(error.has_value());
    REQUIRE(error->code == urpg::AuthorityErrorCode::MissingAuthorityTag);
}

TEST_CASE("Mixed mode enforces per-block authority", "[authority]") {
    const urpg::AuthorityTag raw_authority{
        .project_mode = urpg::ProjectMode::Mixed,
        .authoritative_kind = urpg::AuthorityKind::RawCommandList
    };

    const auto raw_edit = urpg::SourceAuthorityPolicy::Validate(raw_authority, urpg::EditOperation::EditRawCommandList);
    REQUIRE_FALSE(raw_edit.has_value());

    const auto ast_edit_denied = urpg::SourceAuthorityPolicy::Validate(raw_authority, urpg::EditOperation::EditUrpgAst);
    REQUIRE(ast_edit_denied.has_value());
    REQUIRE(ast_edit_denied->code == urpg::AuthorityErrorCode::ReadOnlyDerivedView);

    const urpg::AuthorityTag ast_authority{
        .project_mode = urpg::ProjectMode::Mixed,
        .authoritative_kind = urpg::AuthorityKind::UrpgAst
    };

    const auto ast_edit = urpg::SourceAuthorityPolicy::Validate(ast_authority, urpg::EditOperation::EditUrpgAst);
    REQUIRE_FALSE(ast_edit.has_value());

    const auto raw_edit_denied = urpg::SourceAuthorityPolicy::Validate(ast_authority, urpg::EditOperation::EditRawCommandList);
    REQUIRE(raw_edit_denied.has_value());
    REQUIRE(raw_edit_denied->code == urpg::AuthorityErrorCode::ReadOnlyDerivedView);
}
