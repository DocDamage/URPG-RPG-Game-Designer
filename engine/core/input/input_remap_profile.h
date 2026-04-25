#pragma once

#include "engine/core/input/input_core.h"

#include <string>
#include <vector>

namespace urpg::input {

struct InputBindingToken {
    std::string device;
    std::string control;
};

struct BindingValidation {
    bool accepted{false};
    std::string code;
};

struct InputProfileBinding {
    InputBindingToken token;
    InputAction action{InputAction::None};
    bool accessibility_duplicate{false};
};

class InputRemapProfile {
public:
    [[nodiscard]] BindingValidation validateBinding(InputBindingToken token, InputAction action, bool allow_accessibility_duplicate) const;
    void bind(InputBindingToken token, InputAction action, bool accessibility_duplicate = false);
    [[nodiscard]] std::vector<InputProfileBinding> bindingsFor(const std::string& control) const;

private:
    std::vector<InputProfileBinding> bindings_;
};

} // namespace urpg::input
