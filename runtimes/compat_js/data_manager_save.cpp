#include "runtimes/compat_js/data_manager_internal_state.h"

#include <chrono>
#include <optional>
#include <utility>

namespace urpg {
namespace compat {
namespace {
constexpr int32_t kAutosaveSlot = 0;

bool isValidSaveSlot(int32_t slot, int32_t maxSaveFiles) {
    return slot >= kAutosaveSlot && slot <= maxSaveFiles;
}
} // namespace
bool DataManager::saveGame(int32_t slot) {
    if (!isValidSaveSlot(slot, getMaxSaveFiles())) {
        return false;
    }

    SaveSlotData slotData;
    slotData.state = globalState_;
    slotData.header = createSaveHeader(slot == kAutosaveSlot);
    impl_->saveSlots[slot] = std::move(slotData);
    return true;
}

bool DataManager::saveGameWithHeader(int32_t slot, const SaveHeader& header) {
    if (!isValidSaveSlot(slot, getMaxSaveFiles())) {
        return false;
    }

    SaveSlotData slotData;
    slotData.state = globalState_;
    slotData.header = header;
    if (slot == kAutosaveSlot) {
        slotData.header.isAutosave = true;
    }
    impl_->saveSlots[slot] = std::move(slotData);
    return true;
}

bool DataManager::loadGame(int32_t slot) {
    if (!isValidSaveSlot(slot, getMaxSaveFiles())) {
        return false;
    }

    auto it = impl_->saveSlots.find(slot);
    if (it == impl_->saveSlots.end()) {
        return false;
    }

    globalState_ = it->second.state;
    impl_->transferPending = false;
    impl_->playtimeStart = std::chrono::steady_clock::now() - std::chrono::seconds(globalState_.playtime);
    return true;
}

bool DataManager::deleteSaveFile(int32_t slot) {
    if (!isValidSaveSlot(slot, getMaxSaveFiles())) {
        return false;
    }

    const bool removedSave = (impl_->saveSlots.erase(slot) > 0);
    const bool removedExt = (impl_->saveHeaderExtensions.erase(slot) > 0);
    return removedSave || removedExt;
}

bool DataManager::doesSaveFileExist(int32_t slot) const {
    if (!isValidSaveSlot(slot, getMaxSaveFiles())) {
        return false;
    }
    return impl_->saveSlots.find(slot) != impl_->saveSlots.end();
}

int32_t DataManager::getMaxSaveFiles() const {
    return 20; // MZ default
}

std::optional<SaveHeader> DataManager::getSaveHeader(int32_t slot) const {
    if (!isValidSaveSlot(slot, getMaxSaveFiles())) {
        return std::nullopt;
    }

    auto it = impl_->saveSlots.find(slot);
    if (it == impl_->saveSlots.end()) {
        return std::nullopt;
    }
    return it->second.header;
}

std::vector<SaveHeader> DataManager::getAllSaveHeaders() const {
    std::vector<SaveHeader> headers;
    for (int32_t i = 1; i <= getMaxSaveFiles(); ++i) {
        auto header = getSaveHeader(i);
        if (header) {
            headers.push_back(*header);
        }
    }
    return headers;
}

bool DataManager::isAutosaveEnabled() const {
    return autosaveEnabled_;
}

void DataManager::setAutosaveEnabled(bool enabled) {
    autosaveEnabled_ = enabled;
}

bool DataManager::saveAutosave() {
    if (!autosaveEnabled_) {
        return false;
    }
    return saveGame(0);
}

bool DataManager::loadAutosave() {
    return loadGame(0);
}

bool DataManager::setSaveHeaderExtension(int32_t slot, const std::string& key, const Value& value) {
    if (!isValidSaveSlot(slot, getMaxSaveFiles()) || key.empty()) {
        return false;
    }

    impl_->saveHeaderExtensions[slot][key] = value;
    return true;
}

std::optional<Value> DataManager::getSaveHeaderExtension(int32_t slot, const std::string& key) const {
    if (!isValidSaveSlot(slot, getMaxSaveFiles()) || key.empty()) {
        return std::nullopt;
    }

    auto slotIt = impl_->saveHeaderExtensions.find(slot);
    if (slotIt == impl_->saveHeaderExtensions.end()) {
        return std::nullopt;
    }

    auto keyIt = slotIt->second.find(key);
    if (keyIt == slotIt->second.end()) {
        return std::nullopt;
    }
    return keyIt->second;
}

SaveHeader DataManager::createSaveHeader(bool isAutosave) const {
    SaveHeader header;
    header.version = 1;
    header.playtimeFrames = globalState_.playtime * 60;
    header.mapId = globalState_.mapId;
    header.playerId = 0;
    header.playerX = globalState_.playerX;
    header.playerY = globalState_.playerY;
    header.isAutosave = isAutosave;
    return header;
}
} // namespace compat
} // namespace urpg
