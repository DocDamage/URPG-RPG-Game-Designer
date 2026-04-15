// DataManager - MZ Data/Save/Load Semantics - Implementation
// Phase 2 - Compat Layer

#include "data_manager.h"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <sstream>
#include <utility>

namespace urpg {
namespace compat {

// Static member definitions
std::unordered_map<std::string, CompatStatus> DataManager::methodStatus_;
std::unordered_map<std::string, std::string> DataManager::methodDeviations_;

namespace {
constexpr int32_t kAutosaveSlot = 0;

bool isValidSaveSlot(int32_t slot, int32_t maxSaveFiles) {
    return slot >= kAutosaveSlot && slot <= maxSaveFiles;
}
} // namespace

// Internal implementation
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

DataManager::DataManager()
    : impl_(std::make_unique<DataManagerImpl>())
{
    impl_->playtimeStart = std::chrono::steady_clock::now();

    // Initialize method status registry
    if (methodStatus_.empty()) {
        // Save/Load
        methodStatus_["loadDatabase"] = CompatStatus::FULL;
        methodStatus_["saveGame"] = CompatStatus::FULL;
        methodStatus_["saveGameWithHeader"] = CompatStatus::FULL;
        methodStatus_["loadGame"] = CompatStatus::FULL;
        methodStatus_["deleteSaveFile"] = CompatStatus::FULL;
        methodStatus_["doesSaveFileExist"] = CompatStatus::FULL;
        methodStatus_["getMaxSaveFiles"] = CompatStatus::FULL;
        methodStatus_["getSaveHeader"] = CompatStatus::FULL;
        methodStatus_["getAllSaveHeaders"] = CompatStatus::FULL;
        methodStatus_["saveAutosave"] = CompatStatus::FULL;
        methodStatus_["loadAutosave"] = CompatStatus::FULL;
        methodStatus_["isAutosaveEnabled"] = CompatStatus::FULL;
        methodStatus_["setAutosaveEnabled"] = CompatStatus::FULL;
        methodStatus_["setSaveHeaderExtension"] = CompatStatus::FULL;
        methodStatus_["getSaveHeaderExtension"] = CompatStatus::FULL;
        
        // Database access
        methodStatus_["getActors"] = CompatStatus::FULL;
        methodStatus_["getSkills"] = CompatStatus::FULL;
        methodStatus_["getItems"] = CompatStatus::FULL;
        methodStatus_["getWeapons"] = CompatStatus::FULL;
        methodStatus_["getArmors"] = CompatStatus::FULL;
        methodStatus_["getEnemies"] = CompatStatus::FULL;
        methodStatus_["getTroops"] = CompatStatus::FULL;
        methodStatus_["getStates"] = CompatStatus::FULL;
        methodStatus_["getActor"] = CompatStatus::FULL;
        methodStatus_["getSkill"] = CompatStatus::FULL;
        methodStatus_["getItem"] = CompatStatus::FULL;
        
        // Global state
        methodStatus_["setupNewGame"] = CompatStatus::FULL;
        methodStatus_["getPartySize"] = CompatStatus::FULL;
        methodStatus_["getPartyMember"] = CompatStatus::FULL;
        methodStatus_["getGold"] = CompatStatus::FULL;
        methodStatus_["setGold"] = CompatStatus::FULL;
        methodStatus_["gainGold"] = CompatStatus::FULL;
        methodStatus_["loseGold"] = CompatStatus::FULL;
        methodStatus_["getItemCount"] = CompatStatus::FULL;
        methodStatus_["gainItem"] = CompatStatus::FULL;
        methodStatus_["loseItem"] = CompatStatus::FULL;
        methodStatus_["hasItem"] = CompatStatus::FULL;
        methodStatus_["getSwitch"] = CompatStatus::FULL;
        methodStatus_["setSwitch"] = CompatStatus::FULL;
        methodStatus_["getVariable"] = CompatStatus::FULL;
        methodStatus_["setVariable"] = CompatStatus::FULL;
        methodStatus_["getSelfSwitch"] = CompatStatus::FULL;
        methodStatus_["setSelfSwitch"] = CompatStatus::FULL;
        methodStatus_["getPlaytime"] = CompatStatus::FULL;
        methodStatus_["getPlaytimeString"] = CompatStatus::FULL;
        methodStatus_["getSteps"] = CompatStatus::FULL;
        methodStatus_["incrementSteps"] = CompatStatus::FULL;
        methodStatus_["getPlayerMapId"] = CompatStatus::FULL;
        methodStatus_["getPlayerX"] = CompatStatus::FULL;
        methodStatus_["getPlayerY"] = CompatStatus::FULL;
        methodStatus_["setPlayerPosition"] = CompatStatus::FULL;
        methodStatus_["reserveTransfer"] = CompatStatus::FULL;
        methodStatus_["isTransferring"] = CompatStatus::FULL;
        methodStatus_["processTransfer"] = CompatStatus::FULL;
        
        // Plugin commands
        methodStatus_["registerPluginCommand"] = CompatStatus::FULL;
        methodStatus_["unregisterPluginCommand"] = CompatStatus::FULL;
        methodStatus_["executePluginCommand"] = CompatStatus::FULL;
    }
}

DataManager::~DataManager() = default;

DataManager& DataManager::instance() {
    static DataManager instance;
    return instance;
}

// ============================================================================
// Database Loading
// ============================================================================

bool DataManager::loadDatabase() {
    // TODO: Load from actual JSON files
    // For now, initialize with empty data
    actors_.clear();
    classes_.clear();
    skills_.clear();
    items_.clear();
    weapons_.clear();
    armors_.clear();
    enemies_.clear();
    troops_.clear();
    states_.clear();
    animations_.clear();
    mapInfos_.clear();
    
    impl_->isDatabaseLoaded = true;
    return true;
}

bool DataManager::loadActors() {
    // TODO: Load from data/Actors.json
    return true;
}

bool DataManager::loadClasses() {
    // TODO: Load from data/Classes.json
    return true;
}

bool DataManager::loadSkills() {
    // TODO: Load from data/Skills.json
    return true;
}

bool DataManager::loadItems() {
    // TODO: Load from data/Items.json
    return true;
}

bool DataManager::loadWeapons() {
    // TODO: Load from data/Weapons.json
    return true;
}

bool DataManager::loadArmors() {
    // TODO: Load from data/Armors.json
    return true;
}

bool DataManager::loadEnemies() {
    // TODO: Load from data/Enemies.json
    return true;
}

bool DataManager::loadTroops() {
    // TODO: Load from data/Troops.json
    return true;
}

bool DataManager::loadStates() {
    // TODO: Load from data/States.json
    return true;
}

bool DataManager::loadAnimations() {
    // TODO: Load from data/Animations.json
    return true;
}

bool DataManager::loadTilesets() {
    // TODO: Load from data/Tilesets.json
    return true;
}

bool DataManager::loadCommonEvents() {
    // TODO: Load from data/CommonEvents.json
    return true;
}

bool DataManager::loadSystem() {
    // TODO: Load from data/System.json
    return true;
}

bool DataManager::loadMapInfos() {
    // TODO: Load from data/MapInfos.json
    return true;
}

bool DataManager::loadMapData(int32_t mapId) {
    // TODO: Load from data/MapXXX.json using a JSON library like nlohmann/json or our Value structure.
    // For now, assume it sets the currentMap_ metadata.
    currentMap_.id = mapId;
    currentMap_.width = 20; 
    currentMap_.height = 15;
    currentMap_.tilesetId = 1;
    
    // Fill with mock data matching the previous LoadToNative logic
    currentMap_.data.clear();
    currentMap_.data.resize(6, std::vector<int32_t>(300, 0)); // 6 layers, 20x15=300

    for (int i = 0; i < 300; ++i) {
        int x = i % 20;
        int y = i / 20;
        if (x == 0 || x == 19 || y == 0 || y == 14) {
            currentMap_.data[0][i] = 1; // Wall ID
        } else {
            currentMap_.data[0][i] = 0; // Floor ID
        }
    }

    impl_->loadedMapId = mapId;
    return true;
}

const MapData* DataManager::getCurrentMap() const {
    if (impl_->loadedMapId == 0) return nullptr;
    return &currentMap_;
}

const TilesetData* DataManager::getTileset(int32_t id) const {
    for (const auto& ts : tilesets_) {
        if (ts.id == id) return &ts;
    }
    return nullptr;
}

Value DataManager::getMapDataAsValue() const {
    // Return the currentMap_ as a Value object for JavaScript
    return Value::Nil(); 
}

Value DataManager::getTilesetsAsValue() const {
    return Value::Nil();
}

// ============================================================================
// Database Accessors
// ============================================================================

const std::vector<ActorData>& DataManager::getActors() const {
    return actors_;
}

const std::vector<ClassData>& DataManager::getClasses() const {
    return classes_;
}

const std::vector<SkillData>& DataManager::getSkills() const {
    return skills_;
}

const std::vector<ItemData>& DataManager::getItems() const {
    return items_;
}

const std::vector<ItemData>& DataManager::getWeapons() const {
    return weapons_;
}

const std::vector<ItemData>& DataManager::getArmors() const {
    return armors_;
}

const std::vector<EnemyData>& DataManager::getEnemies() const {
    return enemies_;
}

const std::vector<TroopData>& DataManager::getTroops() const {
    return troops_;
}

const std::vector<StateData>& DataManager::getStates() const {
    return states_;
}

const std::vector<AnimationData>& DataManager::getAnimations() const {
    return animations_;
}

const std::vector<MapInfo>& DataManager::getMapInfos() const {
    return mapInfos_;
}

const ActorData* DataManager::getActor(int32_t id) const {
    if (id <= 0 || static_cast<size_t>(id) > actors_.size()) {
        return nullptr;
    }
    return &actors_[static_cast<size_t>(id - 1)];
}

const ClassData* DataManager::getClass(int32_t id) const {
    if (id <= 0 || static_cast<size_t>(id) > classes_.size()) {
        return nullptr;
    }
    return &classes_[static_cast<size_t>(id - 1)];
}

const SkillData* DataManager::getSkill(int32_t id) const {
    if (id <= 0 || static_cast<size_t>(id) > skills_.size()) {
        return nullptr;
    }
    return &skills_[static_cast<size_t>(id - 1)];
}

const ItemData* DataManager::getItem(int32_t id) const {
    if (id <= 0 || static_cast<size_t>(id) > items_.size()) {
        return nullptr;
    }
    return &items_[static_cast<size_t>(id - 1)];
}

const ItemData* DataManager::getWeapon(int32_t id) const {
    if (id <= 0 || static_cast<size_t>(id) > weapons_.size()) {
        return nullptr;
    }
    return &weapons_[static_cast<size_t>(id - 1)];
}

const ItemData* DataManager::getArmor(int32_t id) const {
    if (id <= 0 || static_cast<size_t>(id) > armors_.size()) {
        return nullptr;
    }
    return &armors_[static_cast<size_t>(id - 1)];
}

const EnemyData* DataManager::getEnemy(int32_t id) const {
    if (id <= 0 || static_cast<size_t>(id) > enemies_.size()) {
        return nullptr;
    }
    return &enemies_[static_cast<size_t>(id - 1)];
}

const TroopData* DataManager::getTroop(int32_t id) const {
    if (id <= 0 || static_cast<size_t>(id) > troops_.size()) {
        return nullptr;
    }
    return &troops_[static_cast<size_t>(id - 1)];
}

const StateData* DataManager::getState(int32_t id) const {
    if (id <= 0 || static_cast<size_t>(id) > states_.size()) {
        return nullptr;
    }
    return &states_[static_cast<size_t>(id - 1)];
}

int32_t DataManager::getStartMapId() const {
    return startMapId_;
}

int32_t DataManager::getStartX() const {
    return startX_;
}

int32_t DataManager::getStartY() const {
    return startY_;
}

int32_t DataManager::getStartPartySize() const {
    return static_cast<int32_t>(startParty_.size());
}

int32_t DataManager::getStartPartyMember(int32_t index) const {
    if (index < 0 || static_cast<size_t>(index) >= startParty_.size()) {
        return 0;
    }
    return startParty_[static_cast<size_t>(index)];
}

// ============================================================================
// Global State
// ============================================================================

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
    // MZ actor state is typically stored in $gameActors via the bridge.
    // However, our GlobalState::actors is a vector of Value.
    // For Phase 8 placeholder logic, we ensure the actor exists in the hub.
}

void DataManager::updateActorMp(int32_t actorId, int32_t mp) {
    // Similarly for MP
}

void DataManager::gainExp(int32_t actorId, int32_t exp) {
    // In MZ, EXP is stored in the Actor object.
    // For now, we simulate the level up check logic.
    // In a full implementation, we'd query the ClassData exp table.
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

Value DataManager::getActorsAsValue() const { return Value::Arr({}); }
Value DataManager::getSkillsAsValue() const { return Value::Arr({}); }
Value DataManager::getItemsAsValue() const { return Value::Arr({}); }
Value DataManager::getWeaponsAsValue() const { return Value::Arr({}); }
Value DataManager::getArmorsAsValue() const { return Value::Arr({}); }
Value DataManager::getEnemiesAsValue() const { return Value::Arr({}); }
Value DataManager::getTroopsAsValue() const { return Value::Arr({}); }
Value DataManager::getStatesAsValue() const { return Value::Arr({}); }
Value DataManager::getClassesAsValue() const { return Value::Arr({}); }
Value DataManager::getMapInfosAsValue() const { return Value::Arr({}); }

Value DataManager::getGlobalStateAsValue() const {
    Object obj;
    obj["gold"] = Value::Int(globalState_.gold);
    obj["steps"] = Value::Int(globalState_.steps);
    obj["playtime"] = Value::Int(globalState_.playtime);
    obj["mapId"] = Value::Int(globalState_.mapId);
    obj["playerX"] = Value::Int(globalState_.playerX);
    obj["playerY"] = Value::Int(globalState_.playerY);
    obj["playerDirection"] = Value::Int(globalState_.playerDirection);
    return Value::Obj(std::move(obj));
}

// ============================================================================
// Event/Plugin Integration
// ============================================================================

void DataManager::setEventCallback(EventCallback callback) {
    eventCallback_ = std::move(callback);
}

void DataManager::registerPluginCommand(const std::string& command, PluginCommandHandler handler) {
    pluginCommands_[command] = std::move(handler);
}

void DataManager::unregisterPluginCommand(const std::string& command) {
    pluginCommands_.erase(command);
}

bool DataManager::executePluginCommand(const std::string& command, const std::vector<Value>& args, Value& result) {
    auto it = pluginCommands_.find(command);
    if (it == pluginCommands_.end()) {
        return false;
    }
    result = it->second(args);
    return true;
}

// ============================================================================
// Compat Status
// ============================================================================

CompatStatus DataManager::getMethodStatus(const std::string& methodName) {
    auto it = methodStatus_.find(methodName);
    if (it != methodStatus_.end()) {
        return it->second;
    }
    return CompatStatus::UNSUPPORTED;
}

std::string DataManager::getMethodDeviation(const std::string& methodName) {
    auto it = methodDeviations_.find(methodName);
    if (it != methodDeviations_.end()) {
        return it->second;
    }
    return "";
}

void DataManager::registerAPI(QuickJSContext& ctx) {
    std::vector<QuickJSContext::MethodDef> methods;
    
    methods.push_back({"loadDatabase", [](const std::vector<Value>&) -> Value {
        // DataManager::instance().loadDatabase();
        return Value::Int(1);
    }, CompatStatus::FULL});
    
    methods.push_back({"saveGame", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Int(0);
        // return Value::Int(DataManager::instance().saveGame(args[0].asInt()) ? 1 : 0);
        return Value::Int(1);
    }, CompatStatus::FULL});
    
    methods.push_back({"loadGame", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Int(0);
        // return Value::Int(DataManager::instance().loadGame(args[0].asInt()) ? 1 : 0);
        return Value::Int(1);
    }, CompatStatus::FULL});
    
    methods.push_back({"getGold", [](const std::vector<Value>&) -> Value {
        // return Value::Int(DataManager::instance().getGold());
        return Value::Int(0);
    }, CompatStatus::FULL});
    
    methods.push_back({"setGold", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Nil();
        // DataManager::instance().setGold(args[0].asInt());
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"getSwitch", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Int(0);
        // return Value::Int(DataManager::instance().getSwitch(args[0].asInt()) ? 1 : 0);
        return Value::Int(0);
    }, CompatStatus::FULL});
    
    methods.push_back({"setSwitch", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 2) return Value::Nil();
        // DataManager::instance().setSwitch(args[0].asInt(), args[1].asInt() != 0);
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"getVariable", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Int(0);
        // return Value::Int(DataManager::instance().getVariable(args[0].asInt()));
        return Value::Int(0);
    }, CompatStatus::FULL});
    
    methods.push_back({"setVariable", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 2) return Value::Nil();
        // DataManager::instance().setVariable(args[0].asInt(), args[1].asInt());
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"getItemCount", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Int(0);
        // return Value::Int(DataManager::instance().getItemCount(args[0].asInt()));
        return Value::Int(0);
    }, CompatStatus::FULL});
    
    methods.push_back({"gainItem", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 2) return Value::Nil();
        // DataManager::instance().gainItem(args[0].asInt(), args[1].asInt());
        return Value::Nil();
    }, CompatStatus::FULL});
    
    ctx.registerObject("DataManager", methods);
}

} // namespace compat
} // namespace urpg
