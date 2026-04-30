#include "engine/core/input/input_remap_profile.h"

namespace urpg::input {

namespace {

bool isSupportedRemapDevice(const std::string& device) {
    return device == "keyboard" || device == "controller";
}

} // namespace

BindingValidation InputRemapProfile::validateBinding(InputBindingToken token, InputAction action, bool allow_accessibility_duplicate) const {
    if (!isSupportedRemapDevice(token.device)) {
        if (token.device == "touch") {
            return {false, "touch_binding_unsupported",
                    "Touch input uses hit-test driven UI and world interactions; it is not bindable through the action remap profile."};
        }
        return {false, "input_device_unsupported", "Input device '" + token.device + "' is not supported by the action remap profile."};
    }

    for (const auto& binding : bindings_) {
        if (binding.token.device == token.device && binding.token.control == token.control && binding.action != action && !allow_accessibility_duplicate) {
            return {false, "binding_conflict", "Control is already bound to another action."};
        }
    }
    return {true, "", ""};
}

void InputRemapProfile::bind(InputBindingToken token, InputAction action, bool accessibility_duplicate) {
    bindings_.push_back({std::move(token), action, accessibility_duplicate});
}

std::vector<InputProfileBinding> InputRemapProfile::bindingsFor(const std::string& control) const {
    std::vector<InputProfileBinding> matches;
    for (const auto& binding : bindings_) {
        if (binding.token.control == control) {
            matches.push_back(binding);
        }
    }
    return matches;
}

} // namespace urpg::input
