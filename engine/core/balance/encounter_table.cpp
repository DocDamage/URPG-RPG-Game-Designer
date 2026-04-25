#include "engine/core/balance/encounter_table.h"

#include <algorithm>
#include <random>
#include <utility>

namespace urpg::balance {

void EncounterTable::addEncounter(EncounterEntry entry) {
    entries_.push_back(std::move(entry));
}

std::vector<EncounterDiagnostic> EncounterTable::validate() const {
    int32_t total_weight = 0;
    for (const auto& entry : entries_) {
        total_weight += std::max(0, entry.weight);
    }
    if (!entries_.empty() && total_weight == 0) {
        return {{"zero_weight_pool", "Encounter weights must include at least one positive weight."}};
    }
    return {};
}

std::vector<std::string> EncounterTable::preview(const std::string& region_id, uint64_t seed, std::size_t count) const {
    std::vector<EncounterEntry> pool;
    for (const auto& entry : entries_) {
        if (entry.region_id == region_id && entry.weight > 0) {
            pool.push_back(entry);
        }
    }
    std::stable_sort(pool.begin(), pool.end(), [](const auto& lhs, const auto& rhs) { return lhs.enemy_id < rhs.enemy_id; });
    std::vector<std::string> result;
    if (pool.empty()) {
        return result;
    }
    int32_t total_weight = 0;
    for (const auto& entry : pool) {
        total_weight += entry.weight;
    }
    std::mt19937_64 rng(seed);
    for (std::size_t i = 0; i < count; ++i) {
        const auto roll = static_cast<int32_t>(rng() % total_weight);
        int32_t cursor = 0;
        for (const auto& entry : pool) {
            cursor += entry.weight;
            if (roll < cursor) {
                result.push_back(entry.enemy_id);
                break;
            }
        }
    }
    return result;
}

} // namespace urpg::balance
