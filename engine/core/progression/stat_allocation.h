#pragma once

#include "engine/core/progression/class_progression.h"

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace urpg::progression {

struct ActorStatBlock {
    int32_t hp = 1;
    int32_t mp = 0;
    int32_t atk = 1;
    int32_t def = 1;
    int32_t mat = 1;
    int32_t mdf = 1;
    int32_t agi = 1;
    int32_t luk = 1;
};

struct StatAllocationRule {
    std::string stat_id;
    int32_t point_cost = 1;
    int32_t stat_gain = 1;
    int32_t cap = 999;
};

struct StatAllocationPool {
    std::string id;
    std::string actor_id;
    std::string class_id;
    int32_t points_per_level = 0;
    int32_t unspent_points = 0;
    std::vector<StatAllocationRule> rules;
};

struct StatAllocationRequest {
    std::string pool_id;
    std::map<std::string, int32_t> points_by_stat;
};

struct StatAllocationPreview {
    ActorStatBlock before;
    ActorStatBlock after;
    int32_t spent_points = 0;
    int32_t remaining_points = 0;
    std::vector<ProgressionDiagnostic> diagnostics;
};

class StatAllocationDocument {
public:
    void addPool(StatAllocationPool pool);
    [[nodiscard]] std::vector<ProgressionDiagnostic> validate() const;
    [[nodiscard]] StatAllocationPreview preview(const std::string& pool_id,
                                                const ActorStatBlock& current_stats,
                                                const StatAllocationRequest& request) const;

private:
    [[nodiscard]] const StatAllocationPool* findPool(const std::string& pool_id) const;
    [[nodiscard]] const StatAllocationRule* findRule(const StatAllocationPool& pool, const std::string& stat_id) const;

    std::map<std::string, StatAllocationPool> pools_;
};

} // namespace urpg::progression
