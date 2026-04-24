#include "runtimes/compat_js/battle_manager.h"

#include "runtimes/compat_js/battle_manager_internal_state.h"
#include "runtimes/compat_js/battle_manager_support.h"

#include <algorithm>
#include <utility>

namespace urpg {
namespace compat {
std::vector<BattleSubject*> BattleManager::getActors() {
    std::vector<BattleSubject*> result;
    for (auto& actor : actors_) {
        result.push_back(&actor);
    }
    return result;
}

const std::vector<BattleSubject>& BattleManager::getActorsConst() const {
    return actors_;
}

std::vector<BattleSubject*> BattleManager::getEnemies() {
    std::vector<BattleSubject*> result;
    for (auto& enemy : enemies_) {
        result.push_back(&enemy);
    }
    return result;
}

const std::vector<BattleSubject>& BattleManager::getEnemiesConst() const {
    return enemies_;
}

BattleSubject* BattleManager::getActor(int32_t index) {
    if (index < 0 || static_cast<size_t>(index) >= actors_.size()) {
        return nullptr;
    }
    return &actors_[static_cast<size_t>(index)];
}

BattleSubject* BattleManager::getEnemy(int32_t index) {
    if (index < 0 || static_cast<size_t>(index) >= enemies_.size()) {
        return nullptr;
    }
    return &enemies_[static_cast<size_t>(index)];
}

void BattleManager::addActorSubject(const BattleSubject& subject) {
    if (!impl_->manualActorsOverride) {
        actors_.clear();
        impl_->manualActorsOverride = true;
    }
    actors_.push_back(subject);
}

void BattleManager::addEnemySubject(const BattleSubject& subject) {
    if (!impl_->manualEnemiesOverride) {
        enemies_.clear();
        impl_->manualEnemiesOverride = true;
    }
    enemies_.push_back(subject);
}

std::vector<BattleSubject*> BattleManager::getAllSubjects() {
    std::vector<BattleSubject*> result;
    for (auto& actor : actors_) {
        result.push_back(&actor);
    }
    for (auto& enemy : enemies_) {
        result.push_back(&enemy);
    }
    return result;
}

std::vector<BattleSubject*> BattleManager::getActiveActors() {
    std::vector<BattleSubject*> result;
    for (auto& actor : actors_) {
        if (!actor.hidden && actor.hp > 0) {
            result.push_back(&actor);
        }
    }
    return result;
}

std::vector<BattleSubject*> BattleManager::getActiveEnemies() {
    std::vector<BattleSubject*> result;
    for (auto& enemy : enemies_) {
        if (!enemy.hidden && enemy.hp > 0) {
            result.push_back(&enemy);
        }
    }
    return result;
}

bool BattleManager::isAllActorsDead() const {
    for (const auto& actor : actors_) {
        if (!actor.hidden && actor.hp > 0) {
            return false;
        }
    }
    return !actors_.empty();
}

bool BattleManager::isAllEnemiesDead() const {
    for (const auto& enemy : enemies_) {
        if (!enemy.hidden && enemy.hp > 0) {
            return false;
        }
    }
    return !enemies_.empty();
}

// ============================================================================
// Action System
// ============================================================================

void BattleManager::queueAction(BattleSubject* subject, BattleActionType type,
                                 int32_t targetIndex, int32_t skillId, int32_t itemId) {
    if (!subject) return;

    BattleAction action;
    action.subject = subject;
    action.type = type;
    action.targetIndex = targetIndex;
    action.skillId = skillId;
    action.itemId = itemId;
    action.forced = false;

    actionQueue_.push_back(action);
    subject->acted = true;
}

void BattleManager::forceAction(int32_t subjectIndex, BattleSubjectType subjectType,
                                 BattleActionType type, int32_t targetIndex,
                                 int32_t skillId, int32_t itemId) {
    BattleSubject* subject = nullptr;
    if (subjectType == BattleSubjectType::ACTOR) {
        subject = getActor(subjectIndex);
    } else {
        subject = getEnemy(subjectIndex);
    }

    if (!subject) return;

    BattleAction action;
    action.subject = subject;
    action.type = type;
    action.targetIndex = targetIndex;
    action.skillId = skillId;
    action.itemId = itemId;
    action.forced = true;

    actionQueue_.push_back(action);
}

BattleAction* BattleManager::getNextAction() {
    if (actionQueue_.empty()) {
        return nullptr;
    }

    currentActionIndex_ = 0;
    return &actionQueue_[0];
}

void BattleManager::processAction(BattleAction* action) {
    if (!action || !action->subject) return;

    phase_ = BattlePhase::ACTION;
    triggerHook(HookPoint::ON_ACTION_START, {});

    // Process action based on type
    switch (action->type) {
        case BattleActionType::ATTACK: {
            BattleSubject* target = resolveActionTarget(this, *action);
            if (target) {
                applyDamage(target, resolveAttackDamage(action->subject, target, 10), true);
            }
            break;
        }
        case BattleActionType::GUARD:
            // Set guard state
            break;
        case BattleActionType::SKILL: {
            BattleSubject* target = resolveActionTarget(this, *action);
            if (target) {
                applySkill(action->subject, target, action->skillId);
            }
            break;
        }
        case BattleActionType::ITEM: {
            BattleSubject* target = resolveActionTarget(this, *action);
            if (target) {
                applyItem(action->subject, target, action->itemId);
            }
            break;
        }
        case BattleActionType::ESCAPE:
            processEscape();
            break;
        case BattleActionType::WAIT:
            break;
    }

    triggerHook(HookPoint::ON_ACTION_END, {});

    // Remove processed action
    if (!actionQueue_.empty()) {
        actionQueue_.erase(actionQueue_.begin());
    }
    currentActionIndex_ = -1;
}

void BattleManager::clearActions() {
    actionQueue_.clear();
    currentActionIndex_ = -1;
}

void BattleManager::sortActionsBySpeed() {
    std::sort(actionQueue_.begin(), actionQueue_.end(),
              [](const BattleAction& a, const BattleAction& b) {
                  if (!a.subject || !b.subject) return false;
                  return a.subject->actionSpeed > b.subject->actionSpeed;
              });
}

// ============================================================================
// Input Phase
// ============================================================================

BattleSubject* BattleManager::selectNextActor() {
    for (auto& actor : actors_) {
        if (!actor.hidden && actor.hp > 0 && !actor.acted) {
            return &actor;
        }
    }
    return nullptr;
}

bool BattleManager::isAllActorsInputted() const {
    for (const auto& actor : actors_) {
        if (!actor.hidden && actor.hp > 0 && !actor.acted) {
            return false;
        }
    }
    return true;
}

void BattleManager::setActorAction(int32_t actorIndex, BattleActionType type,
                                    int32_t targetIndex, int32_t skillId, int32_t itemId) {
    BattleSubject* actor = getActor(actorIndex);
    if (actor) {
        queueAction(actor, type, targetIndex, skillId, itemId);
    }
}

void BattleManager::autoBattleActor(int32_t actorIndex) {
    BattleSubject* actor = getActor(actorIndex);
    if (!actor || actor->acted) return;

    auto enemies = getActiveEnemies();
    if (!enemies.empty()) {
        const auto bestTarget = *std::max_element(enemies.begin(), enemies.end(),
                                                  [](const BattleSubject* lhs, const BattleSubject* rhs) {
                                                      return targetPriorityScore(lhs) < targetPriorityScore(rhs);
                                                  });
        int32_t targetIndex = bestTarget->index;
        setActorAction(actorIndex, BattleActionType::ATTACK, targetIndex);
    }
}

// ============================================================================
// Damage and Effects
// ============================================================================

} // namespace compat
} // namespace urpg
