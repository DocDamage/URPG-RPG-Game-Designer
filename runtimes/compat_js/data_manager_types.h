#pragma once

#include "engine/runtimes/bridge/value.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace urpg {
namespace compat {

struct SaveHeader {
    int32_t version = 0;
    std::string timestamp;
    int32_t playtimeFrames = 0;
    int32_t mapId = 0;
    int32_t playerId = 0;
    int32_t playerX = 0;
    int32_t playerY = 0;
    std::string partyNames;
    std::string mapDisplayName;
    bool isAutosave = false;
};

struct GlobalState {
    std::vector<Value> actors;
    std::vector<int32_t> partyMembers;
    int32_t gold = 0;
    int32_t steps = 0;
    int32_t playtime = 0;

    int32_t mapId = 0;
    int32_t playerX = 0;
    int32_t playerY = 0;
    int32_t playerDirection = 2;

    std::vector<bool> switches;
    std::vector<int32_t> variables;
    std::unordered_map<std::string, bool> selfSwitches;

    std::unordered_map<int32_t, int32_t> items;
    std::unordered_map<int32_t, int32_t> weapons;
    std::unordered_map<int32_t, int32_t> armors;
};

struct ActorData {
    int32_t id = 0;
    std::string name;
    std::string nickname;
    int32_t classId = 0;
    int32_t initialLevel = 1;
    int32_t maxLevel = 99;
    int32_t level = 1;
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
    int32_t typeId = 0;
    int32_t occasion = 0;
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
    std::string battlerName;
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
    std::vector<int32_t> dropItems;
};

struct TroopData {
    int32_t id = 0;
    std::string name;
    std::vector<int32_t> members;
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
    std::vector<std::vector<int32_t>> data;
    Value events;
};

struct ClassData {
    int32_t id = 0;
    std::string name;
    int32_t maxLevel = 99;
    std::vector<int32_t> learnings;
    std::vector<int32_t> expTable;
    std::vector<std::pair<int32_t, int32_t>> skillsToLearn;
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

} // namespace compat
} // namespace urpg
