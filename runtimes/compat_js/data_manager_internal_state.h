#pragma once

#include "runtimes/compat_js/data_manager.h"

#include <chrono>
#include <cstdint>
#include <unordered_map>

namespace urpg {
namespace compat {
struct SaveSlotData {
    SaveHeader header;
    GlobalState state;
};

struct PendingTransfer {
    int32_t mapId = 0;
    int32_t x = 0;
    int32_t y = 0;
    int32_t direction = -1;
};

class DataManagerImpl {
public:
    std::chrono::steady_clock::time_point playtimeStart;
    int32_t loadedMapId = 0;
    bool isDatabaseLoaded = false;
    bool transferPending = false;
    PendingTransfer pendingTransfer;
    std::unordered_map<int32_t, SaveSlotData> saveSlots;
    std::unordered_map<int32_t, std::unordered_map<std::string, Value>> saveHeaderExtensions;
};


} // namespace compat
} // namespace urpg
