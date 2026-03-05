#include "engine/core/events/event_edit_guard.h"

#include <nlohmann/json.hpp>

namespace urpg {

namespace {

std::string ToString(ProjectMode mode) {
    switch (mode) {
        case ProjectMode::Native:
            return "native";
        case ProjectMode::Compat:
            return "compat";
        case ProjectMode::Mixed:
            return "mixed";
        default:
            return "unknown";
    }
}

std::string ToString(EditOperation operation) {
    switch (operation) {
        case EditOperation::EditRawCommandList:
            return "edit_raw_command_list";
        case EditOperation::EditUrpgAst:
            return "edit_urpg_ast";
        default:
            return "unknown";
    }
}

std::string ToString(AuthorityErrorCode code) {
    switch (code) {
        case AuthorityErrorCode::InvalidForMode:
            return "invalid_for_mode";
        case AuthorityErrorCode::MissingAuthorityTag:
            return "missing_authority_tag";
        case AuthorityErrorCode::ReadOnlyDerivedView:
            return "read_only_derived_view";
        default:
            return "unknown";
    }
}

std::string BuildDiagnosticJsonl(const EventEditRequest& request, const AuthorityError& error) {
    nlohmann::json diagnostic;
    diagnostic["ts"] = request.timestamp_utc.empty() ? "1970-01-01T00:00:00Z" : request.timestamp_utc;
    diagnostic["level"] = "warn";
    diagnostic["subsystem"] = "event_authority";
    diagnostic["event"] = "edit_rejected";
    diagnostic["event_id"] = request.event_id;
    if (!request.block_id.empty()) {
        diagnostic["block_id"] = request.block_id;
    }
    diagnostic["mode"] = ToString(request.authority.project_mode);
    diagnostic["operation"] = ToString(request.operation);
    diagnostic["error_code"] = ToString(error.code);
    diagnostic["message"] = error.message;
    return diagnostic.dump();
}

} // namespace

EventEditResult EventEditGuard::ValidateAndDiagnose(const EventEditRequest& request) {
    EventEditResult result;

    if (const auto error = SourceAuthorityPolicy::Validate(request.authority, request.operation)) {
        result.allowed = false;
        result.diagnostic_jsonl = BuildDiagnosticJsonl(request, error.value());
        return result;
    }

    result.allowed = true;
    return result;
}

} // namespace urpg
