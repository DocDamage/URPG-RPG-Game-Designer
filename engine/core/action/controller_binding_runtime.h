#pragma once

#include "engine/core/input/input_core.h"
#include "engine/core/input/input_remap_store.h"

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace urpg::action {

enum class ControllerButton : std::uint32_t {
    DPadUp = 0,
    DPadDown,
    DPadLeft,
    DPadRight,
    FaceBottom,
    FaceRight,
    FaceLeft,
    FaceTop,
    LeftShoulder,
    RightShoulder,
    Select,
    Start,
    LeftStickPress,
    RightStickPress
};

enum class ControllerBindingIssueSeverity {
    Warning,
    Error
};

enum class ControllerBindingIssueCategory {
    MissingRequiredAction
};

struct ControllerBindingIssue {
    ControllerBindingIssueSeverity severity = ControllerBindingIssueSeverity::Error;
    ControllerBindingIssueCategory category = ControllerBindingIssueCategory::MissingRequiredAction;
    urpg::input::InputAction action = urpg::input::InputAction::None;
    std::string message;
};

class ControllerBindingRuntime {
public:
    ControllerBindingRuntime();

    void bindButton(ControllerButton button, urpg::input::InputAction action);
    void clearBinding(ControllerButton button);

    [[nodiscard]] std::optional<urpg::input::InputAction> getBinding(ControllerButton button) const;
    [[nodiscard]] std::map<ControllerButton, urpg::input::InputAction> getAllBindings() const;

    void resetToDefaults();

    [[nodiscard]] nlohmann::json saveToJson() const;
    void loadFromJson(const nlohmann::json& value);

    [[nodiscard]] bool hasUnsavedChanges() const;
    [[nodiscard]] std::vector<ControllerBindingIssue> getIssues() const;
    [[nodiscard]] std::vector<urpg::input::InputAction> getMissingRequiredActions() const;

    [[nodiscard]] static std::string buttonToString(ControllerButton button);
    [[nodiscard]] static std::optional<ControllerButton> stringToButton(const std::string& value);
    [[nodiscard]] static std::string actionToString(urpg::input::InputAction action);
    [[nodiscard]] static std::optional<urpg::input::InputAction> stringToAction(const std::string& value);

private:
    urpg::input::InputRemapStore remap_store_;
    bool unsaved_changes_ = false;

    [[nodiscard]] static std::int32_t buttonToCode(ControllerButton button);
    [[nodiscard]] static std::optional<ControllerButton> codeToButton(std::int32_t code);
    [[nodiscard]] static std::map<ControllerButton, urpg::input::InputAction> buildDefaultBindings();
    [[nodiscard]] static std::vector<urpg::input::InputAction> requiredActions();
};

} // namespace urpg::action
