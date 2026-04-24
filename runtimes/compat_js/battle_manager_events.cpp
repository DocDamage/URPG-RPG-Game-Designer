#include "runtimes/compat_js/battle_manager.h"

#include "runtimes/compat_js/battle_manager_internal_state.h"
#include "runtimes/compat_js/battle_manager_support.h"
#include "runtimes/compat_js/data_manager.h"

#include <algorithm>
#include <vector>

namespace urpg {
namespace compat {
void BattleManager::startBattleEvent(int32_t eventId) {
    const TroopData* troop = DataManager::instance().getTroop(troopId_);
    if (!troop) {
        impl_->currentBattleEventId = 0;
        impl_->battleEventActive = false;
        return;
    }

    const auto* pages = asArray(troop->pages);
    if (!pages || eventId <= 0 || static_cast<size_t>(eventId) > pages->size()) {
        impl_->currentBattleEventId = 0;
        impl_->battleEventActive = false;
        return;
    }

    impl_->currentBattleEventId = eventId;
    impl_->battleEventActive = true;
}

void BattleManager::updateBattleEvents() {
    ++impl_->battleEventTick;

    for (auto& animation : activeAnimations_) {
        if (animation.framesRemaining > 0) {
            --animation.framesRemaining;
        }
    }

    activeAnimations_.erase(
        std::remove_if(activeAnimations_.begin(), activeAnimations_.end(),
                       [](const BattleAnimationPlayback& animation) {
                           return animation.framesRemaining <= 0;
                       }),
        activeAnimations_.end());

    const TroopData* troop = DataManager::instance().getTroop(troopId_);
    const auto* pages = troop ? asArray(troop->pages) : nullptr;
    if (!pages || pages->empty()) {
        impl_->battleEventActive = false;
        impl_->currentBattleEventId = 0;
        return;
    }

    if (impl_->pageStates.size() != pages->size()) {
        impl_->pageStates.resize(pages->size());
    }

    const auto findActorIndexById = [this](int32_t actorId) -> int32_t {
        for (size_t i = 0; i < actors_.size(); ++i) {
            if (actors_[i].id == actorId) {
                return static_cast<int32_t>(i);
            }
        }
        return -1;
    };

    const auto isPageConditionMet = [this, &findActorIndexById](const Object& pageObject) -> bool {
        const auto conditionsIt = pageObject.find("conditions");
        if (conditionsIt == pageObject.end()) {
            return true;
        }

        const auto* conditions = asObject(conditionsIt->second);
        if (!conditions) {
            return true;
        }

        if (valueToBool(conditions->at("turnValid"), false)) {
            const int32_t turnA = valueToInt(conditions->at("turnA"), 0);
            const int32_t turnB = valueToInt(conditions->at("turnB"), 0);
            if (!checkTurnCondition(turnA, turnB)) {
                return false;
            }
        }

        if (valueToBool(conditions->at("switchValid"), false)) {
            if (!checkSwitchCondition(valueToInt(conditions->at("switchId"), 0))) {
                return false;
            }
        }

        if (valueToBool(conditions->at("enemyValid"), false)) {
            if (!checkEnemyHpCondition(valueToInt(conditions->at("enemyIndex"), 0),
                                       valueToInt(conditions->at("enemyHp"), 100))) {
                return false;
            }
        }

        if (valueToBool(conditions->at("actorValid"), false)) {
            const int32_t actorIndex = findActorIndexById(valueToInt(conditions->at("actorId"), 0));
            if (actorIndex < 0 ||
                !checkActorHpCondition(actorIndex, valueToInt(conditions->at("actorHp"), 100))) {
                return false;
            }
        }

        if (valueToBool(conditions->at("turnEnding"), false) && phase_ != BattlePhase::TURN) {
            return false;
        }

        return true;
    };

    if (!impl_->battleEventActive) {
        for (size_t pageIndex = 0; pageIndex < pages->size(); ++pageIndex) {
            const auto* pageObject = asObject((*pages)[pageIndex]);
            if (!pageObject || !isPageConditionMet(*pageObject)) {
                continue;
            }

            const int32_t span = valueToInt(pageObject->at("span"), 0);
            auto& pageState = impl_->pageStates[pageIndex];
            bool shouldTrigger = false;
            if (span == 0) {
                shouldTrigger = !pageState.ranThisBattle;
            } else if (span == 1) {
                shouldTrigger = pageState.lastTriggeredTurn != turnCount_;
            } else {
                shouldTrigger = pageState.lastTriggeredTick != impl_->battleEventTick;
            }

            if (shouldTrigger) {
                startBattleEvent(static_cast<int32_t>(pageIndex + 1));
                pageState.ranThisBattle = true;
                pageState.lastTriggeredTurn = turnCount_;
                pageState.lastTriggeredTick = impl_->battleEventTick;
                break;
            }
        }
    }

    if (!impl_->battleEventActive) {
        return;
    }

    const size_t activePageIndex = static_cast<size_t>(impl_->currentBattleEventId - 1);
    if (activePageIndex >= pages->size()) {
        impl_->battleEventActive = false;
        impl_->currentBattleEventId = 0;
        return;
    }

    const auto* pageObject = asObject((*pages)[activePageIndex]);
    if (!pageObject) {
        impl_->battleEventActive = false;
        impl_->currentBattleEventId = 0;
        return;
    }

    const auto listIt = pageObject->find("list");
    const auto* commands = (listIt != pageObject->end()) ? asArray(listIt->second) : nullptr;
    if (!commands) {
        impl_->battleEventActive = false;
        impl_->currentBattleEventId = 0;
        return;
    }

    std::vector<bool> branchStack;
    const auto isParentBranchActive = [&branchStack]() -> bool {
        return std::all_of(branchStack.begin(), branchStack.end(), [](bool active) { return active; });
    };
    const auto evaluateConditional = [this](const Array& params) -> bool {
        if (params.empty()) {
            return false;
        }

        const int32_t mode = valueToInt(params[0], -1);
        if (mode == 0 && params.size() >= 3) {
            const int32_t switchId = valueToInt(params[1], 0);
            const bool expected = valueToInt(params[2], 0) == 0;
            return DataManager::instance().getSwitch(switchId) == expected;
        }

        if (mode == 12 && params.size() >= 2) {
            return valueToString(params[1]) == "BattleManager.isBattleTest()" && isBattleTest();
        }

        if (mode == 1 && params.size() >= 5) {
            const int32_t left = DataManager::instance().getVariable(valueToInt(params[1], 0));
            const int32_t right = valueToInt(params[2], 0) == 0
                ? valueToInt(params[3], 0)
                : DataManager::instance().getVariable(valueToInt(params[3], 0));
            switch (valueToInt(params[4], -1)) {
                case 0: return left == right;
                case 1: return left >= right;
                case 2: return left <= right;
                case 3: return left > right;
                case 4: return left < right;
                case 5: return left != right;
                default: return false;
            }
        }

        return false;
    };

    for (const auto& commandValue : *commands) {
        const auto* command = asObject(commandValue);
        if (!command) {
            continue;
        }

        const int32_t code = valueToInt(command->at("code"), 0);
        const auto paramsIt = command->find("parameters");
        const Array emptyParams;
        const Array* params = (paramsIt != command->end()) ? asArray(paramsIt->second) : &emptyParams;

        if (code == 111) {
            const bool parentActive = isParentBranchActive();
            const bool conditionMet = parentActive && params && evaluateConditional(*params);
            branchStack.push_back(conditionMet);
            continue;
        }

        if (code == 411) {
            if (!branchStack.empty()) {
                const bool previous = branchStack.back();
                branchStack.pop_back();
                const bool parentActive = isParentBranchActive();
                branchStack.push_back(parentActive && !previous);
            }
            continue;
        }

        if (code == 412) {
            if (!branchStack.empty()) {
                branchStack.pop_back();
            }
            continue;
        }

        if (!isParentBranchActive()) {
            continue;
        }

        switch (code) {
            case 0:
            case 108:
            case 408:
                break;
            case 121:
                if (params && params->size() >= 3) {
                    const int32_t startId = valueToInt((*params)[0], 0);
                    const int32_t endId = valueToInt((*params)[1], startId);
                    const bool value = valueToInt((*params)[2], 0) == 0;
                    for (int32_t switchId = startId; switchId <= endId; ++switchId) {
                        DataManager::instance().setSwitch(switchId, value);
                    }
                }
                break;
            case 122:
                if (params && params->size() >= 5) {
                    const int32_t startId = valueToInt((*params)[0], 0);
                    const int32_t endId = valueToInt((*params)[1], startId);
                    const int32_t operation = valueToInt((*params)[2], 0);
                    const int32_t operandType = valueToInt((*params)[3], 0);
                    const int32_t operand = (operandType == 0) ? valueToInt((*params)[4], 0) : 0;
                    for (int32_t variableId = startId; variableId <= endId; ++variableId) {
                        int32_t current = DataManager::instance().getVariable(variableId);
                        switch (operation) {
                            case 0: current = operand; break;
                            case 1: current += operand; break;
                            case 2: current -= operand; break;
                            case 3: current *= operand; break;
                            case 4: if (operand != 0) current /= operand; break;
                            case 5: if (operand != 0) current %= operand; break;
                            default: break;
                        }
                        DataManager::instance().setVariable(variableId, current);
                    }
                }
                break;
            case 125:
                if (params && params->size() >= 3) {
                    const int32_t operation = valueToInt((*params)[0], 0);
                    const int32_t operandType = valueToInt((*params)[1], 0);
                    const int32_t amount = (operandType == 0) ? valueToInt((*params)[2], 0) : 0;
                    if (operation == 0) {
                        DataManager::instance().gainGold(amount);
                    } else {
                        DataManager::instance().loseGold(amount);
                    }
                }
                break;
            default:
                break;
        }
    }

    impl_->battleEventActive = false;
    impl_->currentBattleEventId = 0;
}

bool BattleManager::isBattleEventActive() const {
    return impl_->battleEventActive;
}

} // namespace compat
} // namespace urpg
