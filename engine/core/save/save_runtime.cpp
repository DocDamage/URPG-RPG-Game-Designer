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
    } catch (...) {
        // Ignore malformed metadata for runtime load metadata hydration.
    }
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

} // namespace urpg
