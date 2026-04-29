#include "engine/core/progression/stat_allocation.h"

#include <algorithm>
#include <utility>

namespace urpg::progression {

namespace {

int32_t& statRef(ActorStatBlock& stats, const std::string& stat_id) {
    if (stat_id == "hp") {
        return stats.hp;
    }
    if (stat_id == "mp") {
        return stats.mp;
    }
    if (stat_id == "atk") {
        return stats.atk;
    }
    if (stat_id == "def") {
        return stats.def;
    }
    if (stat_id == "mat") {
        return stats.mat;
    }
    if (stat_id == "mdf") {
        return stats.mdf;
    }
    if (stat_id == "agi") {
        return stats.agi;
    }
    return stats.luk;
}

bool knownStat(const std::string& stat_id) {
    static const std::set<std::string> kStats{"hp", "mp", "atk", "def", "mat", "mdf", "agi", "luk"};
    return kStats.contains(stat_id);
}

} // namespace

void StatAllocationDocument::addPool(StatAllocationPool pool) {
    pools_[pool.id] = std::move(pool);
}

std::vector<ProgressionDiagnostic> StatAllocationDocument::validate() const {
    std::vector<ProgressionDiagnostic> diagnostics;
    for (const auto& [id, pool] : pools_) {
        if (id.empty() || pool.actor_id.empty()) {
            diagnostics.push_back({"invalid_stat_pool", "Stat allocation pool needs an id and actor id.", id});
        }
        if (pool.points_per_level < 0 || pool.unspent_points < 0) {
            diagnostics.push_back({"invalid_stat_points", "Stat allocation points cannot be negative.", id});
        }
        std::set<std::string> seen_stats;
        for (const auto& rule : pool.rules) {
            if (!knownStat(rule.stat_id)) {
                diagnostics.push_back({"unknown_stat_id", "Stat allocation rule references an unknown stat.", id});
            }
            if (!seen_stats.insert(rule.stat_id).second) {
                diagnostics.push_back({"duplicate_stat_rule", "Stat allocation pool has duplicate stat rules.", id});
            }
            if (rule.point_cost <= 0 || rule.stat_gain <= 0 || rule.cap < 0) {
                diagnostics.push_back({"invalid_stat_rule", "Stat allocation rules need positive cost/gain and non-negative cap.", id});
            }
        }
    }
    return diagnostics;
}

StatAllocationPreview StatAllocationDocument::preview(const std::string& pool_id,
                                                      const ActorStatBlock& current_stats,
                                                      const StatAllocationRequest& request) const {
    StatAllocationPreview preview;
    preview.before = current_stats;
    preview.after = current_stats;

    const auto* pool = findPool(pool_id);
    if (pool == nullptr) {
        preview.diagnostics.push_back({"missing_stat_pool", "Stat allocation pool does not exist.", pool_id});
        return preview;
    }

    int32_t spent = 0;
    for (const auto& [stat_id, points] : request.points_by_stat) {
        if (points <= 0) {
            preview.diagnostics.push_back({"invalid_allocated_points", "Allocated stat points must be positive.", stat_id});
            continue;
        }
        const auto* rule = findRule(*pool, stat_id);
        if (rule == nullptr) {
            preview.diagnostics.push_back({"missing_stat_rule", "No stat allocation rule exists for requested stat.", stat_id});
            continue;
        }

        const int32_t cost = points * rule->point_cost;
        spent += cost;
        int32_t& stat = statRef(preview.after, stat_id);
        stat = std::min(rule->cap, stat + (points * rule->stat_gain));
    }

    preview.spent_points = spent;
    preview.remaining_points = pool->unspent_points - spent;
    if (preview.remaining_points < 0) {
        preview.diagnostics.push_back({"stat_points_overspent", "Stat allocation request spends more points than available.", pool_id});
    }
    return preview;
}

const StatAllocationPool* StatAllocationDocument::findPool(const std::string& pool_id) const {
    const auto it = pools_.find(pool_id);
    return it == pools_.end() ? nullptr : &it->second;
}

const StatAllocationRule* StatAllocationDocument::findRule(const StatAllocationPool& pool,
                                                           const std::string& stat_id) const {
    const auto it = std::find_if(pool.rules.begin(), pool.rules.end(),
                                 [&](const StatAllocationRule& rule) { return rule.stat_id == stat_id; });
    return it == pool.rules.end() ? nullptr : &*it;
}

} // namespace urpg::progression
