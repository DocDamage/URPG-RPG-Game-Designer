#include "engine/core/items/loot_affix_generator.h"

#include <algorithm>
#include <random>
#include <utility>

namespace urpg::items {

void LootAffixGenerator::addAffix(LootAffix affix) {
    affixes_.push_back(std::move(affix));
}

std::optional<LootAffix> LootAffixGenerator::roll(const std::string& rarity, uint64_t seed) const {
    std::vector<LootAffix> pool;
    for (const auto& affix : affixes_) {
        if (affix.rarity == rarity) {
            pool.push_back(affix);
        }
    }
    std::stable_sort(pool.begin(), pool.end(), [](const auto& lhs, const auto& rhs) { return lhs.id < rhs.id; });
    if (pool.empty()) {
        return std::nullopt;
    }
    std::mt19937_64 rng(seed);
    auto selected = pool[static_cast<std::size_t>(rng() % pool.size())];
    const auto span = std::max(1, selected.max_value - selected.min_value + 1);
    selected.value = selected.min_value + static_cast<int32_t>(rng() % span);
    return selected;
}

std::vector<LootAffixDiagnostic> LootAffixGenerator::validate() const {
    std::vector<LootAffixDiagnostic> diagnostics;
    for (const auto& affix : affixes_) {
        if (affix.min_value > affix.max_value) {
            diagnostics.push_back({"invalid_affix_bounds", "Affix minimum exceeds maximum."});
        }
        if (affix.economy_multiplier <= 0) {
            diagnostics.push_back({"invalid_affix_economy", "Affix economy multiplier must be positive."});
        }
    }
    return diagnostics;
}

} // namespace urpg::items
