#include "editor/progression/stat_allocation_panel.h"

#include <utility>

namespace urpg::editor {

namespace {

nlohmann::json statsToJson(const urpg::progression::ActorStatBlock& stats) {
    return {{"hp", stats.hp},   {"mp", stats.mp},   {"atk", stats.atk}, {"def", stats.def},
            {"mat", stats.mat}, {"mdf", stats.mdf}, {"agi", stats.agi}, {"luk", stats.luk}};
}

nlohmann::json diagnosticsToJson(const std::vector<urpg::progression::ProgressionDiagnostic>& diagnostics) {
    nlohmann::json rows = nlohmann::json::array();
    for (const auto& diagnostic : diagnostics) {
        rows.push_back({{"code", diagnostic.code}, {"message", diagnostic.message}, {"id", diagnostic.id}});
    }
    return rows;
}

int statValue(const urpg::progression::ActorStatBlock& stats, const std::string& stat_id) {
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
    if (stat_id == "luk") {
        return stats.luk;
    }
    return 0;
}

nlohmann::json buildPoolRows(const urpg::progression::StatAllocationDocument& document,
                             const std::string& selected_pool_id) {
    nlohmann::json rows = nlohmann::json::array();
    for (const auto& [id, pool] : document.pools()) {
        rows.push_back({{"pool_id", id},
                        {"actor_id", pool.actor_id},
                        {"class_id", pool.class_id},
                        {"points_per_level", pool.points_per_level},
                        {"unspent_points", pool.unspent_points},
                        {"selected", id == selected_pool_id},
                        {"rule_count", pool.rules.size()}});
    }
    return rows;
}

nlohmann::json buildStatControlRows(const urpg::progression::StatAllocationDocument& document,
                                    const urpg::progression::StatAllocationRequest& request,
                                    const urpg::progression::ActorStatBlock& before,
                                    const urpg::progression::ActorStatBlock& after, int remaining_points) {
    const auto pools = document.pools();
    const auto pool_it = pools.find(request.pool_id);
    if (pool_it == pools.end()) {
        return nlohmann::json::array();
    }

    nlohmann::json rows = nlohmann::json::array();
    for (const auto& rule : pool_it->second.rules) {
        const int allocated =
            request.points_by_stat.count(rule.stat_id) == 0 ? 0 : request.points_by_stat.at(rule.stat_id);
        const int before_value = statValue(before, rule.stat_id);
        const int after_value = statValue(after, rule.stat_id);
        const bool at_cap = after_value >= rule.cap;
        const bool can_increment = remaining_points >= rule.point_cost && !at_cap;
        const bool can_decrement = allocated > 0;
        const std::string increment_disabled_reason =
            at_cap ? "stat_cap_reached" : (remaining_points < rule.point_cost ? "not_enough_points" : "");
        rows.push_back(
            {{"stat_id", rule.stat_id},
             {"before", before_value},
             {"after", after_value},
             {"allocated_points", allocated},
             {"point_cost", rule.point_cost},
             {"stat_gain", rule.stat_gain},
             {"cap", rule.cap},
             {"at_cap", at_cap},
             {"cap_warning", at_cap ? "Stat is at or above its cap." : ""},
             {"increment_button",
              {{"visible", true},
               {"enabled", can_increment},
               {"action", "increment_stat"},
               {"stat_id", rule.stat_id},
               {"disabled_reason",
                can_increment ? nlohmann::json(nullptr) : nlohmann::json(increment_disabled_reason)}}},
             {"decrement_button",
              {{"visible", true},
               {"enabled", can_decrement},
               {"action", "decrement_stat"},
               {"stat_id", rule.stat_id},
               {"disabled_reason", can_decrement ? nlohmann::json(nullptr) : nlohmann::json("no_points_allocated")}}}});
    }
    return rows;
}

} // namespace

void StatAllocationPanel::bindDocument(urpg::progression::StatAllocationDocument document) {
    document_ = std::move(document);
}

void StatAllocationPanel::setCurrentStats(urpg::progression::ActorStatBlock stats) {
    current_stats_ = stats;
}

void StatAllocationPanel::setRequest(urpg::progression::StatAllocationRequest request) {
    request_ = std::move(request);
}

void StatAllocationPanel::setPostLoadActorId(std::string actor_id) {
    post_load_actor_id_ = std::move(actor_id);
}

void StatAllocationPanel::setLoadedAllocations(std::vector<urpg::progression::AppliedStatAllocation> allocations) {
    loaded_allocations_ = std::move(allocations);
}

void StatAllocationPanel::render() {
    const auto preview = document_.preview(request_.pool_id, current_stats_, request_);

    nlohmann::json post_load_rows = nlohmann::json::array();
    size_t post_load_applicable = 0;
    size_t post_load_already_applied = 0;
    size_t post_load_blocked = 0;
    const std::string actor_id =
        !post_load_actor_id_.empty()
            ? post_load_actor_id_
            : (loaded_allocations_.empty() ? std::string{} : loaded_allocations_.front().actor_id);
    const auto application_preview =
        actor_id.empty()
            ? urpg::progression::StatAllocationApplicationPreview{}
            : urpg::progression::buildStatAllocationApplicationPreview(actor_id, current_stats_, loaded_allocations_);
    for (const auto& row : application_preview.rows) {
        post_load_rows.push_back(
            {{"pool_id", row.pool_id},
             {"actor_id", row.actor_id},
             {"class_id", row.class_id},
             {"current", statsToJson(row.current)},
             {"saved_after", statsToJson(row.saved_after)},
             {"points_by_stat", row.points_by_stat},
             {"spent_points", row.spent_points},
             {"remaining_points", row.remaining_points},
             {"valid_record", row.valid_record},
             {"already_applied", row.already_applied},
             {"can_apply", row.can_apply},
             {"blocked_reason", row.blocked_reason},
             {"diagnostics", diagnosticsToJson(row.diagnostics)},
             {"apply_button",
              {{"visible", true},
               {"enabled", row.can_apply},
               {"action", "apply_loaded_stat_allocation"},
               {"disabled_reason",
                row.can_apply ? nlohmann::json(nullptr)
                              : nlohmann::json(row.blocked_reason.empty() ? "blocked" : row.blocked_reason)}}}});
    }
    post_load_applicable = application_preview.applicable_count;
    post_load_already_applied = application_preview.already_applied_count;
    post_load_blocked = application_preview.blocked_count;

    snapshot_ = {
        {"panel", "stat_allocation"},
        {"pool_id", request_.pool_id},
        {"pool_rows", buildPoolRows(document_, request_.pool_id)},
        {"selected_pool",
         document_.pools().count(request_.pool_id) == 0
             ? nlohmann::json(nullptr)
             : nlohmann::json({{"pool_id", request_.pool_id},
                               {"actor_id", document_.pools().at(request_.pool_id).actor_id},
                               {"class_id", document_.pools().at(request_.pool_id).class_id},
                               {"unspent_points", document_.pools().at(request_.pool_id).unspent_points}})},
        {"spent_points", preview.spent_points},
        {"remaining_points", preview.remaining_points},
        {"before", statsToJson(preview.before)},
        {"after", statsToJson(preview.after)},
        {"diagnostic_count", preview.diagnostics.size()},
        {"diagnostics", diagnosticsToJson(preview.diagnostics)},
        {"stat_controls",
         buildStatControlRows(document_, request_, preview.before, preview.after, preview.remaining_points)},
        {"controls",
         {{"commit_button",
           {{"visible", true},
            {"enabled", preview.diagnostics.empty()},
            {"action", "commit_stat_allocation"},
            {"disabled_reason", preview.diagnostics.empty() ? nlohmann::json(nullptr)
                                                            : nlohmann::json(preview.diagnostics.front().code)}}}}},
        {"post_load",
         {{"actor_id", actor_id},
          {"row_count", application_preview.row_count},
          {"applicable_count", post_load_applicable},
          {"already_applied_count", post_load_already_applied},
          {"blocked_count", post_load_blocked},
          {"rows", post_load_rows}}}};
}

nlohmann::json StatAllocationPanel::lastRenderSnapshot() const {
    return snapshot_;
}

} // namespace urpg::editor
