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

void StatAllocationPanel::render() {
    const auto preview = document_.preview(request_.pool_id, current_stats_, request_);
    snapshot_ = {{"panel", "stat_allocation"},
                 {"pool_id", request_.pool_id},
                 {"spent_points", preview.spent_points},
                 {"remaining_points", preview.remaining_points},
                 {"before", statsToJson(preview.before)},
                 {"after", statsToJson(preview.after)},
                 {"diagnostic_count", preview.diagnostics.size()}};
}

nlohmann::json StatAllocationPanel::lastRenderSnapshot() const {
    return snapshot_;
}

} // namespace urpg::editor
