#include "engine/core/input/input_remap_profile.h"

#include <map>
#include <stdexcept>

namespace urpg::input {

namespace {

bool isSupportedRemapDevice(const std::string& device) {
    return device == "keyboard" || device == "controller";
}

std::string actionToString(InputAction action) {
    switch (action) {
        case InputAction::MoveUp: return "move_up";
        case InputAction::MoveDown: return "move_down";
        case InputAction::MoveLeft: return "move_left";
        case InputAction::MoveRight: return "move_right";
        case InputAction::Confirm: return "confirm";
        case InputAction::Cancel: return "cancel";
        case InputAction::Menu: return "menu";
        case InputAction::PageLeft: return "page_left";
        case InputAction::PageRight: return "page_right";
        case InputAction::BattleAttack: return "battle_attack";
        case InputAction::BattleSkill: return "battle_skill";
        case InputAction::BattleItem: return "battle_item";
        case InputAction::BattleDefend: return "battle_defend";
        case InputAction::BattleEscape: return "battle_escape";
        case InputAction::Debug: return "debug";
        case InputAction::None: return "none";
    }
    return "none";
}

InputAction actionFromString(const std::string& action) {
    static const std::map<std::string, InputAction> actions = {
        {"none", InputAction::None}, {"move_up", InputAction::MoveUp}, {"move_down", InputAction::MoveDown},
        {"move_left", InputAction::MoveLeft}, {"move_right", InputAction::MoveRight}, {"confirm", InputAction::Confirm},
        {"cancel", InputAction::Cancel}, {"menu", InputAction::Menu}, {"page_left", InputAction::PageLeft},
        {"page_right", InputAction::PageRight}, {"battle_attack", InputAction::BattleAttack},
        {"battle_skill", InputAction::BattleSkill}, {"battle_item", InputAction::BattleItem},
        {"battle_defend", InputAction::BattleDefend}, {"battle_escape", InputAction::BattleEscape}, {"debug", InputAction::Debug},
    };
    const auto found = actions.find(action);
    return found == actions.end() ? InputAction::None : found->second;
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

nlohmann::json InputRemapProfile::toJson() const {
    nlohmann::json bindings = nlohmann::json::array();
    for (const auto& binding : bindings_) {
        bindings.push_back({{"device", binding.token.device}, {"control", binding.token.control},
                            {"action", actionToString(binding.action)},
                            {"accessibility_duplicate", binding.accessibility_duplicate}});
    }
    return {{"schema", "urpg.input_remap_profile.v1"}, {"bindings", std::move(bindings)}};
}

InputRemapProfile InputRemapProfile::fromJson(const nlohmann::json& json) {
    if (json.value("schema", "") != "urpg.input_remap_profile.v1") {
        throw std::invalid_argument("Input remap profile has an unsupported schema.");
    }
    InputRemapProfile profile;
    const auto bindings = json.value("bindings", nlohmann::json::array());
    if (!bindings.is_array()) throw std::invalid_argument("Input remap profile bindings must be an array.");
    for (const auto& binding : bindings) {
        const InputBindingToken token{binding.value("device", ""), binding.value("control", "")};
        const auto action = actionFromString(binding.value("action", "none"));
        const auto validation = profile.validateBinding(token, action, binding.value("accessibility_duplicate", false));
        if (token.control.empty() || action == InputAction::None || !validation.accepted) {
            throw std::invalid_argument("Input remap profile contains an invalid binding.");
        }
        profile.bind(token, action, binding.value("accessibility_duplicate", false));
    }
    return profile;
}

} // namespace urpg::input
