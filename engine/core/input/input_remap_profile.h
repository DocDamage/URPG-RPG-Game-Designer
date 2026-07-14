#pragma once

#include "engine/core/input/input_core.h"

#include <nlohmann/json.hpp>

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
    std::string message;
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
    [[nodiscard]] nlohmann::json toJson() const;
    static InputRemapProfile fromJson(const nlohmann::json& json);

private:
    std::vector<InputProfileBinding> bindings_;
};

} // namespace urpg::input
