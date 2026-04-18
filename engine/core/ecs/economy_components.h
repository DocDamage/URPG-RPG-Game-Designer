#pragma once

#include "engine/core/math/fixed32.h"
#include <string>

namespace urpg {

/**
 * @brief Component used to mark an entity as a "Merchant" with a store inventory.
 */
struct MerchantComponent {
    std::string shopId;
    float priceMultiplier = 1.0f;
};

/**
 * @brief Component to mark an entity as "Currency" (e.g., Gold).
 */
struct CurrencyComponent {
    uint32_t amount = 0;
};

} // namespace urpg
