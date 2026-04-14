#include "engine/core/save/save_recovery.h"

#include <fstream>
#include <iterator>

#include <nlohmann/json.hpp>

namespace urpg {

namespace {

std::string ReadAll(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        return {};
    }
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

void TryHydrateSkeletonFromMetadata(const std::string& metadata_payload, SaveSlotMeta& out_meta) {
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
        if (root.contains("_party_snapshot") && root["_party_snapshot"].is_array()) {
            out_meta.party_snapshot.clear();
            for (const auto& member : root["_party_snapshot"]) {
                if (!member.is_object()) {
                    continue;
                }

                PartyMemberSnapshot snapshot;
                if (member.contains("name") && member["name"].is_string()) {
                    snapshot.name = member["name"].get<std::string>();
                }
                if (member.contains("level") && member["level"].is_number_integer()) {
                    snapshot.level = member["level"].get<int32_t>();
                }
                if (member.contains("hp") && member["hp"].is_number_integer()) {
                    snapshot.hp = member["hp"].get<int32_t>();
                }
                if (member.contains("max_hp") && member["max_hp"].is_number_integer()) {
                    snapshot.max_hp = member["max_hp"].get<int32_t>();
                }
                out_meta.party_snapshot.push_back(std::move(snapshot));
            }
        }
    } catch (...) {
        // Metadata can be partially corrupted. Level 3 still continues.
    }
}

} // namespace

SaveRecoveryResult SaveRecoveryManager::Recover(const SaveRecoveryRequest& request) {
    SaveRecoveryResult result;

    // Level 1: last autosave checkpoint.
    if (!request.autosave_path.empty()) {
        result.full_payload = ReadAll(request.autosave_path);
        if (!result.full_payload.empty()) {
            result.ok = true;
            result.tier = SaveRecoveryTier::Level1Autosave;
            return result;
        }
    }

    // Level 2: metadata + variables only.
    if (!request.metadata_path.empty() && !request.variables_path.empty()) {
        result.metadata_payload = ReadAll(request.metadata_path);
        result.variables_payload = ReadAll(request.variables_path);
        if (!result.metadata_payload.empty() && !result.variables_payload.empty()) {
            result.ok = true;
            result.tier = SaveRecoveryTier::Level2MetadataVariables;
            return result;
        }
    }

    // Level 3: safe mode skeleton load.
    if (!request.metadata_path.empty()) {
        result.metadata_payload = ReadAll(request.metadata_path);
    }

    result.skeleton_meta.map_display_name = request.safe_mode_fallback_map;
    TryHydrateSkeletonFromMetadata(result.metadata_payload, result.skeleton_meta);
    if (result.skeleton_meta.map_display_name.empty()) {
        result.skeleton_meta.map_display_name = request.safe_mode_fallback_map;
    }

    result.variables_reset = true;
    result.ok = true;
    result.tier = SaveRecoveryTier::Level3SafeSkeleton;
    return result;
}

} // namespace urpg
