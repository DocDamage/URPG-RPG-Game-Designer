// DataManager - MZ Data/Save/Load Semantics - Implementation
// Phase 2 - Compat Layer

#include "data_manager.h"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <sstream>
#include <utility>

namespace urpg {
namespace compat {

// Static member definitions
std::unordered_map<std::string, CompatStatus> DataManager::methodStatus_;
std::unordered_map<std::string, std::string> DataManager::methodDeviations_;

namespace {
using json = nlohmann::json;

constexpr int32_t kAutosaveSlot = 0;

bool isValidSaveSlot(int32_t slot, int32_t maxSaveFiles) {
    return slot >= kAutosaveSlot && slot <= maxSaveFiles;
}

std::vector<std::vector<int32_t>> parseParams(const json& j) {
    std::vector<std::vector<int32_t>> params;
    if (!j.is_array()) return params;
    for (const auto& row : j) {
        std::vector<int32_t> inner;
        if (row.is_array()) {
            for (const auto& v : row) {
                inner.push_back(v.get<int32_t>());
            }
        }
        params.push_back(std::move(inner));
    }
    return params;
}

std::vector<int32_t> parseIntArray(const json& j) {
    std::vector<int32_t> result;
    if (!j.is_array()) return result;
    for (const auto& v : j) {
        if (!v.is_null()) {
            result.push_back(v.get<int32_t>());
        }
    }
    return result;
}

std::vector<Value> parseValueArray(const json& j) {
    std::vector<Value> result;
    if (!j.is_array()) return result;
    for (const auto& v : j) {
        if (v.is_null()) {
            result.push_back(Value::Nil());
        } else if (v.is_boolean()) {
            result.push_back(Value::Int(v.get<bool>() ? 1 : 0));
        } else if (v.is_number_integer()) {
            result.push_back(Value::Int(v.get<int64_t>()));
        } else if (v.is_number_float()) {
            Value out;
            out.v = v.get<double>();
            result.push_back(std::move(out));
        } else if (v.is_string()) {
            result.push_back(Value::Str(v.get<std::string>()));
        } else if (v.is_array()) {
            result.push_back(Value::Arr(parseValueArray(v)));
        } else if (v.is_object()) {
            Object obj;
            for (const auto& [key, val] : v.items()) {
                if (val.is_null()) {
                    obj[key] = Value::Nil();
                } else if (val.is_boolean()) {
                    obj[key] = Value::Int(val.get<bool>() ? 1 : 0);
                } else if (val.is_number_integer()) {
                    obj[key] = Value::Int(val.get<int64_t>());
                } else if (val.is_number_float()) {
                    Value out;
                    out.v = val.get<double>();
                    obj[key] = std::move(out);
                } else if (val.is_string()) {
                    obj[key] = Value::Str(val.get<std::string>());
                } else if (val.is_array()) {
                    obj[key] = Value::Arr(parseValueArray(val));
                } else if (val.is_object()) {
                    Object nested;
                    for (const auto& [k2, v2] : val.items()) {
                        if (v2.is_null()) {
                            nested[k2] = Value::Nil();
                        } else if (v2.is_boolean()) {
                            nested[k2] = Value::Int(v2.get<bool>() ? 1 : 0);
                        } else if (v2.is_number_integer()) {
                            nested[k2] = Value::Int(v2.get<int64_t>());
                        } else if (v2.is_number_float()) {
                            Value out2;
                            out2.v = v2.get<double>();
                            nested[k2] = std::move(out2);
                        } else if (v2.is_string()) {
                            nested[k2] = Value::Str(v2.get<std::string>());
                        } else if (v2.is_array()) {
                            nested[k2] = Value::Arr(parseValueArray(v2));
                        } else if (v2.is_object()) {
                            nested[k2] = Value::Obj(Object{});
                        }
                    }
                    obj[key] = Value::Obj(std::move(nested));
                }
            }
            result.push_back(Value::Obj(std::move(obj)));
        }
    }
    return result;
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
        setStatus("loadDatabase", CompatStatus::PARTIAL,
                  "Database load path still seeds empty containers; JSON database ingestion is TODO.");
        setStatus("saveGame", CompatStatus::FULL);
        setStatus("saveGameWithHeader", CompatStatus::FULL);
        setStatus("loadGame", CompatStatus::FULL);
        setStatus("deleteSaveFile", CompatStatus::FULL);
        setStatus("doesSaveFileExist", CompatStatus::FULL);
        setStatus("getMaxSaveFiles", CompatStatus::FULL);
        setStatus("getSaveHeader", CompatStatus::FULL);
        setStatus("getAllSaveHeaders", CompatStatus::FULL);
        setStatus("saveAutosave", CompatStatus::FULL);
        setStatus("loadAutosave", CompatStatus::FULL);
        setStatus("isAutosaveEnabled", CompatStatus::FULL);
        setStatus("setAutosaveEnabled", CompatStatus::FULL);
        setStatus("setSaveHeaderExtension", CompatStatus::FULL);
        setStatus("getSaveHeaderExtension", CompatStatus::FULL);
        
        // Database access
        setStatus("getActors", CompatStatus::PARTIAL,
                  "Returns live containers, but loader currently populates no actor records.");
        setStatus("getSkills", CompatStatus::PARTIAL,
                  "Returns live containers, but loader currently populates no skill records.");
        setStatus("getItems", CompatStatus::PARTIAL,
                  "Returns live containers, but loader currently populates no item records.");
        setStatus("getWeapons", CompatStatus::PARTIAL,
                  "Returns live containers, but loader currently populates no weapon records.");
        setStatus("getArmors", CompatStatus::PARTIAL,
                  "Returns live containers, but loader currently populates no armor records.");
        setStatus("getEnemies", CompatStatus::PARTIAL,
                  "Returns live containers, but loader currently populates no enemy records.");
        setStatus("getTroops", CompatStatus::PARTIAL,
                  "Returns live containers, but loader currently populates no troop records.");
        setStatus("getStates", CompatStatus::PARTIAL,
                  "Returns live containers, but loader currently populates no state records.");
        setStatus("getActor", CompatStatus::PARTIAL,
                  "Actor lookup works against an in-memory list that is still loader-empty.");
        setStatus("getSkill", CompatStatus::PARTIAL,
                  "Skill lookup works against an in-memory list that is still loader-empty.");
        setStatus("getItem", CompatStatus::PARTIAL,
                  "Item lookup works against an in-memory list that is still loader-empty.");
        
        // Global state
        setStatus("setupNewGame", CompatStatus::FULL);
        setStatus("getPartySize", CompatStatus::FULL);
        setStatus("getPartyMember", CompatStatus::FULL);
        setStatus("getGold", CompatStatus::FULL);
        setStatus("setGold", CompatStatus::FULL);
        setStatus("gainGold", CompatStatus::FULL);
        setStatus("loseGold", CompatStatus::FULL);
        setStatus("getItemCount", CompatStatus::FULL);
        setStatus("gainItem", CompatStatus::FULL);
        setStatus("loseItem", CompatStatus::FULL);
        setStatus("hasItem", CompatStatus::FULL);
        setStatus("getSwitch", CompatStatus::FULL);
        setStatus("setSwitch", CompatStatus::FULL);
        setStatus("getVariable", CompatStatus::FULL);
        setStatus("setVariable", CompatStatus::FULL);
        setStatus("getSelfSwitch", CompatStatus::FULL);
        setStatus("setSelfSwitch", CompatStatus::FULL);
        setStatus("getPlaytime", CompatStatus::FULL);
        setStatus("getPlaytimeString", CompatStatus::FULL);
        setStatus("getSteps", CompatStatus::FULL);
        setStatus("incrementSteps", CompatStatus::FULL);
        setStatus("getPlayerMapId", CompatStatus::FULL);
        setStatus("getPlayerX", CompatStatus::FULL);
        setStatus("getPlayerY", CompatStatus::FULL);
        setStatus("setPlayerPosition", CompatStatus::FULL);
        setStatus("reserveTransfer", CompatStatus::FULL);
        setStatus("isTransferring", CompatStatus::FULL);
        setStatus("processTransfer", CompatStatus::FULL);
        
        // Plugin commands
        setStatus("registerPluginCommand", CompatStatus::FULL);
        setStatus("unregisterPluginCommand", CompatStatus::FULL);
        setStatus("executePluginCommand", CompatStatus::FULL);
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

void DataManager::setDataPath(const std::string& path) {
    dataPath_ = path;
}

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
    impl_->isDatabaseLoaded = true;
    return ok;
}

bool DataManager::loadActors() {
    if (dataPath_.empty()) {
        actors_.clear();
        return true;
    }
    std::string filename = dataPath_ + "/Actors.json";
    std::ifstream file(filename);
    if (!file) {
        actors_.clear();
        return false;
    }
    try {
        json j = json::parse(file);
        if (!j.is_array()) {
            actors_.clear();
            return false;
        }
        actors_.clear();
        actors_.reserve(j.size() > 1 ? j.size() - 1 : 0);
        for (size_t i = 1; i < j.size(); ++i) {
            if (j[i].is_null()) continue;
            const auto& e = j[i];
            ActorData a;
            a.id = e.value("id", 0);
            a.name = e.value("name", std::string());
            a.nickname = e.value("nickname", std::string());
            a.classId = e.value("classId", 0);
            a.initialLevel = e.value("initialLevel", 1);
            a.maxLevel = e.value("maxLevel", 99);
            a.faceName = e.value("faceName", std::string());
            a.faceIndex = e.value("faceIndex", 0);
            a.characterName = e.value("characterName", std::string());
            a.characterIndex = e.value("characterIndex", 0);
            a.battlerName = e.value("battlerName", std::string());
            a.battlerIndex = e.value("battlerIndex", 0);
            if (e.contains("params") && e["params"].is_array()) {
                a.params = parseParams(e["params"]);
            }
            actors_.push_back(std::move(a));
        }
        return true;
    } catch (...) {
        actors_.clear();
        return false;
    }
}

bool DataManager::loadClasses() {
    if (dataPath_.empty()) {
        classes_.clear();
        return true;
    }
    std::string filename = dataPath_ + "/Classes.json";
    std::ifstream file(filename);
    if (!file) {
        classes_.clear();
        return false;
    }
    try {
        json j = json::parse(file);
        if (!j.is_array()) {
            classes_.clear();
            return false;
        }
        classes_.clear();
        classes_.reserve(j.size() > 1 ? j.size() - 1 : 0);
        for (size_t i = 1; i < j.size(); ++i) {
            if (j[i].is_null()) continue;
            const auto& e = j[i];
            ClassData c;
            c.id = e.value("id", 0);
            c.name = e.value("name", std::string());
            if (e.contains("params") && e["params"].is_array()) {
                c.params = parseParams(e["params"]);
            }
            if (e.contains("learnings") && e["learnings"].is_array()) {
                c.learnings = parseIntArray(e["learnings"]);
            }
            classes_.push_back(std::move(c));
        }
        return true;
    } catch (...) {
        classes_.clear();
        return false;
    }
}

bool DataManager::loadSkills() {
    if (dataPath_.empty()) {
        skills_.clear();
        return true;
    }
    std::string filename = dataPath_ + "/Skills.json";
    std::ifstream file(filename);
    if (!file) {
        skills_.clear();
        return false;
    }
    try {
        json j = json::parse(file);
        if (!j.is_array()) {
            skills_.clear();
            return false;
        }
        skills_.clear();
        skills_.reserve(j.size() > 1 ? j.size() - 1 : 0);
        for (size_t i = 1; i < j.size(); ++i) {
            if (j[i].is_null()) continue;
            const auto& e = j[i];
            SkillData s;
            s.id = e.value("id", 0);
            s.name = e.value("name", std::string());
            s.description = e.value("description", std::string());
            s.typeId = e.value("typeId", 0);
            s.scope = e.value("scope", 0);
            s.mpCost = e.value("mpCost", 0);
            s.tpCost = e.value("tpCost", 0);
            s.speed = e.value("speed", 0);
            s.successRate = e.value("successRate", 100);
            s.repeats = e.value("repeats", 1);
            s.animationId = e.value("animationId", 0);
            skills_.push_back(std::move(s));
        }
        return true;
    } catch (...) {
        skills_.clear();
        return false;
    }
}

bool DataManager::loadItems() {
    if (dataPath_.empty()) {
        items_.clear();
        return true;
    }
    std::string filename = dataPath_ + "/Items.json";
    std::ifstream file(filename);
    if (!file) {
        items_.clear();
        return false;
    }
    try {
        json j = json::parse(file);
        if (!j.is_array()) {
            items_.clear();
            return false;
        }
        items_.clear();
        items_.reserve(j.size() > 1 ? j.size() - 1 : 0);
        for (size_t i = 1; i < j.size(); ++i) {
            if (j[i].is_null()) continue;
            const auto& e = j[i];
            ItemData item;
            item.id = e.value("id", 0);
            item.name = e.value("name", std::string());
            item.iconIndex = e.value("iconIndex", 0);
            item.description = e.value("description", std::string());
            item.typeId = e.value("typeId", 0);
            item.consumable = e.value("consumable", 1);
            item.price = e.value("price", 0);
            item.scope = e.value("scope", 0);
            item.animationId = e.value("animationId", 0);
            items_.push_back(std::move(item));
        }
        return true;
    } catch (...) {
        items_.clear();
        return false;
    }
}

bool DataManager::loadWeapons() {
    if (dataPath_.empty()) {
        weapons_.clear();
        return true;
    }
    std::string filename = dataPath_ + "/Weapons.json";
    std::ifstream file(filename);
    if (!file) {
        weapons_.clear();
        return false;
    }
    try {
        json j = json::parse(file);
        if (!j.is_array()) {
            weapons_.clear();
            return false;
        }
        weapons_.clear();
        weapons_.reserve(j.size() > 1 ? j.size() - 1 : 0);
        for (size_t i = 1; i < j.size(); ++i) {
            if (j[i].is_null()) continue;
            const auto& e = j[i];
            ItemData item;
            item.id = e.value("id", 0);
            item.name = e.value("name", std::string());
            item.iconIndex = e.value("iconIndex", 0);
            item.description = e.value("description", std::string());
            item.typeId = e.value("typeId", 0);
            item.consumable = e.value("consumable", 0);
            item.price = e.value("price", 0);
            item.scope = e.value("scope", 0);
            item.animationId = e.value("animationId", 0);
            weapons_.push_back(std::move(item));
        }
        return true;
    } catch (...) {
        weapons_.clear();
        return false;
    }
}

bool DataManager::loadArmors() {
    if (dataPath_.empty()) {
        armors_.clear();
        return true;
    }
    std::string filename = dataPath_ + "/Armors.json";
    std::ifstream file(filename);
    if (!file) {
        armors_.clear();
        return false;
    }
    try {
        json j = json::parse(file);
        if (!j.is_array()) {
            armors_.clear();
            return false;
        }
        armors_.clear();
        armors_.reserve(j.size() > 1 ? j.size() - 1 : 0);
        for (size_t i = 1; i < j.size(); ++i) {
            if (j[i].is_null()) continue;
            const auto& e = j[i];
            ItemData item;
            item.id = e.value("id", 0);
            item.name = e.value("name", std::string());
            item.iconIndex = e.value("iconIndex", 0);
            item.description = e.value("description", std::string());
            item.typeId = e.value("typeId", 0);
            item.consumable = e.value("consumable", 0);
            item.price = e.value("price", 0);
            item.scope = e.value("scope", 0);
            item.animationId = e.value("animationId", 0);
            armors_.push_back(std::move(item));
        }
        return true;
    } catch (...) {
        armors_.clear();
        return false;
    }
}

bool DataManager::loadEnemies() {
    if (dataPath_.empty()) {
        enemies_.clear();
        return true;
    }
    std::string filename = dataPath_ + "/Enemies.json";
    std::ifstream file(filename);
    if (!file) {
        enemies_.clear();
        return false;
    }
    try {
        json j = json::parse(file);
        if (!j.is_array()) {
            enemies_.clear();
            return false;
        }
        enemies_.clear();
        enemies_.reserve(j.size() > 1 ? j.size() - 1 : 0);
        for (size_t i = 1; i < j.size(); ++i) {
            if (j[i].is_null()) continue;
            const auto& e = j[i];
            EnemyData en;
            en.id = e.value("id", 0);
            en.name = e.value("name", std::string());
            en.battlerName = e.value("battlerName", std::string());
            en.mhp = e.value("mhp", 100);
            en.mmp = e.value("mmp", 100);
            en.atk = e.value("atk", 10);
            en.def = e.value("def", 10);
            en.mat = e.value("mat", 10);
            en.mdf = e.value("mdf", 10);
            en.agi = e.value("agi", 10);
            en.luk = e.value("luk", 10);
            en.exp = e.value("exp", 0);
            en.gold = e.value("gold", 0);
            if (e.contains("dropItems") && e["dropItems"].is_array()) {
                en.dropItems = parseIntArray(e["dropItems"]);
            }
            enemies_.push_back(std::move(en));
        }
        return true;
    } catch (...) {
        enemies_.clear();
        return false;
    }
}

bool DataManager::loadTroops() {
    if (dataPath_.empty()) {
        troops_.clear();
        return true;
    }
    std::string filename = dataPath_ + "/Troops.json";
    std::ifstream file(filename);
    if (!file) {
        troops_.clear();
        return false;
    }
    try {
        json j = json::parse(file);
        if (!j.is_array()) {
            troops_.clear();
            return false;
        }
        troops_.clear();
        troops_.reserve(j.size() > 1 ? j.size() - 1 : 0);
        for (size_t i = 1; i < j.size(); ++i) {
            if (j[i].is_null()) continue;
            const auto& e = j[i];
            TroopData t;
            t.id = e.value("id", 0);
            t.name = e.value("name", std::string());
            if (e.contains("members") && e["members"].is_array()) {
                for (const auto& m : e["members"]) {
                    if (m.is_number_integer()) {
                        t.members.push_back(m.get<int32_t>());
                    } else if (m.is_object() && m.contains("enemyId")) {
                        t.members.push_back(m["enemyId"].get<int32_t>());
                    }
                }
            }
            if (e.contains("pages") && e["pages"].is_array()) {
                t.pages = parseValueArray(e["pages"]);
            }
            troops_.push_back(std::move(t));
        }
        return true;
    } catch (...) {
        troops_.clear();
        return false;
    }
}

bool DataManager::loadStates() {
    if (dataPath_.empty()) {
        states_.clear();
        return true;
    }
    std::string filename = dataPath_ + "/States.json";
    std::ifstream file(filename);
    if (!file) {
        states_.clear();
        return false;
    }
    try {
        json j = json::parse(file);
        if (!j.is_array()) {
            states_.clear();
            return false;
        }
        states_.clear();
        states_.reserve(j.size() > 1 ? j.size() - 1 : 0);
        for (size_t i = 1; i < j.size(); ++i) {
            if (j[i].is_null()) continue;
            const auto& e = j[i];
            StateData s;
            s.id = e.value("id", 0);
            s.name = e.value("name", std::string());
            s.iconIndex = e.value("iconIndex", 0);
            s.priority = e.value("priority", 0);
            s.restriction = e.value("restriction", 0);
            s.autoRemovalTiming = e.value("autoRemovalTiming", 0);
            s.minTurns = e.value("minTurns", 1);
            s.maxTurns = e.value("maxTurns", 1);
            s.slipDamage = e.value("slipDamage", 0);
            s.removeByDamage = e.value("removeByDamage", false);
            s.removeByWalking = e.value("removeByWalking", false);
            s.chanceByDamage = e.value("chanceByDamage", 100);
            states_.push_back(std::move(s));
        }
        return true;
    } catch (...) {
        states_.clear();
        return false;
    }
}

bool DataManager::loadAnimations() {
    if (dataPath_.empty()) {
        animations_.clear();
        return true;
    }
    std::string filename = dataPath_ + "/Animations.json";
    std::ifstream file(filename);
    if (!file) {
        animations_.clear();
        return false;
    }
    try {
        json j = json::parse(file);
        if (!j.is_array()) {
            animations_.clear();
            return false;
        }
        animations_.clear();
        animations_.reserve(j.size() > 1 ? j.size() - 1 : 0);
        for (size_t i = 1; i < j.size(); ++i) {
            if (j[i].is_null()) continue;
            const auto& e = j[i];
            AnimationData a;
            a.id = e.value("id", 0);
            a.name = e.value("name", std::string());
            if (e.contains("frames") && e["frames"].is_array()) {
                a.frames = parseValueArray(e["frames"]);
            }
            animations_.push_back(std::move(a));
        }
        return true;
    } catch (...) {
        animations_.clear();
        return false;
    }
}

bool DataManager::loadTilesets() {
    if (dataPath_.empty()) return true;
    std::string filename = dataPath_ + "/Tilesets.json";
    std::ifstream file(filename);
    if (!file) return false;
    try {
        json j = json::parse(file);
        if (!j.is_array()) return false;
        return true;
    } catch (...) {
        return false;
    }
}

bool DataManager::loadCommonEvents() {
    if (dataPath_.empty()) return true;
    std::string filename = dataPath_ + "/CommonEvents.json";
    std::ifstream file(filename);
    if (!file) return false;
    try {
        json j = json::parse(file);
        if (!j.is_array()) return false;
        return true;
    } catch (...) {
        return false;
    }
}

bool DataManager::loadSystem() {
    if (dataPath_.empty()) {
        startMapId_ = 1;
        startX_ = 0;
        startY_ = 0;
        startParty_.clear();
        return true;
    }
    std::string filename = dataPath_ + "/System.json";
    std::ifstream file(filename);
    if (!file) {
        startMapId_ = 1;
        startX_ = 0;
        startY_ = 0;
        startParty_.clear();
        return false;
    }
    try {
        json j = json::parse(file);
        startMapId_ = j.value("startMapId", 1);
        startX_ = j.value("startX", 0);
        startY_ = j.value("startY", 0);
        startParty_.clear();
        if (j.contains("partyMembers") && j["partyMembers"].is_array()) {
            for (const auto& v : j["partyMembers"]) {
                startParty_.push_back(v.get<int32_t>());
            }
        }
        return true;
    } catch (...) {
        startMapId_ = 1;
        startX_ = 0;
        startY_ = 0;
        startParty_.clear();
        return false;
    }
}

bool DataManager::loadMapInfos() {
    if (dataPath_.empty()) {
        mapInfos_.clear();
        return true;
    }
    std::string filename = dataPath_ + "/MapInfos.json";
    std::ifstream file(filename);
    if (!file) {
        mapInfos_.clear();
        return false;
    }
    try {
        json j = json::parse(file);
        if (!j.is_array()) {
            mapInfos_.clear();
            return false;
        }
        mapInfos_.clear();
        mapInfos_.reserve(j.size() > 1 ? j.size() - 1 : 0);
        for (size_t i = 1; i < j.size(); ++i) {
            if (j[i].is_null()) continue;
            const auto& e = j[i];
            MapInfo m;
            m.id = e.value("id", 0);
            m.name = e.value("name", std::string());
            m.parentId = e.value("parentId", 0);
            m.order = e.value("order", 0);
            m.expanded = e.value("expanded", false);
            mapInfos_.push_back(std::move(m));
        }
        return true;
    } catch (...) {
        mapInfos_.clear();
        return false;
    }
}

bool DataManager::loadMapData(int32_t mapId) {
    if (dataPath_.empty()) {
        impl_->loadedMapId = mapId;
        currentMap_.id = mapId;
        currentMap_.width = 20; 
        currentMap_.height = 15;
        currentMap_.tilesetId = 1;
        
        currentMap_.data.clear();
        currentMap_.data.resize(6, std::vector<int32_t>(300, 0));

        for (int i = 0; i < 300; ++i) {
            int x = i % 20;
            int y = i / 20;
            if (x == 0 || x == 19 || y == 0 || y == 14) {
                currentMap_.data[0][i] = 1;
            } else {
                currentMap_.data[0][i] = 0;
            }
        }
        return true;
    }
    std::ostringstream oss;
    oss << dataPath_ << "/Map" << std::setw(3) << std::setfill('0') << mapId << ".json";
    std::string filename = oss.str();
    std::ifstream file(filename);
    if (!file) {
        impl_->loadedMapId = 0;
        return false;
    }
    try {
        json j = json::parse(file);
        impl_->loadedMapId = mapId;
        currentMap_.id = mapId;
        return true;
    } catch (...) {
        impl_->loadedMapId = 0;
        return false;
    }
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

EnemyData* DataManager::getEnemy(int32_t id) {
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

    setupGameActors();
    
    // Initialize runtime party / troop state
    gameParty_ = GameParty{};
    gameParty_.setMembers(globalState_.partyMembers);
    gameTroop_ = GameTroop{};
}

GlobalState& DataManager::getGlobalState() {
    return globalState_;
}

const GlobalState& DataManager::getGlobalState() const {
    return globalState_;
}

GameParty& DataManager::getGameParty() {
    return gameParty_;
}

const GameParty& DataManager::getGameParty() const {
    return gameParty_;
}

GameTroop& DataManager::getGameTroop() {
    return gameTroop_;
}

const GameTroop& DataManager::getGameTroop() const {
    return gameTroop_;
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
// Runtime Actor State ($gameActors compat)
// ============================================================================

GameActor* DataManager::getGameActor(int32_t actorId) {
    auto it = gameActors_.find(actorId);
    if (it != gameActors_.end()) {
        return &it->second;
    }
    return nullptr;
}

const GameActor* DataManager::getGameActor(int32_t actorId) const {
    auto it = gameActors_.find(actorId);
    if (it != gameActors_.end()) {
        return &it->second;
    }
    return nullptr;
}

void DataManager::setupGameActors() {
    gameActors_.clear();
    for (const auto& actor : actors_) {
        GameActor ga;
        ga.actorId = actor.id;
        ga.level = actor.initialLevel;
        int32_t levelIndex = actor.initialLevel - 1;
        if (levelIndex >= 0 && !actor.params.empty() && static_cast<size_t>(levelIndex) < actor.params.size()) {
            const auto& p = actor.params[levelIndex];
            if (!p.empty()) ga.mhp = p[0];
            if (p.size() > 1) ga.mmp = p[1];
            if (p.size() > 2) ga.atk = p[2];
            if (p.size() > 3) ga.def = p[3];
            if (p.size() > 4) ga.mat = p[4];
            if (p.size() > 5) ga.mdf = p[5];
            if (p.size() > 6) ga.agi = p[6];
            if (p.size() > 7) ga.luk = p[7];
        }
        ga.hp = ga.mhp;
        ga.mp = ga.mmp;
        ga.tp = 0;
        ga.mtp = 100; // TP max defaults to 100; plugins may override
        gameActors_[actor.id] = std::move(ga);
    }
}

void DataManager::setGameActorHp(int32_t actorId, int32_t hp) {
    auto it = gameActors_.find(actorId);
    if (it != gameActors_.end()) {
        it->second.hp = hp;
    }
}

void DataManager::setGameActorMp(int32_t actorId, int32_t mp) {
    auto it = gameActors_.find(actorId);
    if (it != gameActors_.end()) {
        it->second.mp = mp;
    }
}

void DataManager::setGameActorTp(int32_t actorId, int32_t tp) {
    auto it = gameActors_.find(actorId);
    if (it != gameActors_.end()) {
        it->second.tp = tp;
    }
}

void DataManager::setGameActorLevel(int32_t actorId, int32_t level) {
    auto it = gameActors_.find(actorId);
    if (it != gameActors_.end()) {
        it->second.level = level;
    }
}

void DataManager::setGameActorMtp(int32_t actorId, int32_t mtp) {
    auto it = gameActors_.find(actorId);
    if (it != gameActors_.end()) {
        it->second.mtp = mtp;
    }
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
    arr.reserve(actors_.size());
    for (const auto& a : actors_) {
        Object obj;
        obj["id"] = Value::Int(a.id);
        obj["name"] = Value::Str(a.name);
        obj["nickname"] = Value::Str(a.nickname);
        obj["classId"] = Value::Int(a.classId);
        obj["initialLevel"] = Value::Int(a.initialLevel);
        obj["maxLevel"] = Value::Int(a.maxLevel);
        obj["faceName"] = Value::Str(a.faceName);
        obj["faceIndex"] = Value::Int(a.faceIndex);
        obj["characterName"] = Value::Str(a.characterName);
        obj["characterIndex"] = Value::Int(a.characterIndex);
        obj["battlerName"] = Value::Str(a.battlerName);
        obj["battlerIndex"] = Value::Int(a.battlerIndex);
        Array paramsArr;
        for (const auto& row : a.params) {
            Array inner;
            for (int32_t v : row) {
                inner.push_back(Value::Int(v));
            }
            paramsArr.push_back(Value::Arr(std::move(inner)));
        }
        obj["params"] = Value::Arr(std::move(paramsArr));
        arr.push_back(Value::Obj(std::move(obj)));
    }
    return Value::Arr(std::move(arr));
}

Value DataManager::getSkillsAsValue() const {
    Array arr;
    arr.reserve(skills_.size());
    for (const auto& s : skills_) {
        Object obj;
        obj["id"] = Value::Int(s.id);
        obj["name"] = Value::Str(s.name);
        obj["description"] = Value::Str(s.description);
        obj["typeId"] = Value::Int(s.typeId);
        obj["scope"] = Value::Int(s.scope);
        obj["mpCost"] = Value::Int(s.mpCost);
        obj["tpCost"] = Value::Int(s.tpCost);
        obj["speed"] = Value::Int(s.speed);
        obj["successRate"] = Value::Int(s.successRate);
        obj["repeats"] = Value::Int(s.repeats);
        obj["animationId"] = Value::Int(s.animationId);
        arr.push_back(Value::Obj(std::move(obj)));
    }
    return Value::Arr(std::move(arr));
}

Value DataManager::getItemsAsValue() const {
    Array arr;
    arr.reserve(items_.size());
    for (const auto& item : items_) {
        Object obj;
        obj["id"] = Value::Int(item.id);
        obj["name"] = Value::Str(item.name);
        obj["iconIndex"] = Value::Int(item.iconIndex);
        obj["description"] = Value::Str(item.description);
        obj["typeId"] = Value::Int(item.typeId);
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
    arr.reserve(weapons_.size());
    for (const auto& item : weapons_) {
        Object obj;
        obj["id"] = Value::Int(item.id);
        obj["name"] = Value::Str(item.name);
        obj["iconIndex"] = Value::Int(item.iconIndex);
        obj["description"] = Value::Str(item.description);
        obj["typeId"] = Value::Int(item.typeId);
        obj["consumable"] = Value::Int(item.consumable);
        obj["price"] = Value::Int(item.price);
        obj["scope"] = Value::Int(item.scope);
        obj["animationId"] = Value::Int(item.animationId);
        arr.push_back(Value::Obj(std::move(obj)));
    }
    return Value::Arr(std::move(arr));
}

Value DataManager::getArmorsAsValue() const {
    Array arr;
    arr.reserve(armors_.size());
    for (const auto& item : armors_) {
        Object obj;
        obj["id"] = Value::Int(item.id);
        obj["name"] = Value::Str(item.name);
        obj["iconIndex"] = Value::Int(item.iconIndex);
        obj["description"] = Value::Str(item.description);
        obj["typeId"] = Value::Int(item.typeId);
        obj["consumable"] = Value::Int(item.consumable);
        obj["price"] = Value::Int(item.price);
        obj["scope"] = Value::Int(item.scope);
        obj["animationId"] = Value::Int(item.animationId);
        arr.push_back(Value::Obj(std::move(obj)));
    }
    return Value::Arr(std::move(arr));
}

Value DataManager::getEnemiesAsValue() const {
    Array arr;
    arr.reserve(enemies_.size());
    for (const auto& e : enemies_) {
        Object obj;
        obj["id"] = Value::Int(e.id);
        obj["name"] = Value::Str(e.name);
        obj["battlerName"] = Value::Str(e.battlerName);
        obj["mhp"] = Value::Int(e.mhp);
        obj["mmp"] = Value::Int(e.mmp);
        obj["atk"] = Value::Int(e.atk);
        obj["def"] = Value::Int(e.def);
        obj["mat"] = Value::Int(e.mat);
        obj["mdf"] = Value::Int(e.mdf);
        obj["agi"] = Value::Int(e.agi);
        obj["luk"] = Value::Int(e.luk);
        obj["exp"] = Value::Int(e.exp);
        obj["gold"] = Value::Int(e.gold);
        Array drops;
        for (int32_t d : e.dropItems) {
            drops.push_back(Value::Int(d));
        }
        obj["dropItems"] = Value::Arr(std::move(drops));
        arr.push_back(Value::Obj(std::move(obj)));
    }
    return Value::Arr(std::move(arr));
}

Value DataManager::getTroopsAsValue() const {
    Array arr;
    arr.reserve(troops_.size());
    for (const auto& t : troops_) {
        Object obj;
        obj["id"] = Value::Int(t.id);
        obj["name"] = Value::Str(t.name);
        Array members;
        for (int32_t m : t.members) {
            members.push_back(Value::Int(m));
        }
        obj["members"] = Value::Arr(std::move(members));
        obj["pages"] = Value::Arr(t.pages);
        arr.push_back(Value::Obj(std::move(obj)));
    }
    return Value::Arr(std::move(arr));
}

Value DataManager::getStatesAsValue() const {
    Array arr;
    arr.reserve(states_.size());
    for (const auto& s : states_) {
        Object obj;
        obj["id"] = Value::Int(s.id);
        obj["name"] = Value::Str(s.name);
        obj["iconIndex"] = Value::Int(s.iconIndex);
        obj["priority"] = Value::Int(s.priority);
        obj["restriction"] = Value::Int(s.restriction);
        obj["autoRemovalTiming"] = Value::Int(s.autoRemovalTiming);
        obj["minTurns"] = Value::Int(s.minTurns);
        obj["maxTurns"] = Value::Int(s.maxTurns);
        arr.push_back(Value::Obj(std::move(obj)));
    }
    return Value::Arr(std::move(arr));
}

Value DataManager::getClassesAsValue() const {
    Array arr;
    arr.reserve(classes_.size());
    for (const auto& c : classes_) {
        Object obj;
        obj["id"] = Value::Int(c.id);
        obj["name"] = Value::Str(c.name);
        Array learnings;
        for (int32_t s : c.learnings) {
            learnings.push_back(Value::Int(s));
        }
        obj["learnings"] = Value::Arr(std::move(learnings));
        Array paramsArr;
        for (const auto& row : c.params) {
            Array inner;
            for (int32_t v : row) {
                inner.push_back(Value::Int(v));
            }
            paramsArr.push_back(Value::Arr(std::move(inner)));
        }
        obj["params"] = Value::Arr(std::move(paramsArr));
        arr.push_back(Value::Obj(std::move(obj)));
    }
    return Value::Arr(std::move(arr));
}

Value DataManager::getMapInfosAsValue() const {
    Array arr;
    arr.reserve(mapInfos_.size());
    for (const auto& m : mapInfos_) {
        Object obj;
        obj["id"] = Value::Int(m.id);
        obj["name"] = Value::Str(m.name);
        obj["parentId"] = Value::Int(m.parentId);
        obj["order"] = Value::Int(m.order);
        obj["expanded"] = Value::Int(m.expanded ? 1 : 0);
        arr.push_back(Value::Obj(std::move(obj)));
    }
    return Value::Arr(std::move(arr));
}

Value DataManager::getGlobalStateAsValue() const {
    Object obj;
    obj["gold"] = Value::Int(globalState_.gold);
    obj["steps"] = Value::Int(globalState_.steps);
    obj["playtime"] = Value::Int(globalState_.playtime);
    obj["mapId"] = Value::Int(globalState_.mapId);
    obj["playerX"] = Value::Int(globalState_.playerX);
    obj["playerY"] = Value::Int(globalState_.playerY);
    obj["playerDirection"] = Value::Int(globalState_.playerDirection);
    Array party;
    for (int32_t m : globalState_.partyMembers) {
        party.push_back(Value::Int(m));
    }
    obj["partyMembers"] = Value::Arr(std::move(party));
    Array switches;
    for (bool s : globalState_.switches) {
        switches.push_back(Value::Int(s ? 1 : 0));
    }
    obj["switches"] = Value::Arr(std::move(switches));
    Array variables;
    for (int32_t v : globalState_.variables) {
        variables.push_back(Value::Int(v));
    }
    obj["variables"] = Value::Arr(std::move(variables));
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
// Test Helpers
// ============================================================================

void DataManager::clearDatabase() {
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
    globalState_ = GlobalState{};
    gameActors_.clear();
    gameParty_ = GameParty{};
    gameTroop_ = GameTroop{};
    impl_->isDatabaseLoaded = false;
}

EnemyData& DataManager::addTestEnemy() {
    enemies_.push_back(EnemyData{});
    enemies_.back().id = static_cast<int32_t>(enemies_.size());
    return enemies_.back();
}

TroopData& DataManager::addTestTroop() {
    troops_.push_back(TroopData{});
    troops_.back().id = static_cast<int32_t>(troops_.size());
    return troops_.back();
}

ActorData& DataManager::addTestActor() {
    actors_.push_back(ActorData{});
    actors_.back().id = static_cast<int32_t>(actors_.size());
    return actors_.back();
}

ItemData& DataManager::addTestItem() {
    items_.push_back(ItemData{});
    items_.back().id = static_cast<int32_t>(items_.size());
    return items_.back();
}

SkillData& DataManager::addTestSkill() {
    skills_.push_back(SkillData{});
    skills_.back().id = static_cast<int32_t>(skills_.size());
    return skills_.back();
}

StateData& DataManager::addTestState() {
    states_.push_back(StateData{});
    states_.back().id = static_cast<int32_t>(states_.size());
    return states_.back();
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

namespace {

int64_t valueToInt64(const urpg::Value& value, int64_t fallback = 0) {
    if (const auto* integer = std::get_if<int64_t>(&value.v)) {
        return *integer;
    }
    if (const auto* real = std::get_if<double>(&value.v)) {
        return static_cast<int64_t>(std::llround(*real));
    }
    if (const auto* flag = std::get_if<bool>(&value.v)) {
        return *flag ? 1 : 0;
    }
    if (const auto* text = std::get_if<std::string>(&value.v)) {
        try {
            size_t consumed = 0;
            const int64_t parsed = std::stoll(*text, &consumed, 10);
            if (consumed == text->size()) {
                return parsed;
            }
        } catch (...) {
        }
        try {
            size_t consumed = 0;
            const double parsed = std::stod(*text, &consumed);
            if (consumed == text->size()) {
                return static_cast<int64_t>(std::llround(parsed));
            }
        } catch (...) {
        }
    }
    return fallback;
}

} // namespace

void DataManager::registerAPI(QuickJSContext& ctx) {
    std::vector<QuickJSContext::MethodDef> methods;
    
    methods.push_back({"loadDatabase", [](const std::vector<Value>&) -> Value {
        DataManager::instance().loadDatabase();
        return Value::Int(1);
    }, CompatStatus::FULL});
    
    methods.push_back({"saveGame", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Int(0);
        return Value::Int(DataManager::instance().saveGame(static_cast<int32_t>(valueToInt64(args[0]))) ? 1 : 0);
    }, CompatStatus::FULL});
    
    methods.push_back({"loadGame", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Int(0);
        return Value::Int(DataManager::instance().loadGame(static_cast<int32_t>(valueToInt64(args[0]))) ? 1 : 0);
    }, CompatStatus::FULL});
    
    methods.push_back({"getGold", [](const std::vector<Value>&) -> Value {
        return Value::Int(DataManager::instance().getGold());
    }, CompatStatus::FULL});
    
    methods.push_back({"setGold", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Nil();
        DataManager::instance().setGold(static_cast<int32_t>(valueToInt64(args[0])));
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"getSwitch", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Int(0);
        return Value::Int(DataManager::instance().getSwitch(static_cast<int32_t>(valueToInt64(args[0]))) ? 1 : 0);
    }, CompatStatus::FULL});
    
    methods.push_back({"setSwitch", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 2) return Value::Nil();
        DataManager::instance().setSwitch(static_cast<int32_t>(valueToInt64(args[0])), valueToInt64(args[1]) != 0);
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"getVariable", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Int(0);
        return Value::Int(DataManager::instance().getVariable(static_cast<int32_t>(valueToInt64(args[0]))));
    }, CompatStatus::FULL});
    
    methods.push_back({"setVariable", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 2) return Value::Nil();
        DataManager::instance().setVariable(static_cast<int32_t>(valueToInt64(args[0])), static_cast<int32_t>(valueToInt64(args[1])));
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"getItemCount", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Int(0);
        return Value::Int(DataManager::instance().getItemCount(static_cast<int32_t>(valueToInt64(args[0]))));
    }, CompatStatus::FULL});
    
    methods.push_back({"gainItem", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 2) return Value::Nil();
        DataManager::instance().gainItem(static_cast<int32_t>(valueToInt64(args[0])), static_cast<int32_t>(valueToInt64(args[1])));
        return Value::Nil();
    }, CompatStatus::FULL});
    
    ctx.registerObject("DataManager", methods);
    
    // Register $gameParty and $gameTroop runtime objects
    GameParty::registerAPI(ctx, DataManager::instance().getGameParty());
    GameTroop::registerAPI(ctx, DataManager::instance().getGameTroop());
}

} // namespace compat
} // namespace urpg
