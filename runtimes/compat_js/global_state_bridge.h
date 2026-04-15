#pragma once

#include "quickjs_runtime.h"

namespace urpg {
namespace compat {

/**
 * @brief Bridge for GlobalStateHub (Switches/Variables) into the JS runtime.
 * Provides the $gameSwitches and $gameVariables behavior for MZ compatibility.
 */
class GlobalStateJSBridge {
public:
    /**
     * @brief Injects the global state bridge into a QuickJS context.
     * Sets up global objects: $gameSwitches, $gameVariables.
     */
    static void registerBridge(QuickJSContext& context);

private:
    // JS Callback: $gameSwitches.value(id)
    static Value getSwitch(const Array& args);
    // JS Callback: $gameSwitches.setValue(id, value)
    static Value setSwitch(const Array& args);

    // JS Callback: $gameVariables.value(id)
    static Value getVariable(const Array& args);
    // JS Callback: $gameVariables.setValue(id, value)
    static Value setVariable(const Array& args);
};

} // namespace compat
} // namespace urpg
