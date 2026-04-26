#include "engine/core/character/character_save_state.h"

#include "engine/core/character/character_identity_catalog.h"
#include "engine/core/character/character_identity_validator.h"

#include <exception>

namespace urpg::character {

namespace {

void appendDiagnostics(std::vector<std::string>* diagnostics, const std::vector<std::string>& values) {
    if (diagnostics == nullptr) {
        return;
    }
    diagnostics->insert(diagnostics->end(), values.begin(), values.end());
}

std::vector<std::string> validateIdentity(const CharacterIdentity& identity) {
    std::vector<std::string> diagnostics;
    CharacterIdentityValidator validator;
    const auto issues = validator.validate(identity, defaultCharacterIdentityCatalog());
    diagnostics.reserve(issues.size());
    for (const auto& issue : issues) {
        diagnostics.push_back(issue.field + ": " + issue.message);
    }
    return diagnostics;
}

} // namespace

nlohmann::json buildCreatedProtagonistSaveJson(EntityID entity, const CharacterIdentity& identity) {
    return {
        {"schemaVersion", "1.0.0"},
        {"entity", entity},
        {"identity", identity.toJson()},
    };
}

bool attachCreatedProtagonistToSaveDocument(nlohmann::json& document, EntityID entity,
                                            const CharacterIdentity& identity, std::vector<std::string>* diagnostics) {
    if (!document.is_object()) {
        if (diagnostics != nullptr) {
            diagnostics->push_back("save_document_not_object");
        }
        return false;
    }
    if (entity == 0) {
        if (diagnostics != nullptr) {
            diagnostics->push_back("created_protagonist_missing_entity");
        }
        return false;
    }

    const auto validationDiagnostics = validateIdentity(identity);
    if (!validationDiagnostics.empty()) {
        appendDiagnostics(diagnostics, validationDiagnostics);
        return false;
    }

    document[kCreatedProtagonistSaveKey] = buildCreatedProtagonistSaveJson(entity, identity);
    return true;
}

std::optional<CreatedProtagonistSaveState>
loadCreatedProtagonistFromSaveDocument(const nlohmann::json& document, std::vector<std::string>* diagnostics) {
    if (!document.is_object() || !document.contains(kCreatedProtagonistSaveKey)) {
        return std::nullopt;
    }

    const auto& root = document[kCreatedProtagonistSaveKey];
    if (!root.is_object()) {
        if (diagnostics != nullptr) {
            diagnostics->push_back("created_protagonist_not_object");
        }
        return std::nullopt;
    }
    if (root.value("schemaVersion", "") != "1.0.0") {
        if (diagnostics != nullptr) {
            diagnostics->push_back("created_protagonist_unsupported_schema");
        }
        return std::nullopt;
    }
    if (!root.contains("entity") || !root["entity"].is_number_unsigned()) {
        if (diagnostics != nullptr) {
            diagnostics->push_back("created_protagonist_missing_entity");
        }
        return std::nullopt;
    }
    if (!root.contains("identity") || !root["identity"].is_object()) {
        if (diagnostics != nullptr) {
            diagnostics->push_back("created_protagonist_missing_identity");
        }
        return std::nullopt;
    }

    CreatedProtagonistSaveState state;
    state.entity = root["entity"].get<EntityID>();
    try {
        state.identity = CharacterIdentity::fromJson(root["identity"]);
    } catch (const std::exception& ex) {
        if (diagnostics != nullptr) {
            diagnostics->push_back(std::string("created_protagonist_identity_parse_failed: ") + ex.what());
        }
        return std::nullopt;
    }

    state.diagnostics = validateIdentity(state.identity);
    state.valid = state.entity != 0 && state.diagnostics.empty();
    appendDiagnostics(diagnostics, state.diagnostics);
    return state;
}

} // namespace urpg::character
