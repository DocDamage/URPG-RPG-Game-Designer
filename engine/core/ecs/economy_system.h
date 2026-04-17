#pragma once

#include "engine/core/ecs/world.h"
#include "engine/core/ecs/economy_components.h"
#include "engine/core/gameplay/inventory_components.h"

namespace urpg {

/**
 * @brief System that handles buying/selling from merchants.
 */
class EconomySystem {
public:
    struct TransactionResult {
        bool success;
        std::string message;
    };

    void update(World&) {
        // Merchant economy is currently transaction-driven; no per-frame step yet.
    }

    TransactionResult buyItem(World& world, EntityID buyer, EntityID merchant, const std::string& itemId, uint32_t price) {
        auto* buyerCurrency = world.GetComponent<CurrencyComponent>(buyer);
        auto* buyerInventory = world.GetComponent<InventoryComponent>(buyer);
        auto* merchantData = world.GetComponent<MerchantComponent>(merchant);

        if (!buyerCurrency || buyerCurrency->amount < price) {
            return {false, "Not enough gold"};
        }

        if (!buyerInventory || buyerInventory->slots.size() >= static_cast<size_t>(buyerInventory->maxSlots)) {
            return {false, "Inventory full"};
        }

        buyerCurrency->amount -= price;
        buyerInventory->addItem(itemId);
        
        return {true, "Purchase successful"};
    }
};

} // namespace urpg
