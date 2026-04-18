#pragma once

#include "engine/core/math/fixed32.h"
#include <string>

namespace urpg {

/**
 * @brief Component used to trigger scripts or events when an entity enters a volume.
 */
struct TriggerVolumeComponent {
    std::string onEnterEvent;
    std::string onExitEvent;
    bool isOneShot = false;
    bool isActive = true;
};

/**
 * @brief State tracking for active triggers to detect exit events.
 */
struct TriggerStateComponent {
    bool wasOccupiedLastFrame = false;
    bool isCurrentlyOccupied = false;
};

} // namespace urpg
