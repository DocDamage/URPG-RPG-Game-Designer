#pragma once

#include "engine/core/accessibility/inclusive_experience.h"
#include "engine/core/action/controller_binding_runtime.h"
#include "engine/core/input/input_core.h"

#include <map>
#include <optional>
#include <string>
#include <string_view>

namespace urpg::input {

enum class ControllerAxis { LeftX, LeftY };

class ControllerInputProvider {
public:
    ControllerInputProvider();

    bool connect(std::string device_id, std::string glyph_profile,
                 accessibility::InputOwnership owner = accessibility::InputOwnership::Shared,
                 float deadzone = 0.2F, float axis_scale = 1.0F);
    bool disconnect(InputCore& core, std::string_view device_id);
    bool reconnect(std::string_view device_id);
    bool calibrate(std::string_view device_id, float deadzone, float axis_scale);
    bool applyBindings(const action::ControllerBindingRuntime& bindings);

    accessibility::InputRouteResult buttonEvent(
        InputCore& core, std::string_view device_id, action::ControllerButton button,
        ActionState state, accessibility::InclusiveInputContext context,
        uint64_t held_ms = 0);
    accessibility::InputRouteResult axisEvent(
        InputCore& core, std::string_view device_id, ControllerAxis axis, float raw_value,
        accessibility::InclusiveInputContext context, uint64_t held_ms = 0);

    size_t connectedDeviceCount() const;
    std::string activeDeviceId() const { return router_.activeDeviceId(); }
    std::vector<std::string> recoveryActions() const { return router_.recoveryActions(); }

private:
    struct DeviceState {
        bool connected = true;
        float deadzone = 0.2F;
        std::map<std::string, InputAction> active_controls;
        std::optional<std::string> horizontal_axis_control;
        std::optional<std::string> vertical_axis_control;
    };

    accessibility::InputRouteResult pressControl(
        InputCore& core, const std::string& device_id, const std::string& control,
        float raw_axis, accessibility::InclusiveInputContext context, uint64_t held_ms);
    accessibility::InputRouteResult releaseControl(
        InputCore& core, const std::string& device_id, const std::string& control,
        accessibility::InclusiveInputContext context);
    bool actionActiveFromAnotherSource(InputAction action, std::string_view excluded_device,
                                       std::string_view excluded_control) const;
    void configureBindings();

    accessibility::UnifiedInputRouter router_;
    std::map<action::ControllerButton, InputAction> bindings_;
    std::map<std::string, DeviceState> devices_;
};

} // namespace urpg::input
