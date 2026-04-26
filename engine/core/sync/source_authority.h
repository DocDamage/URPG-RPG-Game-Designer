#pragma once

#include <optional>
#include <string>

namespace urpg {

enum class ProjectMode {
    Native,
    Compat,
    Mixed
};

enum class AuthorityKind {
    RawCommandList,
    UrpgAst
};

enum class EditOperation {
    EditRawCommandList,
    EditUrpgAst
};

enum class AuthorityErrorCode {
    InvalidForMode,
    MissingAuthorityTag,
    ReadOnlyDerivedView
};

struct AuthorityTag {
    ProjectMode project_mode = ProjectMode::Native;
    std::optional<AuthorityKind> authoritative_kind;
};

struct AuthorityError {
    AuthorityErrorCode code;
    std::string message;
};

class SourceAuthorityPolicy {
public:
    static std::optional<AuthorityError> Validate(const AuthorityTag& tag, EditOperation operation);

private:
    static std::optional<AuthorityError> ValidateCompat(EditOperation operation);
    static std::optional<AuthorityError> ValidateNative(EditOperation operation);
    static std::optional<AuthorityError> ValidateMixed(const AuthorityTag& tag, EditOperation operation);
};

} // namespace urpg
