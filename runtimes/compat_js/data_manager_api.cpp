#include "runtimes/compat_js/data_manager.h"

#include <vector>

namespace urpg {
namespace compat {
// ============================================================================
// Compat Status
// ============================================================================

CompatStatus DataManager::getMethodStatus(const std::string& methodName) {
    auto it = methodStatus_.find(methodName);
    if (it != methodStatus_.end()) {
        return it->second;
    }
    return CompatStatus::UNSUPPORTED;
}

std::string DataManager::getMethodDeviation(const std::string& methodName) {
    auto it = methodDeviations_.find(methodName);
    if (it != methodDeviations_.end()) {
        return it->second;
    }
    return "";
}

void DataManager::registerAPI(QuickJSContext& ctx) {
    std::vector<QuickJSContext::MethodDef> methods;

    methods.push_back({"loadDatabase", [](const std::vector<Value>&) -> Value {
        return Value::Int(DataManager::instance().loadDatabase() ? 1 : 0);
    }, CompatStatus::FULL});

    methods.push_back({"saveGame", [](const std::vector<Value>& args) -> Value {
        if (args.empty() || !std::holds_alternative<int64_t>(args[0].v)) return Value::Int(0);
        return Value::Int(DataManager::instance().saveGame(static_cast<int32_t>(std::get<int64_t>(args[0].v))) ? 1 : 0);
    }, CompatStatus::FULL});

    methods.push_back({"loadGame", [](const std::vector<Value>& args) -> Value {
        if (args.empty() || !std::holds_alternative<int64_t>(args[0].v)) return Value::Int(0);
        return Value::Int(DataManager::instance().loadGame(static_cast<int32_t>(std::get<int64_t>(args[0].v))) ? 1 : 0);
    }, CompatStatus::FULL});

    methods.push_back({"getGold", [](const std::vector<Value>&) -> Value {
        return Value::Int(DataManager::instance().getGold());
    }, CompatStatus::FULL});

    methods.push_back({"setGold", [](const std::vector<Value>& args) -> Value {
        if (args.empty() || !std::holds_alternative<int64_t>(args[0].v)) return Value::Nil();
        DataManager::instance().setGold(static_cast<int32_t>(std::get<int64_t>(args[0].v)));
        return Value::Nil();
    }, CompatStatus::FULL});

    methods.push_back({"getSwitch", [](const std::vector<Value>& args) -> Value {
        if (args.empty() || !std::holds_alternative<int64_t>(args[0].v)) return Value::Int(0);
        return Value::Int(DataManager::instance().getSwitch(static_cast<int32_t>(std::get<int64_t>(args[0].v))) ? 1 : 0);
    }, CompatStatus::FULL});

    methods.push_back({"setSwitch", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 2 || !std::holds_alternative<int64_t>(args[0].v)) return Value::Nil();
        bool val = false;
        if (std::holds_alternative<bool>(args[1].v)) val = std::get<bool>(args[1].v);
        else if (std::holds_alternative<int64_t>(args[1].v)) val = std::get<int64_t>(args[1].v) != 0;
        DataManager::instance().setSwitch(static_cast<int32_t>(std::get<int64_t>(args[0].v)), val);
        return Value::Nil();
    }, CompatStatus::FULL});

    methods.push_back({"getVariable", [](const std::vector<Value>& args) -> Value {
        if (args.empty() || !std::holds_alternative<int64_t>(args[0].v)) return Value::Int(0);
        return Value::Int(DataManager::instance().getVariable(static_cast<int32_t>(std::get<int64_t>(args[0].v))));
    }, CompatStatus::FULL});

    methods.push_back({"setVariable", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 2 || !std::holds_alternative<int64_t>(args[0].v)) return Value::Nil();
        int32_t val = 0;
        if (std::holds_alternative<int64_t>(args[1].v)) val = static_cast<int32_t>(std::get<int64_t>(args[1].v));
        else if (std::holds_alternative<double>(args[1].v)) val = static_cast<int32_t>(std::get<double>(args[1].v));
        DataManager::instance().setVariable(static_cast<int32_t>(std::get<int64_t>(args[0].v)), val);
        return Value::Nil();
    }, CompatStatus::FULL});

    methods.push_back({"getItemCount", [](const std::vector<Value>& args) -> Value {
        if (args.empty() || !std::holds_alternative<int64_t>(args[0].v)) return Value::Int(0);
        return Value::Int(DataManager::instance().getItemCount(static_cast<int32_t>(std::get<int64_t>(args[0].v))));
    }, CompatStatus::FULL});

    methods.push_back({"gainItem", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 2 || !std::holds_alternative<int64_t>(args[0].v) || !std::holds_alternative<int64_t>(args[1].v)) return Value::Nil();
        DataManager::instance().gainItem(static_cast<int32_t>(std::get<int64_t>(args[0].v)), static_cast<int32_t>(std::get<int64_t>(args[1].v)));
        return Value::Nil();
    }, CompatStatus::FULL});

    ctx.registerObject("DataManager", methods);
}
} // namespace compat
} // namespace urpg
