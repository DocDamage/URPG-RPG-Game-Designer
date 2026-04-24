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

#include "runtimes/compat_js/data_manager_types.h"
#include "quickjs_runtime.h"

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

    // Status: PARTIAL - Loads seeded compat records and real JSON data when a data root is configured; full project parity is still out of scope
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

    // Status: PARTIAL - Accessors return live seeded/loaded compat containers; coverage still depends on available project data
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

    // Status: PARTIAL - Lookup works against the in-memory seeded/loaded containers
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
