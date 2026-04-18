#include "game_party.h"
#include "quickjs_runtime.h"
#include <algorithm>

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

void GameParty::setMembers(const std::vector<int32_t>& members) {
    members_ = members;
}

int32_t GameParty::size() const {
    return static_cast<int32_t>(members_.size());
}

bool GameParty::isEmpty() const {
    return members_.empty();
}

bool GameParty::exists(int32_t actorId) const {
    for (int32_t id : members_) {
        if (id == actorId) {
            return true;
        }
    }
    return false;
}

void GameParty::addActor(int32_t actorId) {
    if (!exists(actorId)) {
        members_.push_back(actorId);
    }
}

void GameParty::removeActor(int32_t actorId) {
    members_.erase(
        std::remove(members_.begin(), members_.end(), actorId),
        members_.end()
    );
}

void GameParty::setGold(int32_t gold) {
    gold_ = std::max(0, gold);
}

void GameParty::gainGold(int32_t amount) {
    gold_ = std::max(0, gold_ + amount);
}

void GameParty::loseGold(int32_t amount) {
    gold_ = std::max(0, gold_ - amount);
}

bool GameParty::hasGold(int32_t amount) const {
    return gold_ >= amount;
}

int32_t GameParty::numItems(int32_t itemId) const {
    auto it = items_.find(itemId);
    return it != items_.end() ? it->second : 0;
}

void GameParty::gainItem(int32_t itemId, int32_t count) {
    int32_t newCount = numItems(itemId) + count;
    if (newCount <= 0) {
        items_.erase(itemId);
    } else {
        items_[itemId] = newCount;
    }
}

void GameParty::loseItem(int32_t itemId, int32_t count) {
    gainItem(itemId, -count);
}

bool GameParty::hasItem(int32_t itemId) const {
    return numItems(itemId) > 0;
}

int32_t GameParty::numWeapons(int32_t weaponId) const {
    auto it = weapons_.find(weaponId);
    return it != weapons_.end() ? it->second : 0;
}

void GameParty::gainWeapon(int32_t weaponId, int32_t count) {
    int32_t newCount = numWeapons(weaponId) + count;
    if (newCount <= 0) {
        weapons_.erase(weaponId);
    } else {
        weapons_[weaponId] = newCount;
    }
}

void GameParty::loseWeapon(int32_t weaponId, int32_t count) {
    gainWeapon(weaponId, -count);
}

int32_t GameParty::numArmors(int32_t armorId) const {
    auto it = armors_.find(armorId);
    return it != armors_.end() ? it->second : 0;
}

void GameParty::gainArmor(int32_t armorId, int32_t count) {
    int32_t newCount = numArmors(armorId) + count;
    if (newCount <= 0) {
        armors_.erase(armorId);
    } else {
        armors_[armorId] = newCount;
    }
}

void GameParty::loseArmor(int32_t armorId, int32_t count) {
    gainArmor(armorId, -count);
}

void GameParty::increaseSteps(int32_t amount) {
    steps_ += std::max(0, amount);
}

void GameParty::registerAPI(QuickJSContext& ctx, GameParty& party) {
    std::vector<QuickJSContext::MethodDef> methods;
    
    methods.push_back({"members", [&party](const std::vector<Value>&) -> Value {
        Array arr;
        for (int32_t id : party.members()) {
            arr.push_back(Value::Int(id));
        }
        return Value::Arr(std::move(arr));
    }, CompatStatus::FULL});
    
    methods.push_back({"size", [&party](const std::vector<Value>&) -> Value {
        return Value::Int(party.size());
    }, CompatStatus::FULL});
    
    methods.push_back({"isEmpty", [&party](const std::vector<Value>&) -> Value {
        return Value::Int(party.isEmpty() ? 1 : 0);
    }, CompatStatus::FULL});
    
    methods.push_back({"exists", [&party](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Int(0);
        return Value::Int(party.exists(static_cast<int32_t>(valueToInt64(args[0]))) ? 1 : 0);
    }, CompatStatus::FULL});
    
    methods.push_back({"addActor", [&party](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Nil();
        party.addActor(static_cast<int32_t>(valueToInt64(args[0])));
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"removeActor", [&party](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Nil();
        party.removeActor(static_cast<int32_t>(valueToInt64(args[0])));
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"gold", [&party](const std::vector<Value>&) -> Value {
        return Value::Int(party.gold());
    }, CompatStatus::FULL});
    
    methods.push_back({"setGold", [&party](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Nil();
        party.setGold(static_cast<int32_t>(valueToInt64(args[0])));
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"gainGold", [&party](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Nil();
        party.gainGold(static_cast<int32_t>(valueToInt64(args[0])));
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"loseGold", [&party](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Nil();
        party.loseGold(static_cast<int32_t>(valueToInt64(args[0])));
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"hasGold", [&party](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Int(0);
        return Value::Int(party.hasGold(static_cast<int32_t>(valueToInt64(args[0]))) ? 1 : 0);
    }, CompatStatus::FULL});
    
    methods.push_back({"numItems", [&party](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Int(0);
        return Value::Int(party.numItems(static_cast<int32_t>(valueToInt64(args[0]))));
    }, CompatStatus::FULL});
    
    methods.push_back({"gainItem", [&party](const std::vector<Value>& args) -> Value {
        if (args.size() < 2) return Value::Nil();
        party.gainItem(static_cast<int32_t>(valueToInt64(args[0])), static_cast<int32_t>(valueToInt64(args[1])));
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"loseItem", [&party](const std::vector<Value>& args) -> Value {
        if (args.size() < 2) return Value::Nil();
        party.loseItem(static_cast<int32_t>(valueToInt64(args[0])), static_cast<int32_t>(valueToInt64(args[1])));
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"hasItem", [&party](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Int(0);
        return Value::Int(party.hasItem(static_cast<int32_t>(valueToInt64(args[0]))) ? 1 : 0);
    }, CompatStatus::FULL});
    
    methods.push_back({"numWeapons", [&party](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Int(0);
        return Value::Int(party.numWeapons(static_cast<int32_t>(valueToInt64(args[0]))));
    }, CompatStatus::FULL});
    
    methods.push_back({"gainWeapon", [&party](const std::vector<Value>& args) -> Value {
        if (args.size() < 2) return Value::Nil();
        party.gainWeapon(static_cast<int32_t>(valueToInt64(args[0])), static_cast<int32_t>(valueToInt64(args[1])));
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"loseWeapon", [&party](const std::vector<Value>& args) -> Value {
        if (args.size() < 2) return Value::Nil();
        party.loseWeapon(static_cast<int32_t>(valueToInt64(args[0])), static_cast<int32_t>(valueToInt64(args[1])));
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"numArmors", [&party](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Int(0);
        return Value::Int(party.numArmors(static_cast<int32_t>(valueToInt64(args[0]))));
    }, CompatStatus::FULL});
    
    methods.push_back({"gainArmor", [&party](const std::vector<Value>& args) -> Value {
        if (args.size() < 2) return Value::Nil();
        party.gainArmor(static_cast<int32_t>(valueToInt64(args[0])), static_cast<int32_t>(valueToInt64(args[1])));
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"loseArmor", [&party](const std::vector<Value>& args) -> Value {
        if (args.size() < 2) return Value::Nil();
        party.loseArmor(static_cast<int32_t>(valueToInt64(args[0])), static_cast<int32_t>(valueToInt64(args[1])));
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"steps", [&party](const std::vector<Value>&) -> Value {
        return Value::Int(party.steps());
    }, CompatStatus::FULL});
    
    methods.push_back({"setSteps", [&party](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Nil();
        party.setSteps(static_cast<int32_t>(valueToInt64(args[0])));
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"increaseSteps", [&party](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) {
            party.increaseSteps();
        } else {
            party.increaseSteps(static_cast<int32_t>(valueToInt64(args[0])));
        }
        return Value::Nil();
    }, CompatStatus::FULL});
    
    ctx.registerObject("$gameParty", methods);
}

} // namespace compat
} // namespace urpg
