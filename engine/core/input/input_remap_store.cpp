#include "engine/core/input/input_remap_store.h"

#include <stdexcept>
#include <string>
#include <unordered_map>

namespace urpg::input {

namespace {

// Common virtual-key codes used for the default MZ-compatible layout.
constexpr int32_t VK_LEFT = 37;
constexpr int32_t VK_UP = 38;
constexpr int32_t VK_RIGHT = 39;
constexpr int32_t VK_DOWN = 40;
constexpr int32_t VK_ENTER = 13;
constexpr int32_t VK_ESCAPE = 27;
constexpr int32_t VK_PAGE_UP = 33;
constexpr int32_t VK_PAGE_DOWN = 34;
constexpr int32_t VK_A = 65;
constexpr int32_t VK_Z = 90;
constexpr int32_t VK_D = 68;
constexpr int32_t VK_E = 69;
constexpr int32_t VK_I = 73;
constexpr int32_t VK_Q = 81;
constexpr int32_t VK_S = 83;
constexpr int32_t VK_X = 88;
constexpr int32_t VK_C = 67;

const std::unordered_map<std::string, InputAction> kActionNameMap = {
    {"None", InputAction::None},
    {"MoveUp", InputAction::MoveUp},
    {"MoveDown", InputAction::MoveDown},
    {"MoveLeft", InputAction::MoveLeft},
    {"MoveRight", InputAction::MoveRight},
    {"Confirm", InputAction::Confirm},
    {"Cancel", InputAction::Cancel},
    {"Menu", InputAction::Menu},
    {"PageLeft", InputAction::PageLeft},
    {"PageRight", InputAction::PageRight},
    {"BattleAttack", InputAction::BattleAttack},
    {"BattleSkill", InputAction::BattleSkill},
    {"BattleItem", InputAction::BattleItem},
    {"BattleDefend", InputAction::BattleDefend},
    {"BattleEscape", InputAction::BattleEscape},
    {"Debug", InputAction::Debug},
};

} // anonymous namespace

std::map<int32_t, InputAction> InputRemapStore::buildDefaultMappings() {
    std::map<int32_t, InputAction> defs;
    defs[VK_UP] = InputAction::MoveUp;
    defs[VK_DOWN] = InputAction::MoveDown;
    defs[VK_LEFT] = InputAction::MoveLeft;
    defs[VK_RIGHT] = InputAction::MoveRight;
    defs[VK_Z] = InputAction::Confirm;
    defs[VK_ENTER] = InputAction::Confirm;
    defs[VK_X] = InputAction::Cancel;
    defs[VK_ESCAPE] = InputAction::Cancel;
    defs[VK_C] = InputAction::Menu;
    defs[VK_PAGE_UP] = InputAction::PageLeft;
    defs[VK_Q] = InputAction::PageLeft;
    defs[VK_PAGE_DOWN] = InputAction::PageRight;
    defs[VK_A] = InputAction::BattleAttack;
    defs[VK_S] = InputAction::BattleSkill;
    defs[VK_I] = InputAction::BattleItem;
    defs[VK_D] = InputAction::BattleDefend;
    defs[VK_E] = InputAction::BattleEscape;
    return defs;
}

std::string InputRemapStore::actionToString(InputAction action) {
    switch (action) {
        case InputAction::None: return "None";
        case InputAction::MoveUp: return "MoveUp";
        case InputAction::MoveDown: return "MoveDown";
        case InputAction::MoveLeft: return "MoveLeft";
        case InputAction::MoveRight: return "MoveRight";
        case InputAction::Confirm: return "Confirm";
        case InputAction::Cancel: return "Cancel";
        case InputAction::Menu: return "Menu";
        case InputAction::PageLeft: return "PageLeft";
        case InputAction::PageRight: return "PageRight";
        case InputAction::BattleAttack: return "BattleAttack";
        case InputAction::BattleSkill: return "BattleSkill";
        case InputAction::BattleItem: return "BattleItem";
        case InputAction::BattleDefend: return "BattleDefend";
        case InputAction::BattleEscape: return "BattleEscape";
        case InputAction::Debug: return "Debug";
    }
    return "None";
}

InputAction InputRemapStore::stringToAction(const std::string& name) {
    auto it = kActionNameMap.find(name);
    if (it != kActionNameMap.end()) {
        return it->second;
    }
    return InputAction::None;
}

InputRemapStore::InputRemapStore()
    : m_defaults(buildDefaultMappings()) {
    resetToDefaults();
}

void InputRemapStore::setMapping(int32_t keyCode, InputAction action) {
    m_mappings[keyCode] = action;
    m_unsavedChanges = true;
}

void InputRemapStore::removeMapping(int32_t keyCode) {
    auto it = m_mappings.find(keyCode);
    if (it != m_mappings.end()) {
        m_mappings.erase(it);
        m_unsavedChanges = true;
    }
}

std::optional<InputAction> InputRemapStore::getMapping(int32_t keyCode) const {
    auto it = m_mappings.find(keyCode);
    if (it != m_mappings.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::map<int32_t, InputAction> InputRemapStore::getAllMappings() const {
    return m_mappings;
}

void InputRemapStore::resetToDefaults() {
    m_mappings = m_defaults;
    m_unsavedChanges = false;
}

void InputRemapStore::clear() {
    m_mappings.clear();
    m_unsavedChanges = true;
}

nlohmann::json InputRemapStore::saveToJson() const {
    nlohmann::json j;
    j["version"] = "1.0.0";
    nlohmann::json bindings = nlohmann::json::array();
    for (const auto& [keyCode, action] : m_mappings) {
        nlohmann::json entry;
        entry["keyCode"] = keyCode;
        entry["action"] = actionToString(action);
        bindings.push_back(entry);
    }
    j["bindings"] = bindings;
    return j;
}

void InputRemapStore::loadFromJson(const nlohmann::json& j) {
    if (!j.contains("version") || !j["version"].is_string()) {
        throw std::invalid_argument("InputRemapStore: missing or invalid 'version' field");
    }

    const std::string version = j["version"];
    if (version != "1.0.0") {
        throw std::invalid_argument("InputRemapStore: unsupported version '" + version + "'");
    }

    clear();

    if (j.contains("bindings") && j["bindings"].is_array()) {
        for (const auto& entry : j["bindings"]) {
            if (!entry.contains("keyCode") || !entry["keyCode"].is_number_integer()) {
                continue;
            }
            if (!entry.contains("action") || !entry["action"].is_string()) {
                continue;
            }
            int32_t keyCode = entry["keyCode"].get<int32_t>();
            std::string actionName = entry["action"].get<std::string>();
            m_mappings[keyCode] = stringToAction(actionName);
        }
    }

    m_unsavedChanges = false;
}

bool InputRemapStore::hasUnsavedChanges() const {
    return m_unsavedChanges;
}

} // namespace urpg::input
