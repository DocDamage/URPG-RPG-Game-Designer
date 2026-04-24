#include "runtimes/compat_js/data_manager.h"

#include "runtimes/compat_js/data_manager_internal_state.h"
#include "runtimes/compat_js/data_manager_support.h"

#include <algorithm>
#include <utility>
#include <vector>

namespace urpg {
namespace compat {

namespace {
using json = nlohmann::json;
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
    hero.exp = 0;
    hero.faceName = "Actor1";
    hero.faceIndex = 0;
    hero.characterName = "Actor1";
    hero.characterIndex = 0;
    hero.battlerName = "Actor1_1";
    hero.battlerIndex = 0;
    actors_.push_back(std::move(hero));
    hydrateActorParamsFromClasses(actors_, classes_);
    seedActorResourceState(actors_);
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
            cls.maxLevel = elem.value("maxLevel", 99);
            if (elem.contains("learnings") && elem["learnings"].is_array()) {
                for (const auto& ln : elem["learnings"]) {
                    if (ln.is_object()) {
                        int32_t level = ln.value("level", 1);
                        int32_t skillId = 0;
                        if (ln.contains("skillId")) {
                            if (ln["skillId"].is_number()) skillId = ln["skillId"].get<int32_t>();
                        }
                        cls.skillsToLearn.push_back({level, skillId});
                        if (skillId > 0) {
                            cls.learnings.push_back(skillId);
                        }
                    } else if (ln.contains("skillId")) {
                        cls.learnings.push_back(ln["skillId"].get<int32_t>());
                    }
                }
            }
            if (elem.contains("expParams") && elem["expParams"].is_array() && elem["expParams"].size() >= 4) {
                int32_t base = elem["expParams"][0].get<int32_t>();
                int32_t extra = elem["expParams"][1].get<int32_t>();
                int32_t accA = elem["expParams"][2].get<int32_t>();
                int32_t accB = elem["expParams"][3].get<int32_t>();
                for (int32_t lvl = 1; lvl < cls.maxLevel; ++lvl) {
                    int32_t nextExp = base + (extra * lvl) + (accA * lvl * lvl / 100) + (accB * lvl * lvl * lvl / 10000);
                    if (nextExp < 1) nextExp = 1;
                    cls.expTable.push_back(nextExp);
                }
            } else {
                for (int32_t lvl = 1; lvl < cls.maxLevel; ++lvl) {
                    cls.expTable.push_back(50 + lvl * 15);
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
        seedActorResourceState(actors_);
        return true;
    }
    ClassData warrior;
    warrior.id = 1;
    warrior.name = "Warrior";
    warrior.maxLevel = 99;
    for (int32_t lvl = 1; lvl < warrior.maxLevel; ++lvl) {
        warrior.expTable.push_back(50 + lvl * 15);
    }
    classes_.push_back(std::move(warrior));
    hydrateActorParamsFromClasses(actors_, classes_);
    seedActorResourceState(actors_);
    return true;
}

} // namespace compat
} // namespace urpg
