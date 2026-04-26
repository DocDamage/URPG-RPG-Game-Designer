#include "engine/core/save/save_runtime.h"

#include <fstream>
#include <iterator>

#include <nlohmann/json.hpp>

namespace urpg {

namespace {

std::string ReadAll(const std::filesystem::path& path) {
    if (path.empty()) {
        return {};
    }

    std::ifstream in(path, std::ios::binary);
    if (!in) {
        return {};
    }

    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

void TryHydrateMeta(const std::string& metadata_payload, SaveSlotMeta& out_meta) {
    if (metadata_payload.empty()) {
        return;
    }

    try {
        const nlohmann::json root = nlohmann::json::parse(metadata_payload);
        if (root.contains("_slot_id") && root["_slot_id"].is_number_integer()) {
            out_meta.slot_id = root["_slot_id"].get<int32_t>();
        }
        if (root.contains("_slot_category") && root["_slot_category"].is_string()) {
            const auto category = root["_slot_category"].get<std::string>();
            if (category == "autosave") {
                out_meta.category = SaveSlotCategory::Autosave;
            } else if (category == "quicksave") {
                out_meta.category = SaveSlotCategory::Quicksave;
            } else {
                out_meta.category = SaveSlotCategory::Manual;
            }
        }
        if (root.contains("_retention_class") && root["_retention_class"].is_string()) {
            const auto retentionClass = root["_retention_class"].get<std::string>();
            if (retentionClass == "autosave") {
                out_meta.retention_class = SaveRetentionClass::Autosave;
            } else if (retentionClass == "quicksave") {
                out_meta.retention_class = SaveRetentionClass::Quicksave;
            } else {
                out_meta.retention_class = SaveRetentionClass::Manual;
            }
        } else {
            out_meta.retention_class = RetentionClassForCategory(out_meta.category);
        }
        if (root.contains("_save_version") && root["_save_version"].is_string()) {
            out_meta.save_version = root["_save_version"].get<std::string>();
        }
        if (root.contains("_timestamp") && root["_timestamp"].is_string()) {
            out_meta.timestamp = root["_timestamp"].get<std::string>();
        }
        if (root.contains("_playtime_seconds") && root["_playtime_seconds"].is_number_unsigned()) {
            out_meta.playtime_seconds = root["_playtime_seconds"].get<uint64_t>();
        }
        if (root.contains("_thumbnail_hash") && root["_thumbnail_hash"].is_string()) {
            out_meta.thumbnail_hash = root["_thumbnail_hash"].get<std::string>();
        }
        if (root.contains("_map_display_name") && root["_map_display_name"].is_string()) {
            out_meta.map_display_name = root["_map_display_name"].get<std::string>();
        }
        if (root.contains("_flags") && root["_flags"].is_object()) {
            const auto& flags = root["_flags"];
            if (flags.contains("autosave") && flags["autosave"].is_boolean()) {
                out_meta.flags.autosave = flags["autosave"].get<bool>();
                if (out_meta.flags.autosave) {
                    out_meta.category = SaveSlotCategory::Autosave;
                    out_meta.retention_class = SaveRetentionClass::Autosave;
                }
            }
            if (flags.contains("copilot_generated") && flags["copilot_generated"].is_boolean()) {
                out_meta.flags.copilot_generated = flags["copilot_generated"].get<bool>();
            }
            if (flags.contains("corrupted") && flags["corrupted"].is_boolean()) {
                out_meta.flags.corrupted = flags["corrupted"].get<bool>();
            }
        }
    } catch (...) {
        // Ignore malformed metadata for runtime load metadata hydration.
    }
}

void TryHydrateCreatedProtagonist(RuntimeSaveLoadResult& result) {
    if (result.payload.empty()) {
        return;
    }

    const auto parsed = nlohmann::json::parse(result.payload, nullptr, false);
    if (parsed.is_discarded()) {
        result.diagnostics.push_back("created_protagonist_payload_json_parse_failed");
        return;
    }

    std::vector<std::string> diagnostics;
    result.created_protagonist = character::loadCreatedProtagonistFromSaveDocument(parsed, &diagnostics);
    result.diagnostics.insert(result.diagnostics.end(), diagnostics.begin(), diagnostics.end());
}

} // namespace

RuntimeSaveLoadResult RuntimeSaveLoader::Load(const RuntimeSaveLoadRequest& request) {
    RuntimeSaveLoadResult result;

    if (!request.force_safe_mode) {
        result.payload = ReadAll(request.primary_save_path);
        if (!result.payload.empty()) {
            result.ok = true;
            result.recovery_tier = SaveRecoveryTier::None;
            TryHydrateMeta(ReadAll(request.metadata_path), result.active_meta);
            TryHydrateCreatedProtagonist(result);
            return result;
        }
    }

    SaveRecoveryRequest recovery_request;
    recovery_request.metadata_path = request.metadata_path;
    recovery_request.safe_mode_fallback_map = request.safe_mode_fallback_map;

    if (!request.force_safe_mode) {
        recovery_request.autosave_path = request.autosave_path;
        recovery_request.variables_path = request.variables_path;
    }

    const SaveRecoveryResult recovery_result = SaveRecoveryManager::Recover(recovery_request);
    if (!recovery_result.ok) {
        result.ok = false;
        result.error = recovery_result.error.empty() ? "save recovery failed" : recovery_result.error;
        return result;
    }

    result.ok = true;
    result.loaded_from_recovery = true;
    result.recovery_tier = recovery_result.tier;

    switch (recovery_result.tier) {
    case SaveRecoveryTier::Level1Autosave:
        result.payload = recovery_result.full_payload;
        TryHydrateCreatedProtagonist(result);
        break;
    case SaveRecoveryTier::Level2MetadataVariables:
        result.payload = recovery_result.metadata_payload;
        result.variables_payload = recovery_result.variables_payload;
        TryHydrateMeta(recovery_result.metadata_payload, result.active_meta);
        break;
    case SaveRecoveryTier::Level3SafeSkeleton:
        result.boot_safe_mode = true;
        result.active_meta = recovery_result.skeleton_meta;
        break;
    case SaveRecoveryTier::None:
        break;
    }

    if (request.force_safe_mode) {
        result.boot_safe_mode = true;
        if (result.active_meta.map_display_name.empty()) {
            result.active_meta.map_display_name = request.safe_mode_fallback_map;
        }
    }

    return result;
}

bool RuntimeSaveLoader::Save(const RuntimeSaveLoadRequest& request, const std::string& payload) {
    if (request.primary_save_path.empty()) {
        return false;
    }

    try {
        std::filesystem::create_directories(request.primary_save_path.parent_path());
        std::ofstream out(request.primary_save_path, std::ios::binary);
        if (!out) {
            return false;
        }
        out.write(payload.data(), payload.size());
        return true;
    } catch (...) {
        return false;
    }
}

bool RuntimeSaveLoader::SaveCreatedProtagonist(const RuntimeSaveLoadRequest& request, const std::string& payload,
                                               EntityID entity, const character::CharacterIdentity& identity,
                                               std::vector<std::string>* diagnostics) {
    auto document = nlohmann::json::parse(payload.empty() ? "{}" : payload, nullptr, false);
    if (document.is_discarded()) {
        if (diagnostics != nullptr) {
            diagnostics->push_back("save_payload_json_parse_failed");
        }
        return false;
    }
    if (!document.is_object()) {
        if (diagnostics != nullptr) {
            diagnostics->push_back("save_payload_not_object");
        }
        return false;
    }

    if (!character::attachCreatedProtagonistToSaveDocument(document, entity, identity, diagnostics)) {
        return false;
    }

    return Save(request, document.dump());
}

} // namespace urpg
