#include "runtimes/compat_js/data_manager.h"

#include "runtimes/compat_js/data_manager_support.h"

#include <algorithm>
#include <utility>
#include <vector>

namespace urpg {
namespace compat {
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
            if (elem.contains("damage") && elem["damage"].is_object()) {
                auto& d = elem["damage"];
                skill.damage.type = d.value("type", 0);
                skill.damage.elementId = d.value("elementId", 0);
                skill.damage.formula = d.value("formula", "");
                skill.damage.variance = d.value("variance", 20);
                skill.damage.canCrit = d.value("critical", false);
            }
            if (elem.contains("effects") && elem["effects"].is_array()) {
                for (const auto& e : elem["effects"]) {
                    if (e.is_object()) {
                        EffectData eff;
                        eff.code = e.value("code", 0);
                        eff.dataId = e.value("dataId", 0);
                        eff.value1 = e.value("value1", 0.0);
                        eff.value2 = e.value("value2", 0.0);
                        skill.effects.push_back(eff);
                    }
                }
            }
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
    heal.damage.type = 3;
    heal.damage.power = 30;
    skills_.push_back(std::move(heal));

    SkillData fire;
    fire.id = 2;
    fire.name = "Fire";
    fire.description = "Deals fire damage to one enemy.";
    fire.typeId = 1;
    fire.scope = 1;
    fire.mpCost = 5;
    fire.tpCost = 0;
    fire.speed = 0;
    fire.successRate = 100;
    fire.repeats = 1;
    fire.animationId = 67;
    fire.damage.type = 1;
    fire.damage.power = 25;
    skills_.push_back(std::move(fire));
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
            if (elem.contains("damage") && elem["damage"].is_object()) {
                auto& d = elem["damage"];
                item.damage.type = d.value("type", 0);
                item.damage.elementId = d.value("elementId", 0);
                item.damage.formula = d.value("formula", "");
                item.damage.variance = d.value("variance", 20);
                item.damage.canCrit = d.value("critical", false);
            }
            if (elem.contains("effects") && elem["effects"].is_array()) {
                for (const auto& e : elem["effects"]) {
                    if (e.is_object()) {
                        EffectData eff;
                        eff.code = e.value("code", 0);
                        eff.dataId = e.value("dataId", 0);
                        eff.value1 = e.value("value1", 0.0);
                        eff.value2 = e.value("value2", 0.0);
                        item.effects.push_back(eff);
                    }
                }
            }
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
    EffectData potionEff;
    potionEff.code = 11;
    potionEff.value2 = 200.0;
    potion.effects.push_back(potionEff);
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

} // namespace compat
} // namespace urpg
