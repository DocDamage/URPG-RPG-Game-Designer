#include "engine/core/migrate/migration_runner.h"

#include <nlohmann/json.hpp>

namespace urpg {

namespace {

using json = nlohmann::json;

std::optional<MigrationError> ValidateSpec(const json& spec) {
    if (!spec.is_object() || !spec.contains("from") || !spec.contains("to") || !spec.contains("ops")) {
        return MigrationError{MigrationErrorCode::InvalidSpec, "migration spec must contain from/to/ops"};
    }

    if (!spec["from"].is_string() || !spec["to"].is_string() || !spec["ops"].is_array()) {
        return MigrationError{MigrationErrorCode::InvalidSpec, "from/to must be strings and ops must be an array"};
    }

    return std::nullopt;
}

std::optional<MigrationError> RemoveAtPointer(json& document, const json::json_pointer& ptr) {
    const json::json_pointer parent_ptr = ptr.parent_pointer();
    const std::string leaf = ptr.back();

    if (parent_ptr.empty()) {
        if (document.is_object()) {
            document.erase(leaf);
            return std::nullopt;
        }
        return MigrationError{MigrationErrorCode::InvalidPath, "cannot erase root or non-object root leaf"};
    }

    if (!document.contains(parent_ptr)) {
        return MigrationError{MigrationErrorCode::MissingPath, "rename parent path missing"};
    }

    json& parent = document[parent_ptr];
    if (parent.is_object()) {
        parent.erase(leaf);
        return std::nullopt;
    }

    if (parent.is_array()) {
        size_t index = 0;
        try {
            index = static_cast<size_t>(std::stoul(leaf));
        } catch (...) {
            return MigrationError{MigrationErrorCode::InvalidPath, "array index is not numeric"};
        }

        if (index >= parent.size()) {
            return MigrationError{MigrationErrorCode::MissingPath, "array index out of range"};
        }

        parent.erase(parent.begin() + static_cast<json::difference_type>(index));
        return std::nullopt;
    }

    return MigrationError{MigrationErrorCode::InvalidPath, "rename parent must be object or array"};
}

std::optional<MigrationError> ApplyRename(const json& op, json& document) {
    if (!op.contains("fromPath") || !op.contains("toPath") || !op["fromPath"].is_string() || !op["toPath"].is_string()) {
        return MigrationError{MigrationErrorCode::InvalidSpec, "rename requires string fromPath/toPath"};
    }

    const std::string from_path = op["fromPath"].get<std::string>();
    const std::string to_path = op["toPath"].get<std::string>();

    try {
        const json::json_pointer from_ptr(from_path);
        const json::json_pointer to_ptr(to_path);

        if (!document.contains(from_ptr)) {
            return MigrationError{MigrationErrorCode::MissingPath, "rename source path missing: " + from_path};
        }

        json moved = document.at(from_ptr);
        document[to_ptr] = std::move(moved);
        return RemoveAtPointer(document, from_ptr);
    } catch (const json::exception&) {
        return MigrationError{MigrationErrorCode::InvalidPath, "invalid json pointer in rename op"};
    }
}

std::optional<MigrationError> ApplySet(const json& op, json& document) {
    if (!op.contains("path") || !op["path"].is_string() || !op.contains("value")) {
        return MigrationError{MigrationErrorCode::InvalidSpec, "set requires path and value"};
    }

    const std::string path = op["path"].get<std::string>();

    try {
        const json::json_pointer ptr(path);
        document[ptr] = op["value"];
        return std::nullopt;
    } catch (const json::exception&) {
        return MigrationError{MigrationErrorCode::InvalidPath, "invalid json pointer in set op"};
    }
}

} // namespace

std::optional<MigrationError> MigrationRunner::Apply(const nlohmann::json& migration_spec, nlohmann::json& document) {
    if (auto spec_error = ValidateSpec(migration_spec)) {
        return spec_error;
    }

    const std::string from = migration_spec["from"].get<std::string>();
    const std::string to = migration_spec["to"].get<std::string>();

    if (!document.contains("_urpg_format_version") || !document["_urpg_format_version"].is_string()) {
        return MigrationError{MigrationErrorCode::VersionMismatch, "document missing _urpg_format_version"};
    }

    if (document["_urpg_format_version"].get<std::string>() != from) {
        return MigrationError{MigrationErrorCode::VersionMismatch, "document version does not match migration from"};
    }

    for (const auto& op : migration_spec["ops"]) {
        if (!op.contains("op") || !op["op"].is_string()) {
            return MigrationError{MigrationErrorCode::InvalidSpec, "operation missing op string"};
        }

        const std::string op_name = op["op"].get<std::string>();
        std::optional<MigrationError> error;

        if (op_name == "rename") {
            error = ApplyRename(op, document);
        } else if (op_name == "set") {
            error = ApplySet(op, document);
        } else {
            return MigrationError{MigrationErrorCode::UnknownOp, "unknown op: " + op_name};
        }

        if (error) {
            return error;
        }
    }

    document["_urpg_format_version"] = to;
    return std::nullopt;
}

} // namespace urpg
