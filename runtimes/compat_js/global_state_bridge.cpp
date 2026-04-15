#include "global_state_bridge.h"
#include "engine/core/global_state_hub.h"
#include <iostream>

namespace urpg {
namespace compat {

void GlobalStateJSBridge::registerBridge(QuickJSContext& context) {
    // 1. Setup $gameSwitches
    // This is simplified to show the mapping to GlobalStateHub.
    // In a real implementation, this would involve using context.addGlobalObject
    // or similar methods to expose C++ functions to JS.
    
    /* 
    context.setGlobalFunction("$gameSwitches_value", getSwitch);
    context.setGlobalFunction("$gameSwitches_setValue", setSwitch);
    context.setGlobalFunction("$gameVariables_value", getVariable);
    context.setGlobalFunction("$gameVariables_setValue", setVariable);

    // After setting functions, we inject the MZ object shells in JS
    context.eval(R"(
        window.$gameSwitches = {
            value: function(id) { return $gameSwitches_value(id); },
            setValue: function(id, val) { $gameSwitches_setValue(id, val); }
        };
        window.$gameVariables = {
            value: function(id) { return $gameVariables_value(id); },
            setValue: function(id, val) { $gameVariables_setValue(id, val); }
        };
    )");
    */

    std::cout << "[URPG][JS][Bridge] $gameSwitches & $gameVariables bridge registered\n";
}

Value GlobalStateJSBridge::getSwitch(const Array& args) {
    if (args.empty()) return Value::Nil();
    
    // In RPVM/MZ, IDs can be string or int
    std::string idStr;
    const auto& idArg = args[0].v;
    if (std::holds_alternative<int64_t>(idArg)) {
        idStr = std::to_string(std::get<int64_t>(idArg));
    } else if (std::holds_alternative<std::string>(idArg)) {
        idStr = std::get<std::string>(idArg);
    } else {
        return Value::Nil();
    }

    bool val = GlobalStateHub::getInstance().getSwitch(idStr);
    Value out;
    out.v = val;
    return out;
}

Value GlobalStateJSBridge::setSwitch(const Array& args) {
    if (args.size() < 2) return Value::Nil();

    std::string idStr;
    const auto& idArg = args[0].v;
    if (std::holds_alternative<int64_t>(idArg)) {
        idStr = std::to_string(std::get<int64_t>(idArg));
    } else if (std::holds_alternative<std::string>(idArg)) {
        idStr = std::get<std::string>(idArg);
    } else {
        return Value::Nil();
    }

    const auto& valArg = args[1].v;
    if (std::holds_alternative<bool>(valArg)) {
        GlobalStateHub::getInstance().setSwitch(idStr, std::get<bool>(valArg));
    }

    return Value::Nil();
}

Value GlobalStateJSBridge::getVariable(const Array& args) {
    if (args.empty()) return Value::Nil();
    
    std::string idStr;
    const auto& idArg = args[0].v;
    if (std::holds_alternative<int64_t>(idArg)) {
        idStr = std::to_string(std::get<int64_t>(idArg));
    } else if (std::holds_alternative<std::string>(idArg)) {
        idStr = std::get<std::string>(idArg);
    } else {
        return Value::Nil();
    }

    auto hubVal = GlobalStateHub::getInstance().getVariable(idStr);
    Value out;
    
    // Convert GlobalStateHub::Value to urpg::Value
    std::visit([&out](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, bool>) {
            out.v = arg;
        } else if constexpr (std::is_same_v<T, int32_t>) {
            out.v = (int64_t)arg;
        } else if constexpr (std::is_same_v<T, float>) {
            out.v = (double)arg;
        } else if constexpr (std::is_same_v<T, std::string>) {
            out.v = arg;
        }
    }, hubVal);

    return out;
}

Value GlobalStateJSBridge::setVariable(const Array& args) {
    if (args.size() < 2) return Value::Nil();

    std::string idStr;
    const auto& idArg = args[0].v;
    if (std::holds_alternative<int64_t>(idArg)) {
        idStr = std::to_string(std::get<int64_t>(idArg));
    } else if (std::holds_alternative<std::string>(idArg)) {
        idStr = std::get<std::string>(idArg);
    } else {
        return Value::Nil();
    }

    const auto& valArg = args[1].v;
    GlobalStateHub::Value hubVal;
    
    // Map urpg::Value to GlobalStateHub::Value
    std::visit([&hubVal](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, bool>) {
            hubVal = arg;
        } else if constexpr (std::is_same_v<T, int64_t>) {
            hubVal = (int32_t)arg;
        } else if constexpr (std::is_same_v<T, double>) {
            hubVal = (float)arg;
        } else if constexpr (std::is_same_v<T, std::string>) {
            hubVal = arg;
        }
    }, valArg);

    GlobalStateHub::getInstance().setVariable(idStr, hubVal);
    return Value::Nil();
}

} // namespace compat
} // namespace urpg
