#include "editor/ai/creator_command_panel.h"

#include "editor/spatial/spatial_authoring_workspace.h"

#include <algorithm>
#include <utility>

namespace urpg::editor {

void CreatorCommandPanel::setRequest(urpg::ai::CreatorCommandRequest request) {
    request_ = std::move(request);
    reviewed_document_revision_.clear();
}

void CreatorCommandPanel::setTransportConfig(urpg::ai::CreatorProviderTransportConfig transportConfig) {
    transport_config_ = std::move(transportConfig);
}

void CreatorCommandPanel::setMapWorkspace(SpatialAuthoringWorkspace* workspace) {
    map_workspace_ = workspace;
    reviewed_document_revision_.clear();
}

void CreatorCommandPanel::setTilePaletteBindings(std::vector<TilePaletteBinding> bindings) {
    tile_palette_bindings_ = std::move(bindings);
    reviewed_document_revision_.clear();
}

void CreatorCommandPanel::render() {
    urpg::ai::CreatorCommandPlanner planner;
    current_plan_ = planner.plan(request_);
    reviewed_document_revision_.clear();
    const auto preview = planner.previewDocument(request_, current_plan_);
    const auto validation = urpg::ai::validateCreatorCommandPlan(request_, current_plan_);
    auto dryRunTransport = transport_config_;
    dryRunTransport.execute = false;
    const auto transportPreview = urpg::ai::invokeCreatorProvider(request_, dryRunTransport);

    nlohmann::json apply_preview = {
        {"would_apply", false},
        {"owner", "perspective_2d"},
        {"domain", "tiles"},
        {"code", "creator_native_map_unbound"},
        {"message", "Open a Perspective 2D Map and bind reviewed tile palette entries before applying."},
    };
    if (!validation.empty()) {
        apply_preview["code"] = "creator_plan_validation_failed";
        apply_preview["message"] = "The creator plan has validation diagnostics and cannot be applied.";
    } else if (!current_plan_.prop_edits.empty() || !current_plan_.logic_edits.empty()) {
        apply_preview["code"] = "creator_native_domain_not_available";
        apply_preview["message"] = "This plan also changes props or event logic. Those native domains require their own reviewed commands.";
    } else if (map_workspace_ != nullptr &&
               (request_.map_id.empty() || request_.map_id != map_workspace_->activePerspectiveMapId())) {
        apply_preview["code"] = "creator_native_map_context_mismatch";
        apply_preview["message"] = "Open the Map selected by this creator plan before applying it.";
    } else if (map_workspace_ != nullptr) {
        const bool all_tiles_bound = std::all_of(
            current_plan_.tile_edits.begin(), current_plan_.tile_edits.end(), [&](const urpg::ai::CreatorTileEdit& edit) {
                return std::any_of(tile_palette_bindings_.begin(), tile_palette_bindings_.end(),
                                   [&](const TilePaletteBinding& binding) {
                                       return binding.planned_tile_id == edit.tile_id &&
                                              binding.planned_layer_id == edit.layer_id &&
                                              !binding.layer_id.empty() && !binding.tileset_id.empty() && !binding.tile_id.empty();
                                   });
            });
        apply_preview["would_apply"] = all_tiles_bound;
        apply_preview["code"] = all_tiles_bound ? "creator_native_tile_command_ready"
                                                  : "creator_native_tile_bindings_missing";
        apply_preview["message"] = all_tiles_bound
                                        ? "The reviewed tile-only plan can be applied to the active native Map as one undoable command."
                                        : "Map tile bindings are missing for one or more planned layer and tile IDs.";
        reviewed_document_revision_ = map_workspace_->perspectiveDocumentRevision();
        apply_preview["source_revision"] = reviewed_document_revision_;
    }

    last_render_snapshot_ = {
        {"prompt", request_.prompt},
        {"selected_tile", {{"x", request_.tile_x}, {"y", request_.tile_y}, {"tile_id", request_.selected_tile_id}}},
        {"plan", current_plan_.toJson()},
        {"validation_diagnostics", validation.size()},
        {"apply_preview", std::move(apply_preview)},
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
    const auto validation = urpg::ai::validateCreatorCommandPlan(request_, current_plan_);
    if (!validation.empty()) {
        last_render_snapshot_["last_apply"] = {
            {"applied", false},
            {"code", "creator_plan_validation_failed"},
            {"diagnostics", validation.size()},
        };
        return false;
    }
    if (!current_plan_.prop_edits.empty() || !current_plan_.logic_edits.empty()) {
        last_render_snapshot_["last_apply"] = {
            {"applied", false},
            {"code", "creator_native_domain_not_available"},
            {"message", "This plan includes props or event logic, which are not part of the reviewed native tile command."},
        };
        return false;
    }
    if (map_workspace_ == nullptr) {
        last_render_snapshot_["last_apply"] = {
            {"applied", false},
            {"code", "creator_native_map_unbound"},
            {"message", "Open and bind a Perspective 2D Map before applying a creator tile command."},
        };
        return false;
    }
    if (request_.map_id.empty() || request_.map_id != map_workspace_->activePerspectiveMapId()) {
        last_render_snapshot_["last_apply"] = {
            {"applied", false},
            {"code", "creator_native_map_context_mismatch"},
            {"message", "Open the Map selected by this creator plan before applying it."},
        };
        return false;
    }
    if (reviewed_document_revision_.empty()) {
        last_render_snapshot_["last_apply"] = {
            {"applied", false},
            {"code", "creator_native_plan_not_reviewed"},
            {"message", "Render and review the active Map plan before applying it."},
        };
        return false;
    }

    std::vector<SpatialAuthoringWorkspace::Perspective2DNativeTileEdit> native_edits;
    native_edits.reserve(current_plan_.tile_edits.size());
    for (const auto& edit : current_plan_.tile_edits) {
        const auto binding = std::find_if(tile_palette_bindings_.begin(), tile_palette_bindings_.end(),
                                          [&](const TilePaletteBinding& candidate) {
                                              return candidate.planned_tile_id == edit.tile_id &&
                                                     candidate.planned_layer_id == edit.layer_id;
                                          });
        if (binding == tile_palette_bindings_.end() || binding->layer_id.empty() || binding->tileset_id.empty() ||
            binding->tile_id.empty()) {
            last_render_snapshot_["last_apply"] = {
                {"applied", false},
                {"code", "creator_native_tile_bindings_missing"},
                {"message", "Resolve every planned layer and tile ID to an active Map palette entry before applying."},
            };
            return false;
        }
        native_edits.push_back({binding->layer_id, binding->tileset_id, binding->tile_id, edit.x, edit.y});
    }

    const auto result = map_workspace_->applyNativeTileEdits(reviewed_document_revision_, native_edits);
    last_render_snapshot_["last_apply"] = {
        {"applied", result.success},
        {"code", result.code},
        {"message", result.message},
        {"document_revision", result.document_revision},
        {"applied_tile_count", result.applied_tile_count},
    };
    return result.success;
}

const nlohmann::json& CreatorCommandPanel::lastRenderSnapshot() const {
    return last_render_snapshot_;
}

} // namespace urpg::editor
