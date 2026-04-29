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

ProgressionDiagnostic makeDiagnostic(std::string code, std::string message, std::string target) {
    return {std::move(code), std::move(message), std::move(target)};
}

void appendDiagnosticStrings(std::vector<std::string>* diagnostics, const std::vector<std::string>& values) {
    if (diagnostics == nullptr) {
        return;
    }
    diagnostics->insert(diagnostics->end(), values.begin(), values.end());
}

std::vector<std::string> diagnosticStrings(const std::vector<ProgressionDiagnostic>& diagnostics) {
    std::vector<std::string> out;
    out.reserve(diagnostics.size());
    for (const auto& diagnostic : diagnostics) {
        out.push_back(diagnostic.code + ":" + diagnostic.id);
    }
    return out;
}

std::string firstDiagnosticReason(const std::vector<ProgressionDiagnostic>& diagnostics) {
    if (diagnostics.empty()) {
        return {};
    }
    return diagnostics.front().code + ":" + diagnostics.front().id;
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

AppliedStatAllocation StatAllocationDocument::commit(const std::string& pool_id,
                                                     const ActorStatBlock& current_stats,
                                                     const StatAllocationRequest& request) const {
    AppliedStatAllocation allocation;
    allocation.pool_id = pool_id;
    allocation.before = current_stats;
    allocation.points_by_stat = request.points_by_stat;

    const auto* pool = findPool(pool_id);
    if (pool == nullptr) {
        allocation.diagnostics.push_back(
            makeDiagnostic("missing_stat_pool", "Stat allocation pool does not exist.", pool_id));
        return allocation;
    }
    allocation.actor_id = pool->actor_id;
    allocation.class_id = pool->class_id;

    const auto preview_result = preview(pool_id, current_stats, request);
    allocation.after = preview_result.after;
    allocation.spent_points = preview_result.spent_points;
    allocation.remaining_points = preview_result.remaining_points;
    allocation.diagnostics = preview_result.diagnostics;
    allocation.valid = allocation.diagnostics.empty() && allocation.remaining_points >= 0;
    return allocation;
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

nlohmann::json toJson(const ActorStatBlock& stats) {
    return {{"hp", stats.hp},
            {"mp", stats.mp},
            {"atk", stats.atk},
            {"def", stats.def},
            {"mat", stats.mat},
            {"mdf", stats.mdf},
            {"agi", stats.agi},
            {"luk", stats.luk}};
}

ActorStatBlock actorStatBlockFromJson(const nlohmann::json& json) {
    ActorStatBlock stats;
    if (!json.is_object()) {
        return stats;
    }
    stats.hp = json.value("hp", stats.hp);
    stats.mp = json.value("mp", stats.mp);
    stats.atk = json.value("atk", stats.atk);
    stats.def = json.value("def", stats.def);
    stats.mat = json.value("mat", stats.mat);
    stats.mdf = json.value("mdf", stats.mdf);
    stats.agi = json.value("agi", stats.agi);
    stats.luk = json.value("luk", stats.luk);
    return stats;
}

bool actorStatsEqual(const ActorStatBlock& lhs, const ActorStatBlock& rhs) {
    return lhs.hp == rhs.hp && lhs.mp == rhs.mp && lhs.atk == rhs.atk && lhs.def == rhs.def &&
           lhs.mat == rhs.mat && lhs.mdf == rhs.mdf && lhs.agi == rhs.agi && lhs.luk == rhs.luk;
}

nlohmann::json toJson(const AppliedStatAllocation& allocation) {
    nlohmann::json diagnostics = nlohmann::json::array();
    for (const auto& diagnostic : allocation.diagnostics) {
        diagnostics.push_back(
            nlohmann::json{{"code", diagnostic.code}, {"message", diagnostic.message}, {"id", diagnostic.id}});
    }
    return {
        {"schemaVersion", "1.0.0"},
        {"poolId", allocation.pool_id},
        {"actorId", allocation.actor_id},
        {"classId", allocation.class_id},
        {"before", toJson(allocation.before)},
        {"after", toJson(allocation.after)},
        {"pointsByStat", allocation.points_by_stat},
        {"spentPoints", allocation.spent_points},
        {"remainingPoints", allocation.remaining_points},
        {"valid", allocation.valid},
        {"diagnostics", diagnostics},
    };
}

std::optional<AppliedStatAllocation> appliedStatAllocationFromJson(const nlohmann::json& json,
                                                                   std::vector<std::string>* diagnostics) {
    if (!json.is_object()) {
        if (diagnostics != nullptr) {
            diagnostics->push_back("stat_allocation_not_object");
        }
        return std::nullopt;
    }
    if (json.value("schemaVersion", "") != "1.0.0") {
        if (diagnostics != nullptr) {
            diagnostics->push_back("stat_allocation_unsupported_schema");
        }
        return std::nullopt;
    }
    AppliedStatAllocation allocation;
    allocation.pool_id = json.value("poolId", "");
    allocation.actor_id = json.value("actorId", "");
    allocation.class_id = json.value("classId", "");
    allocation.before = actorStatBlockFromJson(json.value("before", nlohmann::json::object()));
    allocation.after = actorStatBlockFromJson(json.value("after", nlohmann::json::object()));
    allocation.spent_points = json.value("spentPoints", 0);
    allocation.remaining_points = json.value("remainingPoints", 0);
    allocation.valid = json.value("valid", false);
    const auto points = json.find("pointsByStat");
    if (points != json.end() && points->is_object()) {
        for (const auto& [stat, value] : points->items()) {
            if (value.is_number_integer()) {
                allocation.points_by_stat[stat] = value.get<int32_t>();
            }
        }
    }
    if (allocation.pool_id.empty() || allocation.actor_id.empty()) {
        if (diagnostics != nullptr) {
            diagnostics->push_back("stat_allocation_missing_identity");
        }
        return std::nullopt;
    }
    return allocation;
}

bool attachStatAllocationToSaveDocument(nlohmann::json& document, const AppliedStatAllocation& allocation,
                                        std::vector<std::string>* diagnostics) {
    if (!document.is_object()) {
        if (diagnostics != nullptr) {
            diagnostics->push_back("save_document_not_object");
        }
        return false;
    }
    if (!allocation.valid) {
        appendDiagnosticStrings(diagnostics, diagnosticStrings(allocation.diagnostics));
        if (diagnostics != nullptr && allocation.diagnostics.empty()) {
            diagnostics->push_back("stat_allocation_invalid");
        }
        return false;
    }
    if (!document.contains(kStatAllocationsSaveKey) || !document[kStatAllocationsSaveKey].is_array()) {
        document[kStatAllocationsSaveKey] = nlohmann::json::array();
    }
    document[kStatAllocationsSaveKey].push_back(toJson(allocation));
    return true;
}

std::vector<AppliedStatAllocation> loadStatAllocationsFromSaveDocument(
    const nlohmann::json& document, std::vector<std::string>* diagnostics) {
    std::vector<AppliedStatAllocation> allocations;
    if (!document.is_object() || !document.contains(kStatAllocationsSaveKey)) {
        return allocations;
    }
    const auto& root = document[kStatAllocationsSaveKey];
    if (!root.is_array()) {
        if (diagnostics != nullptr) {
            diagnostics->push_back("stat_allocations_not_array");
        }
        return allocations;
    }
    for (const auto& item : root) {
        auto allocation = appliedStatAllocationFromJson(item, diagnostics);
        if (allocation.has_value()) {
            allocations.push_back(std::move(*allocation));
        }
    }
    return allocations;
}

StatAllocationApplicationPreview buildStatAllocationApplicationPreview(
    const std::string& actor_id,
    const ActorStatBlock& current_stats,
    const std::vector<AppliedStatAllocation>& allocations) {
    StatAllocationApplicationPreview preview;
    preview.actor_id = actor_id;
    preview.current = current_stats;

    for (const auto& allocation : allocations) {
        if (allocation.actor_id != actor_id) {
            continue;
        }

        StatAllocationApplicationRow row;
        row.pool_id = allocation.pool_id;
        row.actor_id = allocation.actor_id;
        row.class_id = allocation.class_id;
        row.current = current_stats;
        row.saved_after = allocation.after;
        row.points_by_stat = allocation.points_by_stat;
        row.spent_points = allocation.spent_points;
        row.remaining_points = allocation.remaining_points;
        row.valid_record = allocation.valid;
        row.diagnostics = allocation.diagnostics;
        row.already_applied = actorStatsEqual(current_stats, allocation.after);

        if (!row.valid_record) {
            row.blocked_reason = firstDiagnosticReason(row.diagnostics);
            if (row.blocked_reason.empty()) {
                row.blocked_reason = "invalid_stat_allocation_record";
            }
        } else if (row.already_applied) {
            row.blocked_reason = "already_applied";
        } else {
            row.can_apply = true;
        }

        if (row.can_apply) {
            ++preview.applicable_count;
        } else if (row.already_applied) {
            ++preview.already_applied_count;
        } else {
            ++preview.blocked_count;
        }
        preview.rows.push_back(std::move(row));
    }

    preview.row_count = preview.rows.size();
    return preview;
}

std::optional<ActorStatBlock> applyLatestStatAllocationForActor(
    const std::string& actor_id,
    const ActorStatBlock& current_stats,
    const std::vector<AppliedStatAllocation>& allocations,
    std::vector<std::string>* diagnostics) {
    const auto preview = buildStatAllocationApplicationPreview(actor_id, current_stats, allocations);
    for (auto it = preview.rows.rbegin(); it != preview.rows.rend(); ++it) {
        if (it->can_apply) {
            return it->saved_after;
        }
    }

    if (diagnostics != nullptr) {
        if (preview.rows.empty()) {
            diagnostics->push_back("stat_allocation_no_actor_records");
        } else if (preview.already_applied_count == preview.row_count) {
            diagnostics->push_back("stat_allocation_already_applied");
        } else {
            diagnostics->push_back("stat_allocation_no_applicable_records");
        }
    }
    return std::nullopt;
}

} // namespace urpg::progression
