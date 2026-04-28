#include "engine/core/save/save_load_preview_lab.h"

#include <fstream>

namespace urpg::save {
namespace {

std::string tierToString(SaveRecoveryTier tier) {
    switch (tier) {
    case SaveRecoveryTier::None:
        return "none";
    case SaveRecoveryTier::Level1Autosave:
        return "level1_autosave";
    case SaveRecoveryTier::Level2MetadataVariables:
        return "level2_metadata_variables";
    case SaveRecoveryTier::Level3SafeSkeleton:
        return "level3_safe_skeleton";
    }
    return "none";
}

bool writeText(const std::filesystem::path& path, const std::string& value) {
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);
    if (ec) {
        return false;
    }
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        return false;
    }
    out << value;
    return out.good();
}

bool jsonEquivalent(const std::string& actual, const nlohmann::json& expected) {
    const auto parsed = nlohmann::json::parse(actual, nullptr, false);
    return !parsed.is_discarded() && parsed == expected;
}

} // namespace

std::vector<SaveLoadPreviewDiagnostic> SaveLoadPreviewLabDocument::validate() const {
    std::vector<SaveLoadPreviewDiagnostic> diagnostics;
    if (id.empty()) {
        diagnostics.push_back({"missing_lab_id", "Save/load preview lab requires an id.", ""});
    }
    if (slot_id < 0) {
        diagnostics.push_back({"invalid_slot_id", "Save/load preview lab slot id cannot be negative.", id});
    }
    if (!primary_payload.is_object()) {
        diagnostics.push_back({"invalid_primary_payload", "Primary save payload must be a JSON object.", id});
    }
    if (!autosave_payload.is_object()) {
        diagnostics.push_back({"invalid_autosave_payload", "Autosave payload must be a JSON object.", id});
    }
    if (!metadata_payload.is_object()) {
        diagnostics.push_back({"invalid_metadata_payload", "Metadata payload must be a JSON object.", id});
    }
    if (!variables_payload.is_object()) {
        diagnostics.push_back({"invalid_variables_payload", "Variables payload must be a JSON object.", id});
    }
    if (safe_mode_fallback_map.empty()) {
        diagnostics.push_back({"missing_safe_mode_map", "Safe mode fallback map is required.", id});
    }
    return diagnostics;
}

nlohmann::json SaveLoadPreviewLabDocument::toJson() const {
    return {
        {"schema", "urpg.save_load_preview_lab.v1"},
        {"id", id},
        {"slot_id", slot_id},
        {"primary_payload", primary_payload},
        {"autosave_payload", autosave_payload},
        {"metadata_payload", metadata_payload},
        {"variables_payload", variables_payload},
        {"corrupt_primary", corrupt_primary},
        {"force_safe_mode", force_safe_mode},
        {"safe_mode_fallback_map", safe_mode_fallback_map},
    };
}

SaveLoadPreviewLabDocument SaveLoadPreviewLabDocument::fromJson(const nlohmann::json& json) {
    SaveLoadPreviewLabDocument document;
    if (!json.is_object()) {
        return document;
    }
    document.id = json.value("id", "");
    document.slot_id = json.value("slot_id", 1);
    document.primary_payload = json.value("primary_payload", nlohmann::json::object());
    document.autosave_payload = json.value("autosave_payload", nlohmann::json::object());
    document.metadata_payload = json.value("metadata_payload", nlohmann::json::object());
    document.variables_payload = json.value("variables_payload", nlohmann::json::object());
    document.corrupt_primary = json.value("corrupt_primary", false);
    document.force_safe_mode = json.value("force_safe_mode", false);
    document.safe_mode_fallback_map = json.value("safe_mode_fallback_map", std::string("Safe Mode - Origin"));
    return document;
}

SaveLoadPreviewLabResult RunSaveLoadPreviewLab(const SaveLoadPreviewLabDocument& document,
                                               const std::filesystem::path& workspace_root) {
    SaveLoadPreviewLabResult result;
    result.diagnostics = document.validate();
    if (!result.diagnostics.empty()) {
        return result;
    }

    const auto lab_root = workspace_root / document.id;
    const auto primary_path = lab_root / ("slot_" + std::to_string(document.slot_id) + ".json");
    const auto autosave_path = lab_root / "autosave.json";
    const auto metadata_path = lab_root / "metadata.json";
    const auto variables_path = lab_root / "variables.json";

    RuntimeSaveLoadRequest request;
    request.primary_save_path = primary_path;
    request.autosave_path = autosave_path;
    request.metadata_path = metadata_path;
    request.variables_path = variables_path;
    request.force_safe_mode = document.force_safe_mode;
    request.safe_mode_fallback_map = document.safe_mode_fallback_map;

    result.saved_primary = document.corrupt_primary ? writeText(primary_path, "{not-valid-json")
                                                    : RuntimeSaveLoader::Save(request, document.primary_payload.dump());
    const bool wrote_autosave = writeText(autosave_path, document.autosave_payload.dump());
    const bool wrote_metadata = writeText(metadata_path, document.metadata_payload.dump());
    const bool wrote_variables = writeText(variables_path, document.variables_payload.dump());
    if (!result.saved_primary) {
        result.diagnostics.push_back({"primary_save_failed", "Preview lab could not write the primary save.", document.id});
    }
    if (!wrote_autosave) {
        result.diagnostics.push_back({"autosave_write_failed", "Preview lab could not write autosave data.", document.id});
    }
    if (!wrote_metadata) {
        result.diagnostics.push_back({"metadata_write_failed", "Preview lab could not write metadata.", document.id});
    }
    if (!wrote_variables) {
        result.diagnostics.push_back({"variables_write_failed", "Preview lab could not write variables.", document.id});
    }
    if (!result.diagnostics.empty()) {
        return result;
    }

    const auto loaded = RuntimeSaveLoader::Load(request);
    result.loaded_ok = loaded.ok;
    result.loaded_from_recovery = loaded.loaded_from_recovery;
    result.boot_safe_mode = loaded.boot_safe_mode;
    result.recovery_tier = loaded.recovery_tier;
    result.loaded_payload = loaded.payload;
    result.loaded_variables_payload = loaded.variables_payload;
    result.loaded_meta = loaded.active_meta;
    for (const auto& diagnostic : loaded.diagnostics) {
        result.diagnostics.push_back({"runtime_" + diagnostic, "Runtime save loader reported " + diagnostic,
                                      document.id});
    }

    if (!loaded.ok) {
        result.diagnostics.push_back({"runtime_load_failed", loaded.error.empty() ? "Runtime load failed." : loaded.error,
                                      document.id});
        return result;
    }

    if (document.force_safe_mode) {
        result.payload_matches_expected = result.boot_safe_mode && result.loaded_payload.empty();
    } else if (document.corrupt_primary) {
        result.payload_matches_expected = jsonEquivalent(result.loaded_payload, document.autosave_payload);
    } else {
        result.payload_matches_expected = jsonEquivalent(result.loaded_payload, document.primary_payload);
    }

    if (!result.payload_matches_expected) {
        result.diagnostics.push_back({"payload_mismatch", "Loaded payload does not match the expected preview result.",
                                      document.id});
    }
    if (result.loaded_from_recovery && result.recovery_tier != SaveRecoveryTier::None) {
        result.diagnostics.push_back({"recovery_path_used", "Preview lab loaded through " + tierToString(result.recovery_tier),
                                      document.id});
    }
    return result;
}

} // namespace urpg::save
