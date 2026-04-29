#pragma once

#include "engine/core/progression/class_progression.h"

#include <cstdint>
#include <map>
#include <nlohmann/json.hpp>
#include <optional>
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

inline constexpr const char* kStatAllocationsSaveKey = "_stat_allocations";

struct AppliedStatAllocation {
    std::string pool_id;
    std::string actor_id;
    std::string class_id;
    ActorStatBlock before;
    ActorStatBlock after;
    std::map<std::string, int32_t> points_by_stat;
    int32_t spent_points = 0;
    int32_t remaining_points = 0;
    bool valid = false;
    std::vector<ProgressionDiagnostic> diagnostics;
};

struct StatAllocationApplicationRow {
    std::string pool_id;
    std::string actor_id;
    std::string class_id;
    ActorStatBlock current;
    ActorStatBlock saved_after;
    std::map<std::string, int32_t> points_by_stat;
    int32_t spent_points = 0;
    int32_t remaining_points = 0;
    bool valid_record = false;
    bool already_applied = false;
    bool can_apply = false;
    std::string blocked_reason;
    std::vector<ProgressionDiagnostic> diagnostics;
};

struct StatAllocationApplicationPreview {
    std::string actor_id;
    ActorStatBlock current;
    size_t row_count = 0;
    size_t applicable_count = 0;
    size_t already_applied_count = 0;
    size_t blocked_count = 0;
    std::vector<StatAllocationApplicationRow> rows;
};

class StatAllocationDocument {
public:
    void addPool(StatAllocationPool pool);
    [[nodiscard]] std::vector<ProgressionDiagnostic> validate() const;
    [[nodiscard]] StatAllocationPreview preview(const std::string& pool_id,
                                                const ActorStatBlock& current_stats,
                                                const StatAllocationRequest& request) const;
    [[nodiscard]] AppliedStatAllocation commit(const std::string& pool_id,
                                               const ActorStatBlock& current_stats,
                                               const StatAllocationRequest& request) const;

private:
    [[nodiscard]] const StatAllocationPool* findPool(const std::string& pool_id) const;
    [[nodiscard]] const StatAllocationRule* findRule(const StatAllocationPool& pool, const std::string& stat_id) const;

    std::map<std::string, StatAllocationPool> pools_;
};

nlohmann::json toJson(const ActorStatBlock& stats);
ActorStatBlock actorStatBlockFromJson(const nlohmann::json& json);
bool actorStatsEqual(const ActorStatBlock& lhs, const ActorStatBlock& rhs);
nlohmann::json toJson(const AppliedStatAllocation& allocation);
std::optional<AppliedStatAllocation> appliedStatAllocationFromJson(const nlohmann::json& json,
                                                                   std::vector<std::string>* diagnostics = nullptr);
bool attachStatAllocationToSaveDocument(nlohmann::json& document, const AppliedStatAllocation& allocation,
                                        std::vector<std::string>* diagnostics = nullptr);
std::vector<AppliedStatAllocation> loadStatAllocationsFromSaveDocument(
    const nlohmann::json& document, std::vector<std::string>* diagnostics = nullptr);
StatAllocationApplicationPreview buildStatAllocationApplicationPreview(
    const std::string& actor_id,
    const ActorStatBlock& current_stats,
    const std::vector<AppliedStatAllocation>& allocations);
std::optional<ActorStatBlock> applyLatestStatAllocationForActor(
    const std::string& actor_id,
    const ActorStatBlock& current_stats,
    const std::vector<AppliedStatAllocation>& allocations,
    std::vector<std::string>* diagnostics = nullptr);

} // namespace urpg::progression
