#pragma once

#include "engine/core/math/fixed32.h"
#include <string>
#include <vector>

namespace urpg {

/**
 * @brief A single data binding for the UI.
 */
struct UIDataBinding {
    std::string key;
    std::string value; // Simplified string-based binding
};

/**
 * @brief Component that attaches a UI layout or widget to an entity.
 */
struct UIWidgetComponent {
    std::string layoutId;
    bool isVisible = true;
    std::vector<UIDataBinding> bindings;
};

/**
 * @brief Logic for HUD elements like bars (Health, MP).
 */
struct UIProgressBarComponent {
    Fixed32 progress = Fixed32::FromInt(1); // 0-1
    Vector3 color = {Fixed32::FromInt(1), Fixed32::FromInt(0), Fixed32::FromInt(0)};
};

} // namespace urpg
