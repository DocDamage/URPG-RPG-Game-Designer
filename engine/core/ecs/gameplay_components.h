#pragma once

#include "engine/core/gameplay/inventory_components.h"
#include "engine/core/math/fixed32.h"
#include <string>

namespace urpg {

/**
 * @brief Component that provides a light-weight tag for categorization or logic.
 */
struct TagComponent {
    std::string tag;
};

/**
 * @brief Component for entities that can be "Interacted" with by the player.
 */
struct InteractionComponent {
    std::string interactionEvent;
    Fixed32 interactionRadius = Fixed32::FromInt(2);
    bool requiresFacing = true;
};

} // namespace urpg
