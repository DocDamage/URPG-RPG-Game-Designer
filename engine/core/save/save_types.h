#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace urpg {

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
    std::string save_version;
    std::string timestamp;
    uint64_t playtime_seconds = 0;
    std::string thumbnail_hash;
    std::string map_display_name;
    std::vector<PartyMemberSnapshot> party_snapshot;
    SaveSlotFlags flags;
    std::string data_blob_path;
};

struct SaveEnvelope {
    std::string urpg_format_version = "1.0";
    std::string engine_version_min = "0.9.0";
    SaveSlotMeta meta;
};

} // namespace urpg
