// DataManager - MZ Data/Save/Load Semantics - Implementation
// Phase 2 - Compat Layer

#include "data_manager.h"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <fstream>
#include <sstream>
#include <utility>
#include <nlohmann/json.hpp>

namespace urpg {
namespace compat {

// Static member definitions
std::unordered_map<std::string, CompatStatus> DataManager::methodStatus_;
std::unordered_map<std::string, std::string> DataManager::methodDeviations_;
std::string DataManager::dataDirectory_;

void DataManager::setDataDirectory(const std::string& path) {
    dataDirectory_ = path;
    if (!dataDirectory_.empty() && dataDirectory_.back() != '/' && dataDirectory_.back() != '\\') {
        dataDirectory_.push_back('\\');
    }
}

const std::string& DataManager::getDataDirectory() {
    return dataDirectory_;
}

namespace {
constexpr int32_t kAutosaveSlot = 0;

bool isValidSaveSlot(int32_t slot, int32_t maxSaveFiles) {
    return slot >= kAutosaveSlot && slot <= maxSaveFiles;
}

using json = nlohmann::json;

Value jsonToValue(const json& j) {
    if (j.is_null()) {
        return Value::Nil();
    } else if (j.is_boolean()) {
        Value v; v.v = j.get<bool>(); return v;
    } else if (j.is_number_integer()) {
        return Value::Int(j.get<int64_t>());
    } else if (j.is_number_float()) {
        Value v; v.v = j.get<double>(); return v;
    } else if (j.is_string()) {
        Value v; v.v = j.get<std::string>(); return v;
    } else if (j.is_array()) {
        Array arr;
        arr.reserve(j.size());
        for (const auto& elem : j) {
            arr.push_back(jsonToValue(elem));
        }
        return Value::Arr(std::move(arr));
    } else if (j.is_object()) {
        Object obj;
        for (auto it = j.begin(); it != j.end(); ++it) {
            obj[it.key()] = jsonToValue(it.value());
        }
        return Value::Obj(std::move(obj));
    }
    return Value::Nil();
}

std::optional<json> loadJsonFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return std::nullopt;
    }
    try {
        json j;
        file >> j;
        return j;
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<json> loadJsonArrayFile(const std::string& filename) {
    if (DataManager::getDataDirectory().empty()) {
        return std::nullopt;
    }
    auto j = loadJsonFile(DataManager::getDataDirectory() + filename);
    if (!j || !j->is_array()) {
        return std::nullopt;
    }
    return j;
}

void hydrateActorParamsFromClasses(std::vector<ActorData>& actors, const std::vector<ClassData>& classes) {
    for (auto& actor : actors) {
        if (!actor.params.empty() || actor.classId <= 0) {
            continue;
        }
        if (static_cast<size_t>(actor.classId) > classes.size()) {
            continue;
        }

        const auto& cls = classes[static_cast<size_t>(actor.classId - 1)];
        if (!cls.params.empty()) {
            actor.params = cls.params;
        }
    }
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
    hydrateActorParamsFromClasses(actors_, classes_);
    
    impl_->isDatabaseLoaded = ok;
    return ok;
}

bool DataManager::loadActors() {
    actors_.clear();
    if (auto j = loadJsonArrayFile("Actors.json")) {
        for (const auto& elem : *j) {
            if (elem.is_null()) continue;
            ActorData actor;
            actor.id = elem.value("id", 0);
            actor.name = elem.value("name", "");
            actor.nickname = elem.value("nickname", "");
            actor.classId = elem.value("classId", 0);
            actor.initialLevel = elem.value("initialLevel", 1);
            actor.maxLevel = elem.value("maxLevel", 99);
            actor.level = actor.initialLevel;
            actor.faceName = elem.value("faceName", "");
            actor.faceIndex = elem.value("faceIndex", 0);
            actor.characterName = elem.value("characterName", "");
            actor.characterIndex = elem.value("characterIndex", 0);
            actor.battlerName = elem.value("battlerName", "");
            actor.battlerIndex = 0;
            if (elem.contains("params") && elem["params"].is_array()) {
                for (const auto& row : elem["params"]) {
                    std::vector<int32_t> r;
                    for (const auto& v : row) {
                        r.push_back(v.get<int32_t>());
                    }
                    actor.params.push_back(std::move(r));
                }
            }
            if (actor.id > 0 && static_cast<size_t>(actor.id) > actors_.size()) {
                actors_.resize(static_cast<size_t>(actor.id));
            }
            if (actor.id > 0) {
                actors_[static_cast<size_t>(actor.id) - 1] = std::move(actor);
            }
        }
        hydrateActorParamsFromClasses(actors_, classes_);
        return true;
    }
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
    hydrateActorParamsFromClasses(actors_, classes_);
    return true;
}

bool DataManager::loadClasses() {
    classes_.clear();
    if (auto j = loadJsonArrayFile("Classes.json")) {
        for (const auto& elem : *j) {
            if (elem.is_null()) continue;
            ClassData cls;
            cls.id = elem.value("id", 0);
            cls.name = elem.value("name", "");
            if (elem.contains("learnings") && elem["learnings"].is_array()) {
                for (const auto& ln : elem["learnings"]) {
                    if (ln.contains("skillId")) {
                        cls.learnings.push_back(ln["skillId"].get<int32_t>());
                    }
                }
            }
            if (elem.contains("params") && elem["params"].is_array()) {
                for (const auto& row : elem["params"]) {
                    std::vector<int32_t> r;
                    for (const auto& v : row) {
                        r.push_back(v.get<int32_t>());
                    }
                    cls.params.push_back(std::move(r));
                }
            }
            if (cls.id > 0 && static_cast<size_t>(cls.id) > classes_.size()) {
                classes_.resize(static_cast<size_t>(cls.id));
            }
            if (cls.id > 0) {
                classes_[static_cast<size_t>(cls.id) - 1] = std::move(cls);
            }
        }
        hydrateActorParamsFromClasses(actors_, classes_);
        return true;
    }
    ClassData warrior;
    warrior.id = 1;
    warrior.name = "Warrior";
    classes_.push_back(std::move(warrior));
    hydrateActorParamsFromClasses(actors_, classes_);
    return true;
}

bool DataManager::loadSkills() {
    skills_.clear();
    if (auto j = loadJsonArrayFile("Skills.json")) {
        for (const auto& elem : *j) {
            if (elem.is_null()) continue;
            SkillData skill;
            skill.id = elem.value("id", 0);
            skill.name = elem.value("name", "");
            skill.description = elem.value("description", "");
            skill.typeId = elem.value("stypeId", 0);
            skill.scope = elem.value("scope", 0);
            skill.mpCost = elem.value("mpCost", 0);
            skill.tpCost = elem.value("tpCost", 0);
            skill.speed = elem.value("speed", 0);
            skill.successRate = elem.value("successRate", 100);
            skill.repeats = elem.value("repeats", 1);
            skill.animationId = elem.value("animationId", 0);
            if (skill.id > 0 && static_cast<size_t>(skill.id) > skills_.size()) {
                skills_.resize(static_cast<size_t>(skill.id));
            }
            if (skill.id > 0) {
                skills_[static_cast<size_t>(skill.id) - 1] = std::move(skill);
            }
        }
        return true;
    }
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
    if (auto j = loadJsonArrayFile("Items.json")) {
        for (const auto& elem : *j) {
            if (elem.is_null()) continue;
            ItemData item;
            item.id = elem.value("id", 0);
            item.name = elem.value("name", "");
            item.iconIndex = elem.value("iconIndex", 0);
            item.description = elem.value("description", "");
            item.typeId = elem.value("itypeId", 0);
            item.occasion = elem.value("occasion", 0);
            item.consumable = elem.value("consumable", true) ? 1 : 0;
            item.price = elem.value("price", 0);
            item.scope = elem.value("scope", 0);
            item.animationId = elem.value("animationId", 0);
            if (item.id > 0 && static_cast<size_t>(item.id) > items_.size()) {
                items_.resize(static_cast<size_t>(item.id));
            }
            if (item.id > 0) {
                items_[static_cast<size_t>(item.id) - 1] = std::move(item);
            }
        }
        return true;
    }
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
    if (auto j = loadJsonArrayFile("Weapons.json")) {
        for (const auto& elem : *j) {
            if (elem.is_null()) continue;
            ItemData weapon;
            weapon.id = elem.value("id", 0);
            weapon.name = elem.value("name", "");
            weapon.iconIndex = elem.value("iconIndex", 0);
            weapon.description = elem.value("description", "");
            weapon.typeId = elem.value("wtypeId", 0);
            weapon.occasion = 0;
            weapon.consumable = 0;
            weapon.price = elem.value("price", 0);
            weapon.scope = 0;
            weapon.animationId = elem.value("animationId", 0);
            if (weapon.id > 0 && static_cast<size_t>(weapon.id) > weapons_.size()) {
                weapons_.resize(static_cast<size_t>(weapon.id));
            }
            if (weapon.id > 0) {
                weapons_[static_cast<size_t>(weapon.id) - 1] = std::move(weapon);
            }
        }
        return true;
    }
    return true;
}

bool DataManager::loadArmors() {
    armors_.clear();
    if (auto j = loadJsonArrayFile("Armors.json")) {
        for (const auto& elem : *j) {
            if (elem.is_null()) continue;
            ItemData armor;
            armor.id = elem.value("id", 0);
            armor.name = elem.value("name", "");
            armor.iconIndex = elem.value("iconIndex", 0);
            armor.description = elem.value("description", "");
            armor.typeId = elem.value("atypeId", 0);
            armor.occasion = 0;
            armor.consumable = 0;
            armor.price = elem.value("price", 0);
            armor.scope = 0;
            armor.animationId = 0;
            if (armor.id > 0 && static_cast<size_t>(armor.id) > armors_.size()) {
                armors_.resize(static_cast<size_t>(armor.id));
            }
            if (armor.id > 0) {
                armors_[static_cast<size_t>(armor.id) - 1] = std::move(armor);
            }
        }
        return true;
    }
    return true;
}

bool DataManager::loadEnemies() {
    enemies_.clear();
    if (auto j = loadJsonArrayFile("Enemies.json")) {
        for (const auto& elem : *j) {
            if (elem.is_null()) continue;
            EnemyData enemy;
            enemy.id = elem.value("id", 0);
            enemy.name = elem.value("name", "");
            enemy.battlerName = 0; // struct field is int32_t; JSON provides string
            enemy.mhp = 100;
            enemy.mmp = 100;
            enemy.atk = 10;
            enemy.def = 10;
            enemy.mat = 10;
            enemy.mdf = 10;
            enemy.agi = 10;
            enemy.luk = 10;
            if (elem.contains("params") && elem["params"].is_array() && elem["params"].size() >= 8) {
                enemy.mhp = elem["params"][0].get<int32_t>();
                enemy.mmp = elem["params"][1].get<int32_t>();
                enemy.atk = elem["params"][2].get<int32_t>();
                enemy.def = elem["params"][3].get<int32_t>();
                enemy.mat = elem["params"][4].get<int32_t>();
                enemy.mdf = elem["params"][5].get<int32_t>();
                enemy.agi = elem["params"][6].get<int32_t>();
                enemy.luk = elem["params"][7].get<int32_t>();
            }
            enemy.exp = elem.value("exp", 0);
            enemy.gold = elem.value("gold", 0);
            if (enemy.id > 0 && static_cast<size_t>(enemy.id) > enemies_.size()) {
                enemies_.resize(static_cast<size_t>(enemy.id));
            }
            if (enemy.id > 0) {
                enemies_[static_cast<size_t>(enemy.id) - 1] = std::move(enemy);
            }
        }
        return true;
    }
    EnemyData slime;
    slime.id = 1;
    slime.name = "Slime";
    slime.mhp = 30;
    slime.mmp = 0;
    slime.atk = 8;
    slime.def = 5;
    slime.mat = 4;
    slime.mdf = 4;
    slime.agi = 10;
    slime.luk = 6;
    slime.exp = 12;
    slime.gold = 5;
    slime.dropItems = {1, 1}; // itemId 1, rate 1 (100%)
    enemies_.push_back(std::move(slime));

    EnemyData goblin;
    goblin.id = 2;
    goblin.name = "Goblin";
    goblin.mhp = 50;
    goblin.mmp = 0;
    goblin.atk = 12;
    goblin.def = 8;
    goblin.mat = 4;
    goblin.mdf = 4;
    goblin.agi = 15;
    goblin.luk = 8;
    goblin.exp = 20;
    goblin.gold = 10;
    goblin.dropItems = {1, 1, 2, 2}; // item 1 (100%), item 2 (50%)
    enemies_.push_back(std::move(goblin));
    return true;
}

bool DataManager::loadTroops() {
    troops_.clear();
    if (auto j = loadJsonArrayFile("Troops.json")) {
        for (const auto& elem : *j) {
            if (elem.is_null()) continue;
            TroopData troop;
            troop.id = elem.value("id", 0);
            troop.name = elem.value("name", "");
            if (elem.contains("members") && elem["members"].is_array()) {
                for (const auto& m : elem["members"]) {
                    if (m.contains("enemyId")) {
                        troop.members.push_back(m["enemyId"].get<int32_t>());
                    }
                }
            }
            if (elem.contains("pages")) {
                troop.pages = jsonToValue(elem["pages"]);
            }
            if (troop.id > 0 && static_cast<size_t>(troop.id) > troops_.size()) {
                troops_.resize(static_cast<size_t>(troop.id));
            }
            if (troop.id > 0) {
                troops_[static_cast<size_t>(troop.id) - 1] = std::move(troop);
            }
        }
        return true;
    }
    TroopData troop1;
    troop1.id = 1;
    troop1.name = "Slime x2";
    troop1.members = {1, 1};
    troops_.push_back(std::move(troop1));

    TroopData troop2;
    troop2.id = 2;
    troop2.name = "Goblin";
    troop2.members = {2};
    troops_.push_back(std::move(troop2));
    return true;
}

bool DataManager::loadStates() {
    states_.clear();
    if (auto j = loadJsonArrayFile("States.json")) {
        for (const auto& elem : *j) {
            if (elem.is_null()) continue;
            StateData state;
            state.id = elem.value("id", 0);
            state.name = elem.value("name", "");
            if (elem.contains("iconIndex")) {
                if (elem["iconIndex"].is_number()) {
                    state.iconIndex = std::to_string(elem["iconIndex"].get<int32_t>());
                } else {
                    state.iconIndex = elem.value("iconIndex", "");
                }
            }
            state.priority = elem.value("priority", 0);
            state.restriction = elem.value("restriction", 0);
            state.autoRemovalTiming = elem.value("autoRemovalTiming", 0);
            state.minTurns = elem.value("minTurns", 1);
            state.maxTurns = elem.value("maxTurns", 1);
            if (state.id > 0 && static_cast<size_t>(state.id) > states_.size()) {
                states_.resize(static_cast<size_t>(state.id));
            }
            if (state.id > 0) {
                states_[static_cast<size_t>(state.id) - 1] = std::move(state);
            }
        }
        return true;
    }
    return true;
}

bool DataManager::loadAnimations() {
    animations_.clear();
    if (auto j = loadJsonArrayFile("Animations.json")) {
        for (const auto& elem : *j) {
            if (elem.is_null()) continue;
            AnimationData anim;
            anim.id = elem.value("id", 0);
            anim.name = elem.value("name", "");
            if (elem.contains("frames") && elem["frames"].is_array()) {
                for (const auto& f : elem["frames"]) {
                    anim.frames.push_back(jsonToValue(f));
                }
            }
            if (anim.id > 0 && static_cast<size_t>(anim.id) > animations_.size()) {
                animations_.resize(static_cast<size_t>(anim.id));
            }
            if (anim.id > 0) {
                animations_[static_cast<size_t>(anim.id) - 1] = std::move(anim);
            }
        }
        return true;
    }
    return true;
}

bool DataManager::loadTilesets() {
    tilesets_.clear();
    if (auto j = loadJsonArrayFile("Tilesets.json")) {
        for (const auto& elem : *j) {
            if (elem.is_null()) continue;
            TilesetData ts;
            ts.id = elem.value("id", 0);
            ts.name = elem.value("name", "");
            ts.mode = elem.value("mode", 0);
            if (elem.contains("tilesetNames") && elem["tilesetNames"].is_array()) {
                for (const auto& t : elem["tilesetNames"]) {
                    ts.tilesetNames.push_back(t.get<std::string>());
                }
            }
            if (elem.contains("flags") && elem["flags"].is_array()) {
                for (const auto& f : elem["flags"]) {
                    ts.flags.push_back(f.get<uint32_t>());
                }
            }
            if (ts.id > 0 && static_cast<size_t>(ts.id) > tilesets_.size()) {
                tilesets_.resize(static_cast<size_t>(ts.id));
            }
            if (ts.id > 0) {
                tilesets_[static_cast<size_t>(ts.id) - 1] = std::move(ts);
            }
        }
        return true;
    }
    return true;
}

bool DataManager::loadCommonEvents() {
    return true;
}

bool DataManager::loadSystem() {
    if (auto j = loadJsonFile(dataDirectory_ + "System.json")) {
        startMapId_ = j->value("startMapId", 1);
        startX_ = j->value("startX", 0);
        startY_ = j->value("startY", 0);
        startParty_.clear();
        if (j->contains("partyMembers") && (*j)["partyMembers"].is_array()) {
            for (const auto& m : (*j)["partyMembers"]) {
                startParty_.push_back(m.get<int32_t>());
            }
        }
        return true;
    }
    startMapId_ = 1;
    startX_ = 8;
    startY_ = 6;
    startParty_ = {1};
    return true;
}

bool DataManager::loadMapInfos() {
    mapInfos_.clear();
    if (auto j = loadJsonArrayFile("MapInfos.json")) {
        for (const auto& elem : *j) {
            if (elem.is_null()) continue;
            MapInfo info;
            info.id = elem.value("id", 0);
            info.name = elem.value("name", "");
            info.parentId = elem.value("parentId", 0);
            info.order = elem.value("order", 0);
            info.expanded = elem.value("expanded", false);
            if (info.id > 0 && static_cast<size_t>(info.id) > mapInfos_.size()) {
                mapInfos_.resize(static_cast<size_t>(info.id));
            }
            if (info.id > 0) {
                mapInfos_[static_cast<size_t>(info.id) - 1] = std::move(info);
            }
        }
        return true;
    }
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

int32_t DataManager::getActorParam(int32_t actorId, int32_t paramId, int32_t level) const {
    const auto* actor = getActor(actorId);
    if (actor == nullptr || paramId < 0) {
        return 10;
    }

    const auto readParamTable = [paramId, level](const std::vector<std::vector<int32_t>>& table) -> std::optional<int32_t> {
        if (paramId < 0 || static_cast<size_t>(paramId) >= table.size()) {
            return std::nullopt;
        }
        const auto& row = table[static_cast<size_t>(paramId)];
        if (row.empty()) {
            return std::nullopt;
        }

        const size_t clampedLevel = static_cast<size_t>(std::clamp(level, 0, static_cast<int32_t>(row.size() - 1)));
        return row[clampedLevel];
    };

    if (const auto actorParam = readParamTable(actor->params)) {
        return *actorParam;
    }

    if (const auto* cls = getClass(actor->classId)) {
        if (const auto classParam = readParamTable(cls->params)) {
            return *classParam;
        }
    }

    switch (paramId) {
        case 0: return 100;
        case 1: return 30;
        default: return 10;
    }
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
