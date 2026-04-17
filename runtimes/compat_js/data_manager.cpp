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
        const auto setStatus = [](const std::string& method,
                                  CompatStatus status,
                                  const std::string& deviation = "") {
            methodStatus_[method] = status;
            if (deviation.empty()) {
                methodDeviations_.erase(method);
            } else {
                methodDeviations_[method] = deviation;
            }
        };

        // Save/Load
        setStatus("loadDatabase", CompatStatus::PARTIAL);
        setStatus("saveGame", CompatStatus::PARTIAL);
        setStatus("saveGameWithHeader", CompatStatus::PARTIAL);
        setStatus("loadGame", CompatStatus::PARTIAL);
        setStatus("deleteSaveFile", CompatStatus::PARTIAL);
        setStatus("doesSaveFileExist", CompatStatus::PARTIAL);
        setStatus("getMaxSaveFiles", CompatStatus::PARTIAL);
        setStatus("getSaveHeader", CompatStatus::PARTIAL);
        setStatus("getAllSaveHeaders", CompatStatus::PARTIAL);
        setStatus("saveAutosave", CompatStatus::PARTIAL);
        setStatus("loadAutosave", CompatStatus::PARTIAL);
        setStatus("isAutosaveEnabled", CompatStatus::PARTIAL);
        setStatus("setAutosaveEnabled", CompatStatus::PARTIAL);
        setStatus("setSaveHeaderExtension", CompatStatus::PARTIAL);
        setStatus("getSaveHeaderExtension", CompatStatus::PARTIAL);
        
        // Database access
        setStatus("getActors", CompatStatus::PARTIAL);
        setStatus("getSkills", CompatStatus::PARTIAL);
        setStatus("getItems", CompatStatus::PARTIAL);
        setStatus("getWeapons", CompatStatus::PARTIAL);
        setStatus("getArmors", CompatStatus::PARTIAL);
        setStatus("getEnemies", CompatStatus::PARTIAL);
        setStatus("getTroops", CompatStatus::PARTIAL);
        setStatus("getStates", CompatStatus::PARTIAL);
        setStatus("getActor", CompatStatus::PARTIAL);
        setStatus("getSkill", CompatStatus::PARTIAL);
        setStatus("getItem", CompatStatus::PARTIAL);
        
        // Global state
        setStatus("setupNewGame", CompatStatus::PARTIAL);
        setStatus("getPartySize", CompatStatus::PARTIAL);
        setStatus("getPartyMember", CompatStatus::PARTIAL);
        setStatus("getGold", CompatStatus::PARTIAL);
        setStatus("setGold", CompatStatus::PARTIAL);
        setStatus("gainGold", CompatStatus::PARTIAL);
        setStatus("loseGold", CompatStatus::PARTIAL);
        setStatus("getItemCount", CompatStatus::PARTIAL);
        setStatus("gainItem", CompatStatus::PARTIAL);
        setStatus("loseItem", CompatStatus::PARTIAL);
        setStatus("hasItem", CompatStatus::PARTIAL);
        setStatus("getSwitch", CompatStatus::PARTIAL);
        setStatus("setSwitch", CompatStatus::PARTIAL);
        setStatus("getVariable", CompatStatus::PARTIAL);
        setStatus("setVariable", CompatStatus::PARTIAL);
        setStatus("getSelfSwitch", CompatStatus::PARTIAL);
        setStatus("setSelfSwitch", CompatStatus::PARTIAL);
        setStatus("getPlaytime", CompatStatus::PARTIAL);
        setStatus("getPlaytimeString", CompatStatus::PARTIAL);
        setStatus("getSteps", CompatStatus::PARTIAL);
        setStatus("incrementSteps", CompatStatus::PARTIAL);
        setStatus("getPlayerMapId", CompatStatus::PARTIAL);
        setStatus("getPlayerX", CompatStatus::PARTIAL);
        setStatus("getPlayerY", CompatStatus::PARTIAL);
        setStatus("setPlayerPosition", CompatStatus::PARTIAL);
        setStatus("reserveTransfer", CompatStatus::PARTIAL);
        setStatus("isTransferring", CompatStatus::PARTIAL);
        setStatus("processTransfer", CompatStatus::PARTIAL);
        
        // Plugin commands
        setStatus("registerPluginCommand", CompatStatus::PARTIAL);
        setStatus("unregisterPluginCommand", CompatStatus::PARTIAL);
        setStatus("executePluginCommand", CompatStatus::PARTIAL);
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
    bool ok = true;
    ok = loadActors() && ok;
    ok = loadClasses() && ok;
    ok = loadSkills() && ok;
    ok = loadItems() && ok;
    ok = loadWeapons() && ok;
    ok = loadArmors() && ok;
    ok = loadEnemies() && ok;
    ok = loadTroops() && ok;
    ok = loadStates() && ok;
    ok = loadAnimations() && ok;
    ok = loadTilesets() && ok;
    ok = loadCommonEvents() && ok;
    ok = loadSystem() && ok;
    ok = loadMapInfos() && ok;
    
    impl_->isDatabaseLoaded = ok;
    return ok;
}

bool DataManager::loadActors() {
    actors_.clear();
    ActorData hero;
    hero.id = 1;
    hero.name = "Hero";
    hero.nickname = "";
    hero.classId = 1;
    hero.initialLevel = 1;
    hero.maxLevel = 99;
    hero.level = 1;
    hero.faceName = "Actor1";
    hero.faceIndex = 0;
    hero.characterName = "Actor1";
    hero.characterIndex = 0;
    hero.battlerName = "Actor1_1";
    hero.battlerIndex = 0;
    actors_.push_back(std::move(hero));
    return true;
}

bool DataManager::loadClasses() {
    classes_.clear();
    ClassData warrior;
    warrior.id = 1;
    warrior.name = "Warrior";
    classes_.push_back(std::move(warrior));
    return true;
}

bool DataManager::loadSkills() {
    skills_.clear();
    SkillData heal;
    heal.id = 1;
    heal.name = "Heal";
    heal.description = "Restores HP to one ally.";
    heal.typeId = 1;
    heal.scope = 7;
    heal.mpCost = 5;
    heal.tpCost = 0;
    heal.speed = 0;
    heal.successRate = 100;
    heal.repeats = 1;
    heal.animationId = 41;
    skills_.push_back(std::move(heal));
    return true;
}

bool DataManager::loadItems() {
    items_.clear();
    ItemData potion;
    potion.id = 1;
    potion.name = "Potion";
    potion.iconIndex = 176;
    potion.description = "Restores 200 HP.";
    potion.typeId = 0;
    potion.occasion = 0;
    potion.consumable = 1;
    potion.price = 50;
    potion.scope = 7;
    potion.animationId = 41;
    items_.push_back(std::move(potion));
    return true;
}

bool DataManager::loadWeapons() {
    weapons_.clear();
    return true;
}

bool DataManager::loadArmors() {
    armors_.clear();
    return true;
}

bool DataManager::loadEnemies() {
    enemies_.clear();
    return true;
}

bool DataManager::loadTroops() {
    troops_.clear();
    return true;
}

bool DataManager::loadStates() {
    states_.clear();
    return true;
}

bool DataManager::loadAnimations() {
    animations_.clear();
    return true;
}

bool DataManager::loadTilesets() {
    tilesets_.clear();
    return true;
}

bool DataManager::loadCommonEvents() {
    return true;
}

bool DataManager::loadSystem() {
    startMapId_ = 1;
    startX_ = 8;
    startY_ = 6;
    startParty_ = {1};
    return true;
}

bool DataManager::loadMapInfos() {
    mapInfos_.clear();
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

Value DataManager::getActorsAsValue() const {
    Array arr;
    for (const auto& actor : actors_) {
        Object obj;
        obj["id"] = Value::Int(actor.id);
        Value name; name.v = actor.name; obj["name"] = std::move(name);
        Value nickname; nickname.v = actor.nickname; obj["nickname"] = std::move(nickname);
        obj["classId"] = Value::Int(actor.classId);
        obj["initialLevel"] = Value::Int(actor.initialLevel);
        obj["maxLevel"] = Value::Int(actor.maxLevel);
        obj["level"] = Value::Int(actor.level);
        Value faceName; faceName.v = actor.faceName; obj["faceName"] = std::move(faceName);
        obj["faceIndex"] = Value::Int(actor.faceIndex);
        Value characterName; characterName.v = actor.characterName; obj["characterName"] = std::move(characterName);
        obj["characterIndex"] = Value::Int(actor.characterIndex);
        Value battlerName; battlerName.v = actor.battlerName; obj["battlerName"] = std::move(battlerName);
        obj["battlerIndex"] = Value::Int(actor.battlerIndex);
        arr.push_back(Value::Obj(std::move(obj)));
    }
    return Value::Arr(std::move(arr));
}

Value DataManager::getSkillsAsValue() const {
    Array arr;
    for (const auto& skill : skills_) {
        Object obj;
        obj["id"] = Value::Int(skill.id);
        Value name; name.v = skill.name; obj["name"] = std::move(name);
        Value desc; desc.v = skill.description; obj["description"] = std::move(desc);
        obj["typeId"] = Value::Int(skill.typeId);
        obj["scope"] = Value::Int(skill.scope);
        obj["mpCost"] = Value::Int(skill.mpCost);
        obj["tpCost"] = Value::Int(skill.tpCost);
        obj["speed"] = Value::Int(skill.speed);
        obj["successRate"] = Value::Int(skill.successRate);
        obj["repeats"] = Value::Int(skill.repeats);
        obj["animationId"] = Value::Int(skill.animationId);
        arr.push_back(Value::Obj(std::move(obj)));
    }
    return Value::Arr(std::move(arr));
}

Value DataManager::getItemsAsValue() const {
    Array arr;
    for (const auto& item : items_) {
        Object obj;
        obj["id"] = Value::Int(item.id);
        Value name; name.v = item.name; obj["name"] = std::move(name);
        obj["iconIndex"] = Value::Int(item.iconIndex);
        Value desc; desc.v = item.description; obj["description"] = std::move(desc);
        obj["typeId"] = Value::Int(item.typeId);
        obj["occasion"] = Value::Int(item.occasion);
        obj["consumable"] = Value::Int(item.consumable);
        obj["price"] = Value::Int(item.price);
        obj["scope"] = Value::Int(item.scope);
        obj["animationId"] = Value::Int(item.animationId);
        arr.push_back(Value::Obj(std::move(obj)));
    }
    return Value::Arr(std::move(arr));
}

Value DataManager::getWeaponsAsValue() const {
    Array arr;
    for (const auto& weapon : weapons_) {
        Object obj;
        obj["id"] = Value::Int(weapon.id);
        Value name; name.v = weapon.name; obj["name"] = std::move(name);
        obj["iconIndex"] = Value::Int(weapon.iconIndex);
        Value desc; desc.v = weapon.description; obj["description"] = std::move(desc);
        obj["typeId"] = Value::Int(weapon.typeId);
        obj["occasion"] = Value::Int(weapon.occasion);
        obj["consumable"] = Value::Int(weapon.consumable);
        obj["price"] = Value::Int(weapon.price);
        obj["scope"] = Value::Int(weapon.scope);
        obj["animationId"] = Value::Int(weapon.animationId);
        arr.push_back(Value::Obj(std::move(obj)));
    }
    return Value::Arr(std::move(arr));
}

Value DataManager::getArmorsAsValue() const {
    Array arr;
    for (const auto& armor : armors_) {
        Object obj;
        obj["id"] = Value::Int(armor.id);
        Value name; name.v = armor.name; obj["name"] = std::move(name);
        obj["iconIndex"] = Value::Int(armor.iconIndex);
        Value desc; desc.v = armor.description; obj["description"] = std::move(desc);
        obj["typeId"] = Value::Int(armor.typeId);
        obj["occasion"] = Value::Int(armor.occasion);
        obj["consumable"] = Value::Int(armor.consumable);
        obj["price"] = Value::Int(armor.price);
        obj["scope"] = Value::Int(armor.scope);
        obj["animationId"] = Value::Int(armor.animationId);
        arr.push_back(Value::Obj(std::move(obj)));
    }
    return Value::Arr(std::move(arr));
}

Value DataManager::getEnemiesAsValue() const { return Value::Arr({}); }
Value DataManager::getTroopsAsValue() const { return Value::Arr({}); }
Value DataManager::getStatesAsValue() const { return Value::Arr({}); }
Value DataManager::getClassesAsValue() const {
    Array arr;
    for (const auto& cls : classes_) {
        Object obj;
        obj["id"] = Value::Int(cls.id);
        Value name; name.v = cls.name; obj["name"] = std::move(name);
        arr.push_back(Value::Obj(std::move(obj)));
    }
    return Value::Arr(std::move(arr));
}

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
        return Value::Int(DataManager::instance().loadDatabase() ? 1 : 0);
    }, CompatStatus::PARTIAL});
    
    methods.push_back({"saveGame", [](const std::vector<Value>& args) -> Value {
        if (args.empty() || !std::holds_alternative<int64_t>(args[0].v)) return Value::Int(0);
        return Value::Int(DataManager::instance().saveGame(static_cast<int32_t>(std::get<int64_t>(args[0].v))) ? 1 : 0);
    }, CompatStatus::PARTIAL});
    
    methods.push_back({"loadGame", [](const std::vector<Value>& args) -> Value {
        if (args.empty() || !std::holds_alternative<int64_t>(args[0].v)) return Value::Int(0);
        return Value::Int(DataManager::instance().loadGame(static_cast<int32_t>(std::get<int64_t>(args[0].v))) ? 1 : 0);
    }, CompatStatus::PARTIAL});
    
    methods.push_back({"getGold", [](const std::vector<Value>&) -> Value {
        return Value::Int(DataManager::instance().getGold());
    }, CompatStatus::PARTIAL});
    
    methods.push_back({"setGold", [](const std::vector<Value>& args) -> Value {
        if (args.empty() || !std::holds_alternative<int64_t>(args[0].v)) return Value::Nil();
        DataManager::instance().setGold(static_cast<int32_t>(std::get<int64_t>(args[0].v)));
        return Value::Nil();
    }, CompatStatus::PARTIAL});
    
    methods.push_back({"getSwitch", [](const std::vector<Value>& args) -> Value {
        if (args.empty() || !std::holds_alternative<int64_t>(args[0].v)) return Value::Int(0);
        return Value::Int(DataManager::instance().getSwitch(static_cast<int32_t>(std::get<int64_t>(args[0].v))) ? 1 : 0);
    }, CompatStatus::PARTIAL});
    
    methods.push_back({"setSwitch", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 2 || !std::holds_alternative<int64_t>(args[0].v)) return Value::Nil();
        bool val = false;
        if (std::holds_alternative<bool>(args[1].v)) val = std::get<bool>(args[1].v);
        else if (std::holds_alternative<int64_t>(args[1].v)) val = std::get<int64_t>(args[1].v) != 0;
        DataManager::instance().setSwitch(static_cast<int32_t>(std::get<int64_t>(args[0].v)), val);
        return Value::Nil();
    }, CompatStatus::PARTIAL});
    
    methods.push_back({"getVariable", [](const std::vector<Value>& args) -> Value {
        if (args.empty() || !std::holds_alternative<int64_t>(args[0].v)) return Value::Int(0);
        return Value::Int(DataManager::instance().getVariable(static_cast<int32_t>(std::get<int64_t>(args[0].v))));
    }, CompatStatus::PARTIAL});
    
    methods.push_back({"setVariable", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 2 || !std::holds_alternative<int64_t>(args[0].v)) return Value::Nil();
        int32_t val = 0;
        if (std::holds_alternative<int64_t>(args[1].v)) val = static_cast<int32_t>(std::get<int64_t>(args[1].v));
        else if (std::holds_alternative<double>(args[1].v)) val = static_cast<int32_t>(std::get<double>(args[1].v));
        DataManager::instance().setVariable(static_cast<int32_t>(std::get<int64_t>(args[0].v)), val);
        return Value::Nil();
    }, CompatStatus::PARTIAL});
    
    methods.push_back({"getItemCount", [](const std::vector<Value>& args) -> Value {
        if (args.empty() || !std::holds_alternative<int64_t>(args[0].v)) return Value::Int(0);
        return Value::Int(DataManager::instance().getItemCount(static_cast<int32_t>(std::get<int64_t>(args[0].v))));
    }, CompatStatus::PARTIAL});
    
    methods.push_back({"gainItem", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 2 || !std::holds_alternative<int64_t>(args[0].v) || !std::holds_alternative<int64_t>(args[1].v)) return Value::Nil();
        DataManager::instance().gainItem(static_cast<int32_t>(std::get<int64_t>(args[0].v)), static_cast<int32_t>(std::get<int64_t>(args[1].v)));
        return Value::Nil();
    }, CompatStatus::PARTIAL});
    
    ctx.registerObject("DataManager", methods);
}

} // namespace compat
} // namespace urpg
