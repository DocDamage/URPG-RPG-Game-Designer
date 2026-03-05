#pragma once

#include <nlohmann/json_fwd.hpp>

#include <optional>
#include <string>

namespace urpg {

enum class MigrationErrorCode {
    InvalidSpec,
    VersionMismatch,
    MissingPath,
    InvalidPath,
    UnknownOp
};

struct MigrationError {
    MigrationErrorCode code;
    std::string message;
};

class MigrationRunner {
public:
    static std::optional<MigrationError> Apply(const nlohmann::json& migration_spec, nlohmann::json& document);
};

} // namespace urpg
