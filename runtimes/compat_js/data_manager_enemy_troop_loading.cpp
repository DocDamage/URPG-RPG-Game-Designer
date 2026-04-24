#include "runtimes/compat_js/data_manager.h"

#include "runtimes/compat_js/data_manager_support.h"

#include <utility>
#include <vector>

namespace urpg {
namespace compat {
bool DataManager::loadEnemies() {
    enemies_.clear();
    if (auto j = loadJsonArrayFile("Enemies.json")) {
        for (const auto& elem : *j) {
            if (elem.is_null()) continue;
            EnemyData enemy;
            enemy.id = elem.value("id", 0);
            enemy.name = elem.value("name", "");
            enemy.battlerName = elem.value("battlerName", "");
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
    slime.battlerName = "Monster";
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
    goblin.battlerName = "Monster";
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
} // namespace compat
} // namespace urpg
