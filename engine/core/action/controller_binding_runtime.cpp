#include "engine/core/action/controller_binding_runtime.h"

#include <array>
#include <stdexcept>

namespace urpg::action {

namespace {

constexpr std::int32_t kControllerCodeBase = 1000;

} // namespace

ControllerBindingRuntime::ControllerBindingRuntime() {
    resetToDefaults();
}

void ControllerBindingRuntime::bindButton(ControllerButton button, urpg::input::InputAction action) {
    remap_store_.setMapping(buttonToCode(button), action);
    unsaved_changes_ = true;
}

void ControllerBindingRuntime::clearBinding(ControllerButton button) {
    remap_store_.removeMapping(buttonToCode(button));
    unsaved_changes_ = true;
}

std::optional<urpg::input::InputAction> ControllerBindingRuntime::getBinding(ControllerButton button) const {
    return remap_store_.getMapping(buttonToCode(button));
}

std::map<ControllerButton, urpg::input::InputAction> ControllerBindingRuntime::getAllBindings() const {
    std::map<ControllerButton, urpg::input::InputAction> bindings;

    for (const auto& [code, action] : remap_store_.getAllMappings()) {
        const auto button = codeToButton(code);
        if (button.has_value()) {
            bindings.emplace(*button, action);
        }
    }

    return bindings;
}

void ControllerBindingRuntime::resetToDefaults() {
    remap_store_.clear();
    for (const auto& [button, action] : buildDefaultBindings()) {
        remap_store_.setMapping(buttonToCode(button), action);
    }
    unsaved_changes_ = false;
}

nlohmann::json ControllerBindingRuntime::saveToJson() const {
    nlohmann::json bindings = nlohmann::json::array();

    for (const auto& [button, action] : getAllBindings()) {
        bindings.push_back({
            {"button", buttonToString(button)},
            {"action", actionToString(action)},
        });
    }

    return {
        {"version", "1.0.0"},
        {"bindings", std::move(bindings)},
    };
}

void ControllerBindingRuntime::loadFromJson(const nlohmann::json& value) {
    if (!value.contains("version") || !value.at("version").is_string()) {
        throw std::invalid_argument("ControllerBindingRuntime: missing or invalid 'version' field");
    }

    if (value.at("version").get<std::string>() != "1.0.0") {
        throw std::invalid_argument("ControllerBindingRuntime: unsupported version '" +
                                    value.at("version").get<std::string>() + "'");
    }

    remap_store_.clear();

    if (value.contains("bindings")) {
        if (!value.at("bindings").is_array()) {
            throw std::invalid_argument("ControllerBindingRuntime: 'bindings' must be an array");
        }

        for (const auto& binding : value.at("bindings")) {
            if (!binding.is_object()) {
                throw std::invalid_argument("ControllerBindingRuntime: binding entries must be objects");
            }
            if (!binding.contains("button") || !binding.at("button").is_string()) {
                throw std::invalid_argument("ControllerBindingRuntime: binding entry missing string 'button'");
            }
            if (!binding.contains("action") || !binding.at("action").is_string()) {
                throw std::invalid_argument("ControllerBindingRuntime: binding entry missing string 'action'");
            }

            const auto button = stringToButton(binding.at("button").get<std::string>());
            if (!button.has_value()) {
                throw std::invalid_argument("ControllerBindingRuntime: unknown controller button '" +
                                            binding.at("button").get<std::string>() + "'");
            }

            const auto action = stringToAction(binding.at("action").get<std::string>());
            if (!action.has_value()) {
                throw std::invalid_argument("ControllerBindingRuntime: unknown input action '" +
                                            binding.at("action").get<std::string>() + "'");
            }

            remap_store_.setMapping(buttonToCode(*button), *action);
        }
    }

    unsaved_changes_ = false;
}

bool ControllerBindingRuntime::hasUnsavedChanges() const {
    return unsaved_changes_;
}

std::vector<ControllerBindingIssue> ControllerBindingRuntime::getIssues() const {
    std::vector<ControllerBindingIssue> issues;

    for (const auto action : getMissingRequiredActions()) {
        issues.push_back({
            ControllerBindingIssueSeverity::Error,
            ControllerBindingIssueCategory::MissingRequiredAction,
            action,
            "Required controller action '" + actionToString(action) + "' is not bound.",
        });
    }

    return issues;
}

std::vector<urpg::input::InputAction> ControllerBindingRuntime::getMissingRequiredActions() const {
    std::vector<urpg::input::InputAction> missing;
    const auto bindings = getAllBindings();

    for (const auto action : requiredActions()) {
        bool found = false;
        for (const auto& [button, candidate] : bindings) {
            static_cast<void>(button);
            if (candidate == action) {
                found = true;
                break;
            }
        }

        if (!found) {
            missing.push_back(action);
        }
    }

    return missing;
}

std::string ControllerBindingRuntime::buttonToString(ControllerButton button) {
    switch (button) {
    case ControllerButton::DPadUp: return "DPadUp";
    case ControllerButton::DPadDown: return "DPadDown";
    case ControllerButton::DPadLeft: return "DPadLeft";
    case ControllerButton::DPadRight: return "DPadRight";
    case ControllerButton::FaceBottom: return "FaceBottom";
    case ControllerButton::FaceRight: return "FaceRight";
    case ControllerButton::FaceLeft: return "FaceLeft";
    case ControllerButton::FaceTop: return "FaceTop";
    case ControllerButton::LeftShoulder: return "LeftShoulder";
    case ControllerButton::RightShoulder: return "RightShoulder";
    case ControllerButton::Select: return "Select";
    case ControllerButton::Start: return "Start";
    case ControllerButton::LeftStickPress: return "LeftStickPress";
    case ControllerButton::RightStickPress: return "RightStickPress";
    }

    return "Unknown";
}

std::optional<ControllerButton> ControllerBindingRuntime::stringToButton(const std::string& value) {
    static const std::array<std::pair<const char*, ControllerButton>, 14> kButtonMap{{
        {"DPadUp", ControllerButton::DPadUp},
        {"DPadDown", ControllerButton::DPadDown},
        {"DPadLeft", ControllerButton::DPadLeft},
        {"DPadRight", ControllerButton::DPadRight},
        {"FaceBottom", ControllerButton::FaceBottom},
        {"FaceRight", ControllerButton::FaceRight},
        {"FaceLeft", ControllerButton::FaceLeft},
        {"FaceTop", ControllerButton::FaceTop},
        {"LeftShoulder", ControllerButton::LeftShoulder},
        {"RightShoulder", ControllerButton::RightShoulder},
        {"Select", ControllerButton::Select},
        {"Start", ControllerButton::Start},
        {"LeftStickPress", ControllerButton::LeftStickPress},
        {"RightStickPress", ControllerButton::RightStickPress},
    }};

    for (const auto& [name, button] : kButtonMap) {
        if (value == name) {
            return button;
        }
    }

    return std::nullopt;
}

std::string ControllerBindingRuntime::actionToString(urpg::input::InputAction action) {
    switch (action) {
    case urpg::input::InputAction::None: return "None";
    case urpg::input::InputAction::MoveUp: return "MoveUp";
    case urpg::input::InputAction::MoveDown: return "MoveDown";
    case urpg::input::InputAction::MoveLeft: return "MoveLeft";
    case urpg::input::InputAction::MoveRight: return "MoveRight";
    case urpg::input::InputAction::Confirm: return "Confirm";
    case urpg::input::InputAction::Cancel: return "Cancel";
    case urpg::input::InputAction::Menu: return "Menu";
    case urpg::input::InputAction::Debug: return "Debug";
    }

    return "None";
}

std::optional<urpg::input::InputAction> ControllerBindingRuntime::stringToAction(const std::string& value) {
    using urpg::input::InputAction;

    if (value == "None") {
        return InputAction::None;
    }
    if (value == "MoveUp") {
        return InputAction::MoveUp;
    }
    if (value == "MoveDown") {
        return InputAction::MoveDown;
    }
    if (value == "MoveLeft") {
        return InputAction::MoveLeft;
    }
    if (value == "MoveRight") {
        return InputAction::MoveRight;
    }
    if (value == "Confirm") {
        return InputAction::Confirm;
    }
    if (value == "Cancel") {
        return InputAction::Cancel;
    }
    if (value == "Menu") {
        return InputAction::Menu;
    }
    if (value == "Debug") {
        return InputAction::Debug;
    }

    return std::nullopt;
}

std::int32_t ControllerBindingRuntime::buttonToCode(ControllerButton button) {
    return kControllerCodeBase + static_cast<std::int32_t>(button);
}

std::optional<ControllerButton> ControllerBindingRuntime::codeToButton(std::int32_t code) {
    const auto raw = code - kControllerCodeBase;
    if (raw < 0 || raw > static_cast<std::int32_t>(ControllerButton::RightStickPress)) {
        return std::nullopt;
    }

    return static_cast<ControllerButton>(raw);
}

std::map<ControllerButton, urpg::input::InputAction> ControllerBindingRuntime::buildDefaultBindings() {
    using urpg::input::InputAction;

    return {
        {ControllerButton::DPadUp, InputAction::MoveUp},
        {ControllerButton::DPadDown, InputAction::MoveDown},
        {ControllerButton::DPadLeft, InputAction::MoveLeft},
        {ControllerButton::DPadRight, InputAction::MoveRight},
        {ControllerButton::FaceBottom, InputAction::Confirm},
        {ControllerButton::FaceRight, InputAction::Cancel},
        {ControllerButton::Start, InputAction::Menu},
        {ControllerButton::FaceTop, InputAction::Debug},
    };
}

std::vector<urpg::input::InputAction> ControllerBindingRuntime::requiredActions() {
    using urpg::input::InputAction;

    return {
        InputAction::MoveUp,
        InputAction::MoveDown,
        InputAction::MoveLeft,
        InputAction::MoveRight,
        InputAction::Confirm,
        InputAction::Cancel,
        InputAction::Menu,
    };
}

} // namespace urpg::action
