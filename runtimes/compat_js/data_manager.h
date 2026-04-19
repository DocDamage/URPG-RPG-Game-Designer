#pragma once

// DataManager - MZ Data/Save/Load Semantics
// Phase 2 - Compat Layer
//
// This defines the DataManager compatibility surface for MZ plugins.
// Per Section 4 - WindowCompat Explicit Surface:
// DataManager save/load semantics + header extensions are required.
//
// The DataManager provides access to game data (actors, items, skills, etc.)
// and save/load operations matching MZ behavior.

#include "quickjs_runtime.h"
#include "engine/runtimes/bridge/value.h"
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace urpg {
namespace compat {

// Forward declarations
class DataManagerImpl;

// Save file header structure (MZ format)
struct SaveHeader {
    int32_t version = 0;
    std::string timestamp;
    int32_t playtimeFrames = 0;
    int32_t mapId = 0;
    int32_t playerId = 0;
    int32_t playerX = 0;
    int32_t playerY = 0;
    std::string partyNames; // Comma-separated party member names for display
    std::string mapDisplayName;
    bool isAutosave = false;
};

// Global game state (MZ $gameXxx variables)
struct GlobalState {
    // Actors
    std::vector<Value> actors;
    
    // Party
    std::vector<int32_t> partyMembers;
    int32_t gold = 0;
    int32_t steps = 0;
    int32_t playtime = 0;
    
    // Map
    int32_t mapId = 0;
    int32_t playerX = 0;
    int32_t playerY = 0;
    int32_t playerDirection = 2;
    
    // Switches and variables
    std::vector<bool> switches;
    std::vector<int32_t> variables;
    
    // Self switches (per-map)
    std::unordered_map<std::string, bool> selfSwitches;
    
    // Items
    std::unordered_map<int32_t, int32_t> items; // itemId -> count
    
    // Weapons/Armor
    std::unordered_map<int32_t, int32_t> weapons;
    std::unordered_map<int32_t, int32_t> armors;
};

// Database data types
struct ActorData {
    int32_t id = 0;
    std::string name;
    std::string nickname;
    int32_t classId = 0;
    int32_t initialLevel = 1;
    int32_t maxLevel = 99;
    int32_t level = 1; // Current level (for display/scaling)
    int32_t exp = 0;
    std::string faceName;
    int32_t faceIndex = 0;
    std::string characterName;
    int32_t characterIndex = 0;
    std::string battlerName;
    int32_t battlerIndex = 0;
    int32_t hp = 100;
    int32_t mp = 30;
    int32_t tp = 0;
    // Params: [param][level] - 0=mhp, 1=mmp, 2=atk, 3=def, 4=mat, 5=mdf, 6=agi, 7=luk
    std::vector<std::vector<int32_t>> params;
    std::vector<int32_t> skills;
};

struct SkillDamage {
    int32_t type = 0;
    int32_t elementId = 0;
    std::string formula;
    int32_t variance = 20;
    bool canCrit = false;
    int32_t power = 10;
};

struct ItemDamage {
    int32_t type = 0;
    int32_t elementId = 0;
    std::string formula;
    int32_t variance = 20;
    bool canCrit = false;
    int32_t power = 10;
};

struct EffectData {
    int32_t code = 0;
    int32_t dataId = 0;
    double value1 = 0.0;
    double value2 = 0.0;
};

struct SkillData {
    int32_t id = 0;
    std::string name;
    std::string description;
    int32_t typeId = 0;
    int32_t scope = 0;
    int32_t mpCost = 0;
    int32_t tpCost = 0;
    int32_t speed = 0;
    int32_t successRate = 100;
    int32_t repeats = 1;
    int32_t animationId = 0;
    SkillDamage damage;
    std::vector<EffectData> effects;
};

struct ItemData {
    int32_t id = 0;
    std::string name;
    int32_t iconIndex = 0;
    std::string description;
    int32_t typeId = 0; // 0=regular, 1=key, 2=hidden
    int32_t occasion = 0; // 0=Always, 1=Battle, 2=Menu, 3=Never
    int32_t consumable = 1;
    int32_t price = 0;
    int32_t scope = 0;
    int32_t animationId = 0;
    ItemDamage damage;
    std::vector<EffectData> effects;
};

struct EnemyData {
    int32_t id = 0;
    std::string name;
    int32_t battlerName = 0;
    int32_t mhp = 100;
    int32_t mmp = 100;
    int32_t atk = 10;
    int32_t def = 10;
    int32_t mat = 10;
    int32_t mdf = 10;
    int32_t agi = 10;
    int32_t luk = 10;
    int32_t exp = 0;
    int32_t gold = 0;
    std::vector<int32_t> dropItems; // [itemId, rate, ...]
};

struct TroopData {
    int32_t id = 0;
    std::string name;
    std::vector<int32_t> members; // Enemy IDs
    // Pages (battle events) - simplified
    Value pages;
};

struct TilesetData {
    int32_t id = 0;
    std::string name;
    int32_t mode = 0;
    std::vector<std::string> tilesetNames;
    std::vector<uint32_t> flags;
};

struct MapData {
    int32_t id = 0;
    int32_t width = 0;
    int32_t height = 0;
    int32_t tilesetId = 0;
    std::vector<std::vector<int32_t>> data; // 6 layers of (width * height)
    Value events;
};

struct ClassData {
    int32_t id = 0;
    std::string name;
    int32_t maxLevel = 99;
    std::vector<int32_t> learnings; // Skill IDs by level
    std::vector<int32_t> expTable; // Exp required to reach next level (index 0 = level 1->2)
    std::vector<std::pair<int32_t, int32_t>> skillsToLearn; // {level, skillId}
    // Params mirror RPG Maker's class table layout: [param][level]
    std::vector<std::vector<int32_t>> params;
};

struct StateData {
    int32_t id = 0;
    std::string name;
    std::string iconIndex;
    int32_t priority = 0;
    int32_t restriction = 0;
    int32_t autoRemovalTiming = 0;
    int32_t minTurns = 1;
    int32_t maxTurns = 1;
};

struct AnimationData {
    int32_t id = 0;
    std::string name;
    std::vector<Value> frames;
};

struct MapInfo {
    int32_t id = 0;
    std::string name;
    int32_t parentId = 0;
    int32_t order = 0;
    bool expanded = false;
};

// DataManager - MZ compatibility layer for data access and save/load
class DataManager {
public:
    DataManager();
    ~DataManager();
    
    // Non-copyable
    DataManager(const DataManager&) = delete;
    DataManager& operator=(const DataManager&) = delete;

    // Singleton access for compatibility.
    static DataManager& instance();
    
    // Configure the data directory path for JSON database loading.
    static void setDataDirectory(const std::string& path);
    static const std::string& getDataDirectory();
    
    // ========================================================================
    // Database Loading
    // ========================================================================
    
    // Status: PARTIAL - Database path currently seeds empty containers; JSON ingestion is still TODO
    bool loadDatabase();
    
    // Status: FULL - Load specific data file
    bool loadActors();
    bool loadClasses();
    bool loadSkills();
    bool loadItems();
    bool loadWeapons();
    bool loadArmors();
    bool loadEnemies();
    bool loadTroops();
    bool loadStates();
    bool loadAnimations();
    bool loadTilesets();
    bool loadCommonEvents();
    bool loadSystem();
    bool loadMapInfos();
    
    // Status: PARTIAL - Prefers real MZ map JSON when available, but still falls back to deterministic mock geometry without a data root
    bool loadMapData(int32_t mapId);
    
    // Status: FULL - Current map data access
    const MapData* getCurrentMap() const;
    const TilesetData* getTileset(int32_t id) const;
    
    // ========================================================================
    // Database Accessors
    // ========================================================================
    
    // Status: PARTIAL - Accessors are live, but loaders currently populate empty database containers
    const std::vector<ActorData>& getActors() const;
    const std::vector<ClassData>& getClasses() const;
    const std::vector<SkillData>& getSkills() const;
    const std::vector<ItemData>& getItems() const;
    const std::vector<ItemData>& getWeapons() const;
    const std::vector<ItemData>& getArmors() const;
    const std::vector<EnemyData>& getEnemies() const;
    const std::vector<TroopData>& getTroops() const;
    const std::vector<StateData>& getStates() const;
    const std::vector<AnimationData>& getAnimations() const;
    const std::vector<MapInfo>& getMapInfos() const;
    
    // Status: PARTIAL - Lookup works against the in-memory containers, which are still loader-empty today
    const ActorData* getActor(int32_t id) const;
    ActorData* getActor(int32_t id);
    int32_t getActorParam(int32_t actorId, int32_t paramId, int32_t level = 1) const;
    const ClassData* getClass(int32_t id) const;
    ClassData* getClass(int32_t id);
    const SkillData* getSkill(int32_t id) const;
    SkillData* getSkill(int32_t id);
    const ItemData* getItem(int32_t id) const;
    ItemData* getItem(int32_t id);
    const ItemData* getWeapon(int32_t id) const;
    const ItemData* getArmor(int32_t id) const;
    const EnemyData* getEnemy(int32_t id) const;
    const TroopData* getTroop(int32_t id) const;
    const StateData* getState(int32_t id) const;
    
    // Status: FULL - System data
    int32_t getStartMapId() const;
    int32_t getStartX() const;
    int32_t getStartY() const;
    int32_t getStartPartySize() const;
    int32_t getStartPartyMember(int32_t index) const;
    
    // ========================================================================
    // Global State ($gameXxx)
    // ========================================================================
    
    // Status: FULL - Initialize new game
    void setupNewGame();
    
    // Status: FULL - Get/set global state
    GlobalState& getGlobalState();
    const GlobalState& getGlobalState() const;
    
    // Status: FULL - Party access
    int32_t getPartySize() const;
    int32_t getPartyMember(int32_t index) const;
    int32_t getGold() const;
    void setGold(int32_t gold);
    void gainGold(int32_t amount);
    void loseGold(int32_t amount);
    
    // Status: FULL - Item inventory
    int32_t getItemCount(int32_t itemId) const;
    void setItemCount(int32_t itemId, int32_t count);
    void gainItem(int32_t itemId, int32_t count);
    void loseItem(int32_t itemId, int32_t count);
    bool hasItem(int32_t itemId) const;
    
    // Status: FULL - Weapon/Armor inventory
    int32_t getWeaponCount(int32_t weaponId) const;
    int32_t getArmorCount(int32_t armorId) const;
    void gainWeapon(int32_t weaponId, int32_t count);
    void loseWeapon(int32_t weaponId, int32_t count);
    void gainArmor(int32_t armorId, int32_t count);
    void loseArmor(int32_t armorId, int32_t count);
    
    // Status: FULL - Switches
    bool getSwitch(int32_t switchId) const;
    void setSwitch(int32_t switchId, bool value);
    
    // Status: FULL - Variables
    int32_t getVariable(int32_t varId) const;
    void setVariable(int32_t varId, int32_t value);
    
    // Status: FULL - Self switches
    bool getSelfSwitch(int32_t mapId, int32_t eventId, const std::string& key) const;
    void setSelfSwitch(int32_t mapId, int32_t eventId, const std::string& key, bool value);
    
    // Status: FULL - Playtime
    int32_t getPlaytime() const;
    void updatePlaytime();
    std::string getPlaytimeString() const;
    
    // Status: FULL - Steps
    int32_t getSteps() const;
    void incrementSteps();

    // Status: FULL - Combat Persistence
    void updateActorHp(int32_t actorId, int32_t hp);
    void updateActorMp(int32_t actorId, int32_t mp);
    void gainExp(int32_t actorId, int32_t exp);
    
    // Status: FULL - Player position
    int32_t getPlayerMapId() const;
    int32_t getPlayerX() const;
    int32_t getPlayerY() const;
    int32_t getPlayerDirection() const;
    void setPlayerPosition(int32_t mapId, int32_t x, int32_t y);
    void setPlayerDirection(int32_t direction);

    // Status: FULL - Map transfer requests
    void reserveTransfer(int32_t mapId, int32_t x, int32_t y, int32_t direction = -1);
    bool isTransferring() const;
    void processTransfer();
    
    // ========================================================================
    // Save/Load Operations
    // ========================================================================
    
    // Status: FULL - Save file management
    int32_t getMaxSaveFiles() const;
    bool doesSaveFileExist(int32_t slot) const;
    bool deleteSaveFile(int32_t slot);
    
    // Status: FULL - Save game
    bool saveGame(int32_t slot);
    bool saveGameWithHeader(int32_t slot, const SaveHeader& header);
    
    // Status: FULL - Load game
    bool loadGame(int32_t slot);
    
    // Status: FULL - Autosave
    bool isAutosaveEnabled() const;
    void setAutosaveEnabled(bool enabled);
    bool saveAutosave();
    bool loadAutosave();
    
    // Status: FULL - Save header access
    std::optional<SaveHeader> getSaveHeader(int32_t slot) const;
    std::vector<SaveHeader> getAllSaveHeaders() const;
    
    // Status: FULL - Header extension for plugins
    bool setSaveHeaderExtension(int32_t slot, const std::string& key, const Value& value);
    std::optional<Value> getSaveHeaderExtension(int32_t slot, const std::string& key) const;
    
    // Status: FULL - Create save header from current state
    SaveHeader createSaveHeader(bool isAutosave = false) const;
    
    // ========================================================================
    // JSON Extraction (MZ $dataXxx format)
    // ========================================================================
    
    // Status: FULL - Extract database as Value for JS access
    Value getActorsAsValue() const;
    Value getSkillsAsValue() const;
    Value getItemsAsValue() const;
    Value getWeaponsAsValue() const;
    Value getArmorsAsValue() const;
    Value getEnemiesAsValue() const;
    Value getTroopsAsValue() const;
    Value getStatesAsValue() const;
    Value getClassesAsValue() const;
    Value getMapInfosAsValue() const;
    
    // Status: FULL - Get current data
    Value getMapDataAsValue() const;
    Value getTilesetsAsValue() const;
    
    // Status: FULL - Extract global state as Value for JS access
    Value getGlobalStateAsValue() const;
    
    // ========================================================================
    // Event/Plugin Integration
    // ========================================================================
    
    // Status: FULL - Event command callbacks
    using EventCallback = std::function<void(const std::string& command, const Value& params)>;
    void setEventCallback(EventCallback callback);
    
    // Status: FULL - Plugin command registration
    using PluginCommandHandler = std::function<Value(const std::vector<Value>& args)>;
    void registerPluginCommand(const std::string& command, PluginCommandHandler handler);
    void unregisterPluginCommand(const std::string& command);
    bool executePluginCommand(const std::string& command, const std::vector<Value>& args, Value& result);
    
    // ========================================================================
    // Compat Status
    // ========================================================================
    
    // Register DataManager API with QuickJS context
    static void registerAPI(QuickJSContext& ctx);
    
    // Get compat status for a method
    static CompatStatus getMethodStatus(const std::string& methodName);
    static std::string getMethodDeviation(const std::string& methodName);
    
private:
    std::unique_ptr<DataManagerImpl> impl_;
    
    // Database storage
    std::vector<ActorData> actors_;
    std::vector<ClassData> classes_;
    std::vector<SkillData> skills_;
    std::vector<ItemData> items_;
    std::vector<ItemData> weapons_;
    std::vector<ItemData> armors_;
    std::vector<EnemyData> enemies_;
    std::vector<TroopData> troops_;
    std::vector<StateData> states_;
    std::vector<AnimationData> animations_;
    std::vector<MapInfo> mapInfos_;
    std::vector<TilesetData> tilesets_;
    
    // Loaded map
    MapData currentMap_;
    
    // Global state
    GlobalState globalState_;
    
    // System data
    int32_t startMapId_ = 1;
    int32_t startX_ = 0;
    int32_t startY_ = 0;
    std::vector<int32_t> startParty_;
    
    // Autosave
    bool autosaveEnabled_ = true;
    
    // Plugin commands
    std::unordered_map<std::string, PluginCommandHandler> pluginCommands_;
    
    // Event callback
    EventCallback eventCallback_;
    
    // Data directory path
    static std::string dataDirectory_;
    
    // API status registry
    static std::unordered_map<std::string, CompatStatus> methodStatus_;
    static std::unordered_map<std::string, std::string> methodDeviations_;
};

} // namespace compat
} // namespace urpg
