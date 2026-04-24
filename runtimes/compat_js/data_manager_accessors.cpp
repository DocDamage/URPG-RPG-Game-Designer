#include "runtimes/compat_js/data_manager.h"
#include "runtimes/compat_js/data_manager_support.h"

#include <algorithm>
#include <optional>

namespace urpg {
namespace compat {
const TilesetData* DataManager::getTileset(int32_t id) const {
    for (const auto& ts : tilesets_) {
        if (ts.id == id) return &ts;
    }
    return nullptr;
}

Value DataManager::getTilesetsAsValue() const {
    Array arr;
    for (const auto& tileset : tilesets_) {
        if (tileset.id <= 0) {
            continue;
        }
        arr.push_back(Value::Obj(tilesetToObject(tileset)));
    }
    return Value::Arr(std::move(arr));
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

ActorData* DataManager::getActor(int32_t id) {
    return const_cast<ActorData*>(static_cast<const DataManager*>(this)->getActor(id));
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

ClassData* DataManager::getClass(int32_t id) {
    return const_cast<ClassData*>(static_cast<const DataManager*>(this)->getClass(id));
}

const SkillData* DataManager::getSkill(int32_t id) const {
    if (id <= 0 || static_cast<size_t>(id) > skills_.size()) {
        return nullptr;
    }
    return &skills_[static_cast<size_t>(id - 1)];
}

SkillData* DataManager::getSkill(int32_t id) {
    return const_cast<SkillData*>(static_cast<const DataManager*>(this)->getSkill(id));
}

const ItemData* DataManager::getItem(int32_t id) const {
    if (id <= 0 || static_cast<size_t>(id) > items_.size()) {
        return nullptr;
    }
    return &items_[static_cast<size_t>(id - 1)];
}

ItemData* DataManager::getItem(int32_t id) {
    return const_cast<ItemData*>(static_cast<const DataManager*>(this)->getItem(id));
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

} // namespace compat
} // namespace urpg
