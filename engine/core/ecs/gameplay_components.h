#pragma once

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
 * @brief Component for an entity that acts as an "Inventory" or "Item Container".
 */
struct InventoryComponent {
    uint32_t capacity = 20;
    std::vector<std::string> itemIds;
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
