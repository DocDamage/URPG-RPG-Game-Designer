#include "runtimes/compat_js/data_manager.h"

#include "runtimes/compat_js/data_manager_support.h"

#include <utility>

namespace urpg {
namespace compat {
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
        obj["exp"] = Value::Int(actor.exp);
        Array skillsArr;
        for (int32_t sid : actor.skills) {
            skillsArr.push_back(Value::Int(sid));
        }
        obj["skills"] = Value::Arr(std::move(skillsArr));
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

Value DataManager::getEnemiesAsValue() const {
    Array arr;
    for (const auto& enemy : enemies_) {
        Object obj;
        obj["id"] = Value::Int(enemy.id);
        Value name;
        name.v = enemy.name;
        obj["name"] = std::move(name);
        Value battlerName;
        battlerName.v = enemy.battlerName;
        obj["battlerName"] = std::move(battlerName);
        obj["mhp"] = Value::Int(enemy.mhp);
        obj["mmp"] = Value::Int(enemy.mmp);
        obj["atk"] = Value::Int(enemy.atk);
        obj["def"] = Value::Int(enemy.def);
        obj["mat"] = Value::Int(enemy.mat);
        obj["mdf"] = Value::Int(enemy.mdf);
        obj["agi"] = Value::Int(enemy.agi);
        obj["luk"] = Value::Int(enemy.luk);
        obj["exp"] = Value::Int(enemy.exp);
        obj["gold"] = Value::Int(enemy.gold);
        arr.push_back(Value::Obj(std::move(obj)));
    }
    return Value::Arr(std::move(arr));
}
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

Value DataManager::getMapInfosAsValue() const {
    Array arr;
    for (const auto& info : mapInfos_) {
        if (info.id <= 0) {
            continue;
        }
        arr.push_back(Value::Obj(mapInfoToObject(info)));
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
    return Value::Obj(std::move(obj));
}

// ============================================================================
// Event/Plugin Integration
// ============================================================================

} // namespace compat
} // namespace urpg
