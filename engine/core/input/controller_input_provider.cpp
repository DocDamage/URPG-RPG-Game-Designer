#include "engine/core/input/controller_input_provider.h"

#include <algorithm>
#include <cmath>

namespace urpg::input {

namespace {

constexpr const char* axisControl(ControllerAxis axis, bool positive) {
    if (axis == ControllerAxis::LeftX) return positive ? "left_x_positive" : "left_x_negative";
    return positive ? "left_y_positive" : "left_y_negative";
}

} // namespace

ControllerInputProvider::ControllerInputProvider() {
    bindings_ = action::ControllerBindingRuntime{}.getAllBindings();
    configureBindings();
}

void ControllerInputProvider::configureBindings() {
    router_.clearBindings();
    for (const auto& [button, action] : bindings_) {
        (void)router_.bind({accessibility::InclusiveInputContext::Global, action,
                            {action::ControllerBindingRuntime::buttonToString(button)}, 350, 80}, false);
    }
    (void)router_.bind({accessibility::InclusiveInputContext::Global, InputAction::MoveLeft,
                        {axisControl(ControllerAxis::LeftX, false)}, 350, 80}, false);
    (void)router_.bind({accessibility::InclusiveInputContext::Global, InputAction::MoveRight,
                        {axisControl(ControllerAxis::LeftX, true)}, 350, 80}, false);
    (void)router_.bind({accessibility::InclusiveInputContext::Global, InputAction::MoveUp,
                        {axisControl(ControllerAxis::LeftY, false)}, 350, 80}, false);
    (void)router_.bind({accessibility::InclusiveInputContext::Global, InputAction::MoveDown,
                        {axisControl(ControllerAxis::LeftY, true)}, 350, 80}, false);
}

bool ControllerInputProvider::applyBindings(const action::ControllerBindingRuntime& bindings) {
    if (std::any_of(devices_.begin(), devices_.end(),
                    [](const auto& item) { return !item.second.active_controls.empty(); })) return false;
    if (!bindings.getIssues().empty()) return false;
    bindings_ = bindings.getAllBindings();
    configureBindings();
    return true;
}

bool ControllerInputProvider::connect(std::string device_id, std::string glyph_profile,
                                      accessibility::InputOwnership owner, float deadzone,
                                      float axis_scale) {
    if (!router_.connectDevice({device_id, accessibility::InclusiveDeviceKind::Controller, owner,
                                true, deadzone, axis_scale, std::move(glyph_profile)})) return false;
    DeviceState state;
    state.connected = true;
    state.deadzone = deadzone;
    devices_[std::move(device_id)] = std::move(state);
    return true;
}

bool ControllerInputProvider::disconnect(InputCore& core, std::string_view device_id) {
    auto found = devices_.find(std::string(device_id));
    if (found == devices_.end() || !found->second.connected) return false;
    const auto controls = found->second.active_controls;
    for (const auto& [control, action] : controls) {
        if (!actionActiveFromAnotherSource(action, device_id, control)) {
            core.updateActionState(action, ActionState::Released);
        }
    }
    found->second.active_controls.clear();
    found->second.horizontal_axis_control.reset();
    found->second.vertical_axis_control.reset();
    found->second.connected = false;
    return router_.disconnectDevice(device_id);
}

bool ControllerInputProvider::reconnect(std::string_view device_id) {
    auto found = devices_.find(std::string(device_id));
    if (found == devices_.end() || found->second.connected || !router_.reconnectDevice(device_id)) return false;
    found->second.connected = true;
    return true;
}

bool ControllerInputProvider::calibrate(std::string_view device_id, float deadzone, float axis_scale) {
    auto found = devices_.find(std::string(device_id));
    if (found == devices_.end() || !router_.calibrate(device_id, deadzone, axis_scale)) return false;
    found->second.deadzone = deadzone;
    return true;
}

accessibility::InputRouteResult ControllerInputProvider::buttonEvent(
    InputCore& core, std::string_view device_id, action::ControllerButton button,
    ActionState state, accessibility::InclusiveInputContext context, uint64_t held_ms) {
    const auto control = action::ControllerBindingRuntime::buttonToString(button);
    if (state == ActionState::Released) return releaseControl(core, std::string(device_id), control, context);
    return pressControl(core, std::string(device_id), control, 0.0F, context, held_ms);
}

accessibility::InputRouteResult ControllerInputProvider::axisEvent(
    InputCore& core, std::string_view device_id, ControllerAxis axis, float raw_value,
    accessibility::InclusiveInputContext context, uint64_t held_ms) {
    accessibility::InputRouteResult result;
    auto found = devices_.find(std::string(device_id));
    if (found == devices_.end() || !found->second.connected || !std::isfinite(raw_value)) {
        result.diagnostics.push_back("Controller axis device is disconnected, unknown, or invalid.");
        return result;
    }
    raw_value = std::clamp(raw_value, -1.0F, 1.0F);
    auto& active = axis == ControllerAxis::LeftX ? found->second.horizontal_axis_control
                                                 : found->second.vertical_axis_control;
    std::optional<std::string> next;
    if (std::abs(raw_value) > found->second.deadzone) next = axisControl(axis, raw_value > 0.0F);
    if (active == next) {
        if (!next) return result;
        return pressControl(core, std::string(device_id), *next, raw_value, context, held_ms);
    }
    if (active) result = releaseControl(core, std::string(device_id), *active, context);
    active = next;
    if (next) result = pressControl(core, std::string(device_id), *next, raw_value, context, held_ms);
    return result;
}

size_t ControllerInputProvider::connectedDeviceCount() const {
    return static_cast<size_t>(std::count_if(devices_.begin(), devices_.end(),
                                             [](const auto& item) { return item.second.connected; }));
}

accessibility::InputRouteResult ControllerInputProvider::pressControl(
    InputCore& core, const std::string& device_id, const std::string& control, float raw_axis,
    accessibility::InclusiveInputContext context, uint64_t held_ms) {
    auto result = router_.route(device_id, context, {control}, raw_axis, held_ms);
    if (!result.accepted) return result;
    auto& state = devices_.at(device_id);
    if (state.active_controls.contains(control)) return result;
    const bool already_active = actionActiveFromAnotherSource(result.action, {}, {});
    state.active_controls[control] = result.action;
    if (!already_active) core.updateActionState(result.action, ActionState::Pressed);
    return result;
}

accessibility::InputRouteResult ControllerInputProvider::releaseControl(
    InputCore& core, const std::string& device_id, const std::string& control,
    accessibility::InclusiveInputContext context) {
    accessibility::InputRouteResult result;
    auto device = devices_.find(device_id);
    if (device == devices_.end()) {
        result.diagnostics.push_back("Controller release device is unknown.");
        return result;
    }
    auto active = device->second.active_controls.find(control);
    if (active == device->second.active_controls.end()) {
        result.diagnostics.push_back("Controller control was not active.");
        return result;
    }
    result = router_.route(device_id, context, {control}, 0.0F, 0);
    result.accepted = true;
    result.action = active->second;
    device->second.active_controls.erase(active);
    if (!actionActiveFromAnotherSource(result.action, {}, {})) {
        core.updateActionState(result.action, ActionState::Released);
    }
    return result;
}

bool ControllerInputProvider::actionActiveFromAnotherSource(
    InputAction action, std::string_view excluded_device, std::string_view excluded_control) const {
    for (const auto& [device_id, device] : devices_) {
        for (const auto& [control, active_action] : device.active_controls) {
            if (device_id == excluded_device && control == excluded_control) continue;
            if (active_action == action) return true;
        }
    }
    return false;
}

} // namespace urpg::input
