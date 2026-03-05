#include "engine/core/sync/source_authority.h"

namespace urpg {

std::optional<AuthorityError> SourceAuthorityPolicy::Validate(const AuthorityTag& tag, EditOperation operation) {
    switch (tag.project_mode) {
        case ProjectMode::Compat:
            return ValidateCompat(operation);
        case ProjectMode::Native:
            return ValidateNative(operation);
        case ProjectMode::Mixed:
            return ValidateMixed(tag, operation);
        default:
            return AuthorityError{AuthorityErrorCode::InvalidForMode, "unknown project mode"};
    }
}

std::optional<AuthorityError> SourceAuthorityPolicy::ValidateCompat(EditOperation operation) {
    if (operation == EditOperation::EditUrpgAst) {
        return AuthorityError{AuthorityErrorCode::ReadOnlyDerivedView, "URPG AST is read-only in Compat mode; edit raw command list"};
    }
    return std::nullopt;
}

std::optional<AuthorityError> SourceAuthorityPolicy::ValidateNative(EditOperation operation) {
    if (operation == EditOperation::EditRawCommandList) {
        return AuthorityError{AuthorityErrorCode::InvalidForMode, "Raw command list does not exist in Native mode"};
    }
    return std::nullopt;
}

std::optional<AuthorityError> SourceAuthorityPolicy::ValidateMixed(const AuthorityTag& tag, EditOperation operation) {
    if (!tag.authoritative_kind.has_value()) {
        return AuthorityError{AuthorityErrorCode::MissingAuthorityTag, "Mixed mode event block requires explicit authority tag"};
    }

    const AuthorityKind authority = tag.authoritative_kind.value();
    if (authority == AuthorityKind::RawCommandList && operation == EditOperation::EditUrpgAst) {
        return AuthorityError{AuthorityErrorCode::ReadOnlyDerivedView, "Event block is Raw-authoritative; URPG AST is derived"};
    }

    if (authority == AuthorityKind::UrpgAst && operation == EditOperation::EditRawCommandList) {
        return AuthorityError{AuthorityErrorCode::ReadOnlyDerivedView, "Event block is AST-authoritative; raw command list is derived"};
    }

    return std::nullopt;
}

} // namespace urpg
