#pragma once

#include "engine/core/action/controller_binding_runtime.h"

#include <nlohmann/json.hpp>

namespace urpg::editor {

class ControllerBindingPanel {
public:
    ControllerBindingPanel() = default;

    void bindRuntime(urpg::action::ControllerBindingRuntime* runtime);
    void render();

    [[nodiscard]] nlohmann::json lastRenderSnapshot() const;

private:
    urpg::action::ControllerBindingRuntime* runtime_ = nullptr;
    nlohmann::json last_render_snapshot_;
};

} // namespace urpg::editor
