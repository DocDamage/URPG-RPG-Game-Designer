#include "editor/ai/creator_command_panel.h"

#include <utility>

namespace urpg::editor {

void CreatorCommandPanel::setRequest(urpg::ai::CreatorCommandRequest request) {
    request_ = std::move(request);
}

void CreatorCommandPanel::setProjectData(nlohmann::json projectData) {
    project_data_ = std::move(projectData);
}

void CreatorCommandPanel::setTransportConfig(urpg::ai::CreatorProviderTransportConfig transportConfig) {
    transport_config_ = std::move(transportConfig);
}

void CreatorCommandPanel::render() {
    urpg::ai::CreatorCommandPlanner planner;
    current_plan_ = planner.plan(request_);
    const auto preview = planner.previewDocument(request_, current_plan_);
    const auto validation = urpg::ai::validateCreatorCommandPlan(request_, current_plan_);
    const auto applyPreview = urpg::ai::applyCreatorCommandPlan(request_, current_plan_, project_data_);
    auto dryRunTransport = transport_config_;
    dryRunTransport.execute = false;
    const auto transportPreview = urpg::ai::invokeCreatorProvider(request_, dryRunTransport);

    last_render_snapshot_ = {
        {"prompt", request_.prompt},
        {"selected_tile", {{"x", request_.tile_x}, {"y", request_.tile_y}, {"tile_id", request_.selected_tile_id}}},
        {"plan", current_plan_.toJson()},
        {"validation_diagnostics", validation.size()},
        {"apply_preview", {
            {"would_apply", applyPreview.applied},
            {"diagnostics", applyPreview.diagnostics.size()},
            {"project_patch", applyPreview.project_data},
        }},
        {"provider_request", urpg::ai::buildCreatorProviderRequest(request_)},
        {"provider_transport", transportPreview.toJson()},
        {"preview", {
            {"width", preview.width()},
            {"height", preview.height()},
            {"layer_count", preview.layers().size()},
            {"navigation_diagnostics", preview.validateNavigation().size()},
        }},
    };
}

bool CreatorCommandPanel::applyCurrentPlan() {
    const auto result = urpg::ai::applyCreatorCommandPlan(request_, current_plan_, project_data_);
    project_data_ = result.project_data;
    last_render_snapshot_["last_apply"] = result.toJson();
    return result.applied;
}

const nlohmann::json& CreatorCommandPanel::lastRenderSnapshot() const {
    return last_render_snapshot_;
}

} // namespace urpg::editor
