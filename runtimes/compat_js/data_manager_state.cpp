#include "runtimes/compat_js/data_manager_internal_state.h"

#include <algorithm>
#include <chrono>
#include <optional>
#include <sstream>
#include <utility>

namespace urpg {
namespace compat {
void DataManager::setupNewGame() {
    globalState_ = GlobalState{};
    globalState_.mapId = startMapId_;
    globalState_.playerX = startX_;
    globalState_.playerY = startY_;
    globalState_.playerDirection = 2;
    globalState_.partyMembers = startParty_;
    globalState_.gold = 0;
    globalState_.steps = 0;
    globalState_.playtime = 0;

    impl_->playtimeStart = std::chrono::steady_clock::now();
    impl_->transferPending = false;
}

GlobalState& DataManager::getGlobalState() {
    return globalState_;
}

const GlobalState& DataManager::getGlobalState() const {
    return globalState_;
}

int32_t DataManager::getPartySize() const {
    return static_cast<int32_t>(globalState_.partyMembers.size());
}

int32_t DataManager::getPartyMember(int32_t index) const {
    if (index < 0 || static_cast<size_t>(index) >= globalState_.partyMembers.size()) {
        return 0;
    }
    return globalState_.partyMembers[static_cast<size_t>(index)];
}

int32_t DataManager::getGold() const {
    return globalState_.gold;
}

void DataManager::setGold(int32_t gold) {
    globalState_.gold = std::max(0, gold);
}

void DataManager::gainGold(int32_t amount) {
    globalState_.gold = std::max(0, globalState_.gold + amount);
}

void DataManager::loseGold(int32_t amount) {
    globalState_.gold = std::max(0, globalState_.gold - amount);
}

int32_t DataManager::getItemCount(int32_t itemId) const {
    auto it = globalState_.items.find(itemId);
    return it != globalState_.items.end() ? it->second : 0;
}

void DataManager::setItemCount(int32_t itemId, int32_t count) {
    if (count <= 0) {
        globalState_.items.erase(itemId);
    } else {
        globalState_.items[itemId] = count;
    }
}

void DataManager::gainItem(int32_t itemId, int32_t count) {
    setItemCount(itemId, getItemCount(itemId) + count);
}

void DataManager::loseItem(int32_t itemId, int32_t count) {
    setItemCount(itemId, getItemCount(itemId) - count);
}

bool DataManager::hasItem(int32_t itemId) const {
    return getItemCount(itemId) > 0;
}

int32_t DataManager::getWeaponCount(int32_t weaponId) const {
    auto it = globalState_.weapons.find(weaponId);
    return it != globalState_.weapons.end() ? it->second : 0;
}

int32_t DataManager::getArmorCount(int32_t armorId) const {
    auto it = globalState_.armors.find(armorId);
    return it != globalState_.armors.end() ? it->second : 0;
}

void DataManager::gainWeapon(int32_t weaponId, int32_t count) {
    int32_t newCount = getWeaponCount(weaponId) + count;
    if (newCount <= 0) {
        globalState_.weapons.erase(weaponId);
    } else {
        globalState_.weapons[weaponId] = newCount;
    }
}

void DataManager::loseWeapon(int32_t weaponId, int32_t count) {
    gainWeapon(weaponId, -count);
}

void DataManager::gainArmor(int32_t armorId, int32_t count) {
    int32_t newCount = getArmorCount(armorId) + count;
    if (newCount <= 0) {
        globalState_.armors.erase(armorId);
    } else {
        globalState_.armors[armorId] = newCount;
    }
}

void DataManager::loseArmor(int32_t armorId, int32_t count) {
    gainArmor(armorId, -count);
}

bool DataManager::getSwitch(int32_t switchId) const {
    if (switchId <= 0 || static_cast<size_t>(switchId) > globalState_.switches.size()) {
        return false;
    }
    return globalState_.switches[static_cast<size_t>(switchId - 1)];
}

void DataManager::setSwitch(int32_t switchId, bool value) {
    if (switchId <= 0) return;

    // Resize if needed
    if (static_cast<size_t>(switchId) > globalState_.switches.size()) {
        globalState_.switches.resize(static_cast<size_t>(switchId), false);
    }
    globalState_.switches[static_cast<size_t>(switchId - 1)] = value;
}

int32_t DataManager::getVariable(int32_t varId) const {
    if (varId <= 0 || static_cast<size_t>(varId) > globalState_.variables.size()) {
        return 0;
    }
    return globalState_.variables[static_cast<size_t>(varId - 1)];
}

void DataManager::setVariable(int32_t varId, int32_t value) {
    if (varId <= 0) return;

    // Resize if needed
    if (static_cast<size_t>(varId) > globalState_.variables.size()) {
        globalState_.variables.resize(static_cast<size_t>(varId), 0);
    }
    globalState_.variables[static_cast<size_t>(varId - 1)] = value;
}

bool DataManager::getSelfSwitch(int32_t mapId, int32_t eventId, const std::string& key) const {
    std::string fullKey = std::to_string(mapId) + "_" + std::to_string(eventId) + "_" + key;
    auto it = globalState_.selfSwitches.find(fullKey);
    return it != globalState_.selfSwitches.end() ? it->second : false;
}

void DataManager::setSelfSwitch(int32_t mapId, int32_t eventId, const std::string& key, bool value) {
    std::string fullKey = std::to_string(mapId) + "_" + std::to_string(eventId) + "_" + key;
    globalState_.selfSwitches[fullKey] = value;
}

int32_t DataManager::getPlaytime() const {
    return globalState_.playtime;
}

void DataManager::updatePlaytime() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - impl_->playtimeStart);
    globalState_.playtime = static_cast<int32_t>(elapsed.count());
}

std::string DataManager::getPlaytimeString() const {
    int32_t totalSeconds = globalState_.playtime;
    int32_t hours = totalSeconds / 3600;
    int32_t minutes = (totalSeconds % 3600) / 60;
    int32_t seconds = totalSeconds % 60;

    std::ostringstream oss;
    oss << hours << ":";
    if (minutes < 10) oss << "0";
    oss << minutes << ":";
    if (seconds < 10) oss << "0";
    oss << seconds;
    return oss.str();
}

int32_t DataManager::getSteps() const {
    return globalState_.steps;
}

void DataManager::incrementSteps() {
    globalState_.steps++;
}

void DataManager::updateActorHp(int32_t actorId, int32_t hp) {
    auto* actor = getActor(actorId);
    if (!actor) return;
    const int32_t maxHp = std::max(1, getActorParam(actorId, 0, actor->level));
    actor->hp = std::clamp(hp, 0, maxHp);
}

void DataManager::updateActorMp(int32_t actorId, int32_t mp) {
    auto* actor = getActor(actorId);
    if (!actor) return;
    const int32_t maxMp = std::max(0, getActorParam(actorId, 1, actor->level));
    actor->mp = std::clamp(mp, 0, maxMp);
}

void DataManager::gainExp(int32_t actorId, int32_t exp) {
    auto* actor = getActor(actorId);
    if (!actor) return;

    actor->exp += exp;

    const auto* cls = getClass(actor->classId);
    if (!cls) return;

    const int32_t maxLv = std::min(actor->maxLevel, cls->maxLevel);
    while (actor->level < maxLv) {
        const int32_t nextLevel = actor->level + 1;
        const int32_t required = (nextLevel - 2 >= 0 && nextLevel - 2 < static_cast<int32_t>(cls->expTable.size()))
                                     ? cls->expTable[nextLevel - 2]
                                     : nextLevel * 10; // fallback curve
        if (actor->exp < required) {
            break;
        }
        actor->exp -= required;
        actor->level = nextLevel;
        // Learn skills for this level
        for (const auto& pair : cls->skillsToLearn) {
            if (pair.first == actor->level) {
                if (std::find(actor->skills.begin(), actor->skills.end(), pair.second) == actor->skills.end()) {
                    actor->skills.push_back(pair.second);
                }
            }
        }
    }
}

int32_t DataManager::getPlayerMapId() const {
    return globalState_.mapId;
}

int32_t DataManager::getPlayerX() const {
    return globalState_.playerX;
}

int32_t DataManager::getPlayerY() const {
    return globalState_.playerY;
}

int32_t DataManager::getPlayerDirection() const {
    return globalState_.playerDirection;
}

void DataManager::setPlayerPosition(int32_t mapId, int32_t x, int32_t y) {
    globalState_.mapId = mapId;
    globalState_.playerX = x;
    globalState_.playerY = y;
}

void DataManager::setPlayerDirection(int32_t direction) {
    globalState_.playerDirection = direction;
}

// ============================================================================
// Map Transfer
// ============================================================================

void DataManager::reserveTransfer(int32_t mapId, int32_t x, int32_t y, int32_t direction) {
    impl_->pendingTransfer.mapId = mapId;
    impl_->pendingTransfer.x = x;
    impl_->pendingTransfer.y = y;
    impl_->pendingTransfer.direction = direction;
    impl_->transferPending = true;
}

bool DataManager::isTransferring() const {
    return impl_->transferPending;
}

void DataManager::processTransfer() {
    if (!impl_->transferPending) {
        return;
    }

    globalState_.mapId = impl_->pendingTransfer.mapId;
    globalState_.playerX = impl_->pendingTransfer.x;
    globalState_.playerY = impl_->pendingTransfer.y;
    if (impl_->pendingTransfer.direction >= 0) {
        globalState_.playerDirection = impl_->pendingTransfer.direction;
    }
    impl_->transferPending = false;
}

// ============================================================================
// Save/Load Operations
// ============================================================================

} // namespace compat
} // namespace urpg
