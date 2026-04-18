#include "game_troop.h"
#include "data_manager.h"
#include "quickjs_runtime.h"

namespace urpg {
namespace compat {

namespace {
    int64_t valueToInt64(const urpg::Value& value, int64_t fallback = 0) {
        if (const auto* integer = std::get_if<int64_t>(&value.v)) {
            return *integer;
        }
        if (const auto* real = std::get_if<double>(&value.v)) {
            return static_cast<int64_t>(std::llround(*real));
        }
        return fallback;
    }
}

void GameTroop::setMembers(const std::vector<int32_t>& members) {
    members_ = members;
}

int32_t GameTroop::size() const {
    return static_cast<int32_t>(members_.size());
}

bool GameTroop::isEmpty() const {
    return members_.empty();
}

int32_t GameTroop::totalExp() const {
    int32_t total = 0;
    for (int32_t memberId : members_) {
        const EnemyData* enemy = DataManager::instance().getEnemy(memberId);
        if (enemy) {
            total += enemy->exp;
        }
    }
    return total;
}

int32_t GameTroop::totalGold() const {
    int32_t total = 0;
    for (int32_t memberId : members_) {
        const EnemyData* enemy = DataManager::instance().getEnemy(memberId);
        if (enemy) {
            total += enemy->gold;
        }
    }
    return total;
}

void GameTroop::registerAPI(QuickJSContext& ctx, GameTroop& troop) {
    std::vector<QuickJSContext::MethodDef> methods;
    
    methods.push_back({"members", [&troop](const std::vector<Value>&) -> Value {
        Array arr;
        for (int32_t id : troop.members()) {
            arr.push_back(Value::Int(id));
        }
        return Value::Arr(std::move(arr));
    }, CompatStatus::FULL});
    
    methods.push_back({"size", [&troop](const std::vector<Value>&) -> Value {
        return Value::Int(troop.size());
    }, CompatStatus::FULL});
    
    methods.push_back({"isEmpty", [&troop](const std::vector<Value>&) -> Value {
        return Value::Int(troop.isEmpty() ? 1 : 0);
    }, CompatStatus::FULL});
    
    methods.push_back({"totalExp", [&troop](const std::vector<Value>&) -> Value {
        return Value::Int(troop.totalExp());
    }, CompatStatus::FULL});
    
    methods.push_back({"totalGold", [&troop](const std::vector<Value>&) -> Value {
        return Value::Int(troop.totalGold());
    }, CompatStatus::FULL});
    
    ctx.registerObject("$gameTroop", methods);
}

} // namespace compat
} // namespace urpg
