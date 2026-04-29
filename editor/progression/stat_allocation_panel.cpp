#include "editor/progression/stat_allocation_panel.h"

#include <utility>

namespace urpg::editor {

namespace {

nlohmann::json statsToJson(const urpg::progression::ActorStatBlock& stats) {
    return {{"hp", stats.hp},
            {"mp", stats.mp},
            {"atk", stats.atk},
            {"def", stats.def},
            {"mat", stats.mat},
            {"mdf", stats.mdf},
            {"agi", stats.agi},
            {"luk", stats.luk}};
}

nlohmann::json diagnosticsToJson(const std::vector<urpg::progression::ProgressionDiagnostic>& diagnostics) {
    nlohmann::json rows = nlohmann::json::array();
    for (const auto& diagnostic : diagnostics) {
        rows.push_back({{"code", diagnostic.code}, {"message", diagnostic.message}, {"id", diagnostic.id}});
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
    const std::string actor_id = !post_load_actor_id_.empty()
                                     ? post_load_actor_id_
                                     : (loaded_allocations_.empty() ? std::string{} : loaded_allocations_.front().actor_id);
    const auto application_preview =
        actor_id.empty()
            ? urpg::progression::StatAllocationApplicationPreview{}
            : urpg::progression::buildStatAllocationApplicationPreview(actor_id, current_stats_, loaded_allocations_);
    for (const auto& row : application_preview.rows) {
        post_load_rows.push_back({{"pool_id", row.pool_id},
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
                                  {"diagnostics", diagnosticsToJson(row.diagnostics)}});
    }
    post_load_applicable = application_preview.applicable_count;
    post_load_already_applied = application_preview.already_applied_count;
    post_load_blocked = application_preview.blocked_count;

    snapshot_ = {{"panel", "stat_allocation"},
                 {"pool_id", request_.pool_id},
                 {"spent_points", preview.spent_points},
                 {"remaining_points", preview.remaining_points},
                 {"before", statsToJson(preview.before)},
                 {"after", statsToJson(preview.after)},
                 {"diagnostic_count", preview.diagnostics.size()},
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
