#pragma once

#include <cstdint>

namespace urpg {

enum class BudgetTier : uint8_t {
    CRITICAL = 0,
    HIGH = 1,
    NORMAL = 2,
    LOW = 3
};

struct FrameBudget {
    uint32_t total_us = 16667;
    uint32_t native_reserve = 8000;
    uint32_t plugin_pool = 6000;
    uint32_t headroom = 2667;
};

struct PluginBudgetDecl {
    BudgetTier tier = BudgetTier::NORMAL;
    uint32_t weight = 1;
};

} // namespace urpg
