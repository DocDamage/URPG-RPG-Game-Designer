#include "runtimes/compat_js/battle_manager.h"

#include "runtimes/compat_js/battle_manager_support.h"

#include <utility>
#include <vector>

namespace urpg {
namespace compat {
CompatStatus BattleManager::getMethodStatus(const std::string& methodName) {
    auto it = methodStatus_.find(methodName);
    if (it != methodStatus_.end()) {
        return it->second;
    }
    return CompatStatus::UNSUPPORTED;
}

std::string BattleManager::getMethodDeviation(const std::string& methodName) {
    auto it = methodDeviations_.find(methodName);
    if (it != methodDeviations_.end()) {
        return it->second;
    }
    return "";
}

void BattleManager::registerAPI(QuickJSContext& ctx) {
    std::vector<QuickJSContext::MethodDef> methods;

    methods.push_back(makeMethodDef("setup", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Nil();
        int32_t troopId = 0;
        if (std::holds_alternative<int64_t>(args[0].v)) troopId = static_cast<int32_t>(std::get<int64_t>(args[0].v));
        bool canEscape = true;
        bool canLose = false;
        if (args.size() > 1 && std::holds_alternative<int64_t>(args[1].v)) canEscape = std::get<int64_t>(args[1].v) != 0;
        if (args.size() > 2 && std::holds_alternative<int64_t>(args[2].v)) canLose = std::get<int64_t>(args[2].v) != 0;
        BattleManager::instance().setup(troopId, canEscape, canLose);
        return Value::Nil();
    }, CompatStatus::PARTIAL));

    methods.push_back(makeMethodDef("startBattle", [](const std::vector<Value>&) -> Value {
        BattleManager::instance().startBattle();
        return Value::Nil();
    }, CompatStatus::FULL));

    methods.push_back(makeMethodDef("abortBattle", [](const std::vector<Value>&) -> Value {
        BattleManager::instance().abortBattle();
        return Value::Nil();
    }, CompatStatus::FULL));

    methods.push_back(makeMethodDef("endBattle", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Nil();
        int32_t result = 0;
        if (std::holds_alternative<int64_t>(args[0].v)) result = static_cast<int32_t>(std::get<int64_t>(args[0].v));
        BattleManager::instance().endBattle(static_cast<BattleResult>(result));
        return Value::Nil();
    }, CompatStatus::FULL));

    methods.push_back(makeMethodDef("isBattleTest", [](const std::vector<Value>&) -> Value {
        return Value::Int(BattleManager::instance().isBattleTest() ? 1 : 0);
    }, CompatStatus::FULL));

    methods.push_back(makeMethodDef("canEscape", [](const std::vector<Value>&) -> Value {
        return Value::Int(BattleManager::instance().canEscape() ? 1 : 0);
    }, CompatStatus::FULL));

    methods.push_back(makeMethodDef("canLose", [](const std::vector<Value>&) -> Value {
        return Value::Int(BattleManager::instance().canLose() ? 1 : 0);
    }, CompatStatus::FULL));

    methods.push_back(makeMethodDef("onEscapeSuccess", [](const std::vector<Value>&) -> Value {
        BattleManager::instance().onEscapeSuccess();
        return Value::Nil();
    }, CompatStatus::FULL));

    methods.push_back(makeMethodDef("onEscapeFailure", [](const std::vector<Value>&) -> Value {
        BattleManager::instance().onEscapeFailure();
        return Value::Nil();
    }, CompatStatus::FULL));

    methods.push_back(makeMethodDef("changeBattleBackground", [](const std::vector<Value>& args) -> Value {
        if (!args.empty() && std::holds_alternative<std::string>(args[0].v)) {
            BattleManager::instance().changeBattleBackground(std::get<std::string>(args[0].v));
        }
        return Value::Nil();
    }, CompatStatus::PARTIAL));

    methods.push_back(makeMethodDef("changeBattleBgm", [](const std::vector<Value>& args) -> Value {
        std::string name;
        double volume = 90.0;
        double pitch = 100.0;
        if (!args.empty() && std::holds_alternative<std::string>(args[0].v)) name = std::get<std::string>(args[0].v);
        if (args.size() > 1 && std::holds_alternative<double>(args[1].v)) volume = std::get<double>(args[1].v);
        else if (args.size() > 1 && std::holds_alternative<int64_t>(args[1].v)) volume = static_cast<double>(std::get<int64_t>(args[1].v));
        if (args.size() > 2 && std::holds_alternative<double>(args[2].v)) pitch = std::get<double>(args[2].v);
        else if (args.size() > 2 && std::holds_alternative<int64_t>(args[2].v)) pitch = static_cast<double>(std::get<int64_t>(args[2].v));
        BattleManager::instance().changeBattleBgm(name, volume, pitch);
        return Value::Nil();
    }, CompatStatus::PARTIAL));

    methods.push_back(makeMethodDef("changeVictoryMe", [](const std::vector<Value>& args) -> Value {
        std::string name;
        double volume = 90.0;
        double pitch = 100.0;
        if (!args.empty() && std::holds_alternative<std::string>(args[0].v)) name = std::get<std::string>(args[0].v);
        if (args.size() > 1 && std::holds_alternative<double>(args[1].v)) volume = std::get<double>(args[1].v);
        else if (args.size() > 1 && std::holds_alternative<int64_t>(args[1].v)) volume = static_cast<double>(std::get<int64_t>(args[1].v));
        if (args.size() > 2 && std::holds_alternative<double>(args[2].v)) pitch = std::get<double>(args[2].v);
        else if (args.size() > 2 && std::holds_alternative<int64_t>(args[2].v)) pitch = static_cast<double>(std::get<int64_t>(args[2].v));
        BattleManager::instance().changeVictoryMe(name, volume, pitch);
        return Value::Nil();
    }, CompatStatus::PARTIAL));

    methods.push_back(makeMethodDef("changeDefeatMe", [](const std::vector<Value>& args) -> Value {
        std::string name;
        double volume = 90.0;
        double pitch = 100.0;
        if (!args.empty() && std::holds_alternative<std::string>(args[0].v)) name = std::get<std::string>(args[0].v);
        if (args.size() > 1 && std::holds_alternative<double>(args[1].v)) volume = std::get<double>(args[1].v);
        else if (args.size() > 1 && std::holds_alternative<int64_t>(args[1].v)) volume = static_cast<double>(std::get<int64_t>(args[1].v));
        if (args.size() > 2 && std::holds_alternative<double>(args[2].v)) pitch = std::get<double>(args[2].v);
        else if (args.size() > 2 && std::holds_alternative<int64_t>(args[2].v)) pitch = static_cast<double>(std::get<int64_t>(args[2].v));
        BattleManager::instance().changeDefeatMe(name, volume, pitch);
        return Value::Nil();
    }, CompatStatus::PARTIAL));

    methods.push_back(makeMethodDef("getBattleTransition", [](const std::vector<Value>&) -> Value {
        return Value::Int(BattleManager::instance().getBattleTransition());
    }, CompatStatus::PARTIAL));

    methods.push_back(makeMethodDef("getBattleBackground", [](const std::vector<Value>&) -> Value {
        Value background;
        background.v = BattleManager::instance().getBattleBackground();
        return background;
    }, CompatStatus::PARTIAL));

    methods.push_back(makeMethodDef("getBattleBgm", [](const std::vector<Value>&) -> Value {
        return battleAudioCueToValue(BattleManager::instance().getBattleBgm());
    }, CompatStatus::PARTIAL));

    methods.push_back(makeMethodDef("getVictoryMe", [](const std::vector<Value>&) -> Value {
        return battleAudioCueToValue(BattleManager::instance().getVictoryMe());
    }, CompatStatus::PARTIAL));

    methods.push_back(makeMethodDef("getDefeatMe", [](const std::vector<Value>&) -> Value {
        return battleAudioCueToValue(BattleManager::instance().getDefeatMe());
    }, CompatStatus::PARTIAL));

    methods.push_back(makeMethodDef("getPhase", [](const std::vector<Value>&) -> Value {
        return Value::Int(static_cast<int32_t>(BattleManager::instance().getPhase()));
    }, CompatStatus::FULL));

    methods.push_back(makeMethodDef("getTurnCount", [](const std::vector<Value>&) -> Value {
        return Value::Int(BattleManager::instance().getTurnCount());
    }, CompatStatus::FULL));

    methods.push_back(makeMethodDef("processEscape", [](const std::vector<Value>&) -> Value {
        return Value::Int(BattleManager::instance().processEscape() ? 1 : 0);
    }, CompatStatus::FULL));

    methods.push_back(makeMethodDef("processAction", [](const std::vector<Value>&) -> Value {
        BattleManager::instance().processAction();
        return Value::Nil();
    }, CompatStatus::FULL));

    methods.push_back(makeMethodDef("queueAction", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 2) return Value::Nil();
        int32_t subjectIndex = 0;
        int32_t subjectType = 0;
        int32_t actionType = 0;
        int32_t targetIndex = -1;
        int32_t skillId = 0;
        int32_t itemId = 0;
        if (std::holds_alternative<int64_t>(args[0].v)) subjectIndex = static_cast<int32_t>(std::get<int64_t>(args[0].v));
        if (std::holds_alternative<int64_t>(args[1].v)) subjectType = static_cast<int32_t>(std::get<int64_t>(args[1].v));
        if (args.size() > 2 && std::holds_alternative<int64_t>(args[2].v)) actionType = static_cast<int32_t>(std::get<int64_t>(args[2].v));
        if (args.size() > 3 && std::holds_alternative<int64_t>(args[3].v)) targetIndex = static_cast<int32_t>(std::get<int64_t>(args[3].v));
        if (args.size() > 4 && std::holds_alternative<int64_t>(args[4].v)) skillId = static_cast<int32_t>(std::get<int64_t>(args[4].v));
        if (args.size() > 5 && std::holds_alternative<int64_t>(args[5].v)) itemId = static_cast<int32_t>(std::get<int64_t>(args[5].v));
        BattleManager::instance().queueActionByIndices(subjectIndex, static_cast<BattleSubjectType>(subjectType), static_cast<BattleActionType>(actionType), targetIndex, skillId, itemId);
        return Value::Nil();
    }, CompatStatus::FULL));

    methods.push_back(makeMethodDef("getNextAction", [](const std::vector<Value>&) -> Value {
        BattleAction* action = BattleManager::instance().getNextAction();
        if (!action || !action->subject) return Value::Nil();
        Object obj;
        obj["subjectIndex"] = Value::Int(action->subject->index);
        obj["subjectType"] = Value::Int(static_cast<int32_t>(action->subject->type));
        obj["type"] = Value::Int(static_cast<int32_t>(action->type));
        obj["targetIndex"] = Value::Int(action->targetIndex);
        obj["skillId"] = Value::Int(action->skillId);
        obj["itemId"] = Value::Int(action->itemId);
        return Value::Obj(std::move(obj));
    }, CompatStatus::FULL));

    methods.push_back(makeMethodDef("clearActions", [](const std::vector<Value>&) -> Value {
        BattleManager::instance().clearActions();
        return Value::Nil();
    }, CompatStatus::FULL));

    methods.push_back(makeMethodDef("checkTurnCondition", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 2) return Value::Int(0);
        int32_t turn = 0;
        int32_t span = 0;
        if (std::holds_alternative<int64_t>(args[0].v)) turn = static_cast<int32_t>(std::get<int64_t>(args[0].v));
        if (std::holds_alternative<int64_t>(args[1].v)) span = static_cast<int32_t>(std::get<int64_t>(args[1].v));
        return Value::Int(BattleManager::instance().checkTurnCondition(turn, span) ? 1 : 0);
    }, CompatStatus::FULL));

    methods.push_back(makeMethodDef("forceAction", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 4) return Value::Nil();
        int32_t subjectIndex = 0;
        int32_t subjectType = 0;
        int32_t actionType = 0;
        int32_t targetIndex = -1;
        int32_t skillId = 0;
        int32_t itemId = 0;
        if (std::holds_alternative<int64_t>(args[0].v)) subjectIndex = static_cast<int32_t>(std::get<int64_t>(args[0].v));
        if (std::holds_alternative<int64_t>(args[1].v)) subjectType = static_cast<int32_t>(std::get<int64_t>(args[1].v));
        if (std::holds_alternative<int64_t>(args[2].v)) actionType = static_cast<int32_t>(std::get<int64_t>(args[2].v));
        if (std::holds_alternative<int64_t>(args[3].v)) targetIndex = static_cast<int32_t>(std::get<int64_t>(args[3].v));
        if (args.size() > 4 && std::holds_alternative<int64_t>(args[4].v)) skillId = static_cast<int32_t>(std::get<int64_t>(args[4].v));
        if (args.size() > 5 && std::holds_alternative<int64_t>(args[5].v)) itemId = static_cast<int32_t>(std::get<int64_t>(args[5].v));
        BattleManager::instance().forceAction(subjectIndex, static_cast<BattleSubjectType>(subjectType), static_cast<BattleActionType>(actionType), targetIndex, skillId, itemId);
        return Value::Nil();
    }, CompatStatus::FULL));

    methods.push_back(makeMethodDef("addState", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 3) return Value::Int(0);
        int32_t subjectIndex = 0;
        int32_t subjectType = 0;
        int32_t stateId = 0;
        if (std::holds_alternative<int64_t>(args[0].v)) subjectIndex = static_cast<int32_t>(std::get<int64_t>(args[0].v));
        if (std::holds_alternative<int64_t>(args[1].v)) subjectType = static_cast<int32_t>(std::get<int64_t>(args[1].v));
        if (std::holds_alternative<int64_t>(args[2].v)) stateId = static_cast<int32_t>(std::get<int64_t>(args[2].v));
        BattleSubject* subject = (subjectType == 0) ? BattleManager::instance().getActor(subjectIndex) : BattleManager::instance().getEnemy(subjectIndex);
        if (!subject) return Value::Int(0);
        return Value::Int(BattleManager::instance().addState(subject, stateId, 3) ? 1 : 0);
    }, CompatStatus::FULL));

    methods.push_back(makeMethodDef("removeState", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 3) return Value::Int(0);
        int32_t subjectIndex = 0;
        int32_t subjectType = 0;
        int32_t stateId = 0;
        if (std::holds_alternative<int64_t>(args[0].v)) subjectIndex = static_cast<int32_t>(std::get<int64_t>(args[0].v));
        if (std::holds_alternative<int64_t>(args[1].v)) subjectType = static_cast<int32_t>(std::get<int64_t>(args[1].v));
        if (std::holds_alternative<int64_t>(args[2].v)) stateId = static_cast<int32_t>(std::get<int64_t>(args[2].v));
        BattleSubject* subject = (subjectType == 0) ? BattleManager::instance().getActor(subjectIndex) : BattleManager::instance().getEnemy(subjectIndex);
        if (!subject) return Value::Int(0);
        return Value::Int(BattleManager::instance().removeState(subject, stateId) ? 1 : 0);
    }, CompatStatus::FULL));

    methods.push_back(makeMethodDef("isStateActive", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 3) return Value::Int(0);
        int32_t subjectIndex = 0;
        int32_t subjectType = 0;
        int32_t stateId = 0;
        if (std::holds_alternative<int64_t>(args[0].v)) subjectIndex = static_cast<int32_t>(std::get<int64_t>(args[0].v));
        if (std::holds_alternative<int64_t>(args[1].v)) subjectType = static_cast<int32_t>(std::get<int64_t>(args[1].v));
        if (std::holds_alternative<int64_t>(args[2].v)) stateId = static_cast<int32_t>(std::get<int64_t>(args[2].v));
        BattleSubject* subject = (subjectType == 0) ? BattleManager::instance().getActor(subjectIndex) : BattleManager::instance().getEnemy(subjectIndex);
        if (!subject) return Value::Int(0);
        return Value::Int(BattleManager::instance().isStateActive(subject, stateId) ? 1 : 0);
    }, CompatStatus::FULL));

    ctx.registerObject("BattleManager", methods);
}
} // namespace compat
} // namespace urpg
