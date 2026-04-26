#pragma once

#include "engine/core/math/fixed32.h"
#include <string>
#include <vector>

namespace urpg {

struct InventorySlot {
    std::string itemId;
    int count;
};

struct InventoryComponent {
    std::vector<InventorySlot> slots;
    int maxSlots = 20;

    bool addItem(const std::string& itemId, int count = 1) {
        // Try to stack first
        for (auto& slot : slots) {
            if (slot.itemId == itemId) {
                // In a real engine, we'd check ItemRegistry for maxStack
                slot.count += count;
                return true;
            }
        }

        // Add new slot if space
        if (slots.size() < static_cast<size_t>(maxSlots)) {
            slots.push_back({itemId, count});
            return true;
        }
        return false;
    }

    bool removeItem(const std::string& itemId, int count = 1) {
        for (auto it = slots.begin(); it != slots.end(); ++it) {
            if (it->itemId == itemId) {
                if (it->count >= count) {
                    it->count -= count;
                    if (it->count <= 0) {
                        slots.erase(it);
                    }
                    return true;
                }
            }
        }
        return false;
    }
};

} // namespace urpg
