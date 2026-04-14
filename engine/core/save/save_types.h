#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>

namespace urpg {

enum class SaveSlotCategory : uint8_t {
    Autosave = 0,
    Quicksave = 1,
    Manual = 2,
};

enum class SaveRetentionClass : uint8_t {
    Autosave = 0,
    Quicksave = 1,
    Manual = 2,
};

inline const char* ToString(SaveSlotCategory category) {
    switch (category) {
    case SaveSlotCategory::Autosave:
        return "autosave";
    case SaveSlotCategory::Quicksave:
        return "quicksave";
    case SaveSlotCategory::Manual:
        return "manual";
    }
    return "manual";
}

inline const char* ToString(SaveRetentionClass retention_class) {
    switch (retention_class) {
    case SaveRetentionClass::Autosave:
        return "autosave";
    case SaveRetentionClass::Quicksave:
        return "quicksave";
    case SaveRetentionClass::Manual:
        return "manual";
    }
    return "manual";
}

inline SaveRetentionClass RetentionClassForCategory(SaveSlotCategory category) {
    switch (category) {
    case SaveSlotCategory::Autosave:
        return SaveRetentionClass::Autosave;
    case SaveSlotCategory::Quicksave:
        return SaveRetentionClass::Quicksave;
    case SaveSlotCategory::Manual:
        return SaveRetentionClass::Manual;
    }
    return SaveRetentionClass::Manual;
}

struct PartyMemberSnapshot {
    std::string name;
    int32_t level = 1;
    int32_t hp = 0;
    int32_t max_hp = 0;
};

struct SaveSlotFlags {
    bool autosave = false;
    bool copilot_generated = false;
    bool corrupted = false;
};

struct SaveSlotMeta {
    int32_t slot_id = 0;
    SaveSlotCategory category = SaveSlotCategory::Manual;
    SaveRetentionClass retention_class = SaveRetentionClass::Manual;
    std::string save_version;
    std::string timestamp;
    uint64_t playtime_seconds = 0;
    std::string thumbnail_hash;
    std::string map_display_name;
    std::vector<PartyMemberSnapshot> party_snapshot;
    SaveSlotFlags flags;
    std::string data_blob_path;
    std::map<std::string, std::string> custom_metadata;
};

struct SaveEnvelope {
    std::string urpg_format_version = "1.0";
    std::string engine_version_min = "0.9.0";
    SaveSlotMeta meta;
};

} // namespace urpg
