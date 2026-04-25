#include "engine/core/input/input_remap_profile.h"

namespace urpg::input {

BindingValidation InputRemapProfile::validateBinding(InputBindingToken token, InputAction action, bool allow_accessibility_duplicate) const {
    for (const auto& binding : bindings_) {
        if (binding.token.device == token.device && binding.token.control == token.control && binding.action != action && !allow_accessibility_duplicate) {
            return {false, "binding_conflict"};
        }
    }
    return {true, ""};
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
