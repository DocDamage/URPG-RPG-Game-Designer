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

void CreatorCommandPanel::setPropAssetBindings(std::vector<PropAssetBinding> bindings) {
    prop_asset_bindings_ = std::move(bindings);
    reviewed_document_revision_.clear();
}

void CreatorCommandPanel::setEventLayerBindings(std::vector<EventLayerBinding> bindings) {
    event_layer_bindings_ = std::move(bindings);
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
    const bool is_event_message_plan = current_plan_.intent == "place_event_message" &&
                                       current_plan_.tile_edits.empty() && current_plan_.prop_edits.empty() &&
                                       !current_plan_.logic_edits.empty() &&
                                       std::all_of(current_plan_.logic_edits.begin(), current_plan_.logic_edits.end(),
                                                   [](const urpg::ai::CreatorLogicEdit& edit) {
                                                       return edit.kind == "message" && edit.trigger == "confirm_interact" &&
                                                              edit.payload.is_object() && edit.payload.contains("layer_id") &&
                                                              edit.payload.contains("label") && edit.payload.contains("text") &&
                                                              edit.payload["layer_id"].is_string() && edit.payload["label"].is_string() &&
                                                              edit.payload["text"].is_string();
                                                   });

    nlohmann::json apply_preview = {
        {"would_apply", false},
        {"owner", "perspective_2d"},
        {"domain", "unresolved"},
        {"code", "creator_native_map_unbound"},
        {"message", "Open a Perspective 2D Map and bind reviewed native palette entries before applying."},
    };
    if (!validation.empty()) {
        apply_preview["code"] = "creator_plan_validation_failed";
        apply_preview["message"] = "The creator plan has validation diagnostics and cannot be applied.";
    } else if (!current_plan_.logic_edits.empty() && !is_event_message_plan) {
        apply_preview["code"] = "creator_native_domain_not_available";
        apply_preview["message"] = "This plan changes event logic, which requires its own reviewed native command.";
    } else if (!current_plan_.tile_edits.empty() && !current_plan_.prop_edits.empty()) {
        apply_preview["code"] = "creator_native_multi_domain_not_available";
        apply_preview["message"] = "Review tile and prop edits as separate native commands; mixed creator plans remain unavailable.";
    } else if (map_workspace_ != nullptr &&
               (request_.map_id.empty() || request_.map_id != map_workspace_->activePerspectiveMapId())) {
        apply_preview["code"] = "creator_native_map_context_mismatch";
        apply_preview["message"] = "Open the Map selected by this creator plan before applying it.";
    } else if (map_workspace_ != nullptr) {
        if (!current_plan_.tile_edits.empty()) {
            const bool all_tiles_bound = std::all_of(
                current_plan_.tile_edits.begin(), current_plan_.tile_edits.end(), [&](const urpg::ai::CreatorTileEdit& edit) {
                    return std::any_of(tile_palette_bindings_.begin(), tile_palette_bindings_.end(),
                                       [&](const TilePaletteBinding& binding) {
                                           return binding.planned_tile_id == edit.tile_id &&
                                                  binding.planned_layer_id == edit.layer_id && !binding.layer_id.empty() &&
                                                  !binding.tileset_id.empty() && !binding.tile_id.empty();
                                       });
                });
            apply_preview["domain"] = "tiles";
            apply_preview["would_apply"] = all_tiles_bound;
            apply_preview["code"] = all_tiles_bound ? "creator_native_tile_command_ready"
                                                      : "creator_native_tile_bindings_missing";
            apply_preview["message"] = all_tiles_bound
                                            ? "The reviewed tile-only plan can be applied to the active native Map as one undoable command."
                                            : "Map tile bindings are missing for one or more planned layer and tile IDs.";
        } else if (!current_plan_.prop_edits.empty()) {
            const bool all_props_bound = std::all_of(
                current_plan_.prop_edits.begin(), current_plan_.prop_edits.end(), [&](const urpg::ai::CreatorPropEdit& edit) {
                    return std::any_of(prop_asset_bindings_.begin(), prop_asset_bindings_.end(),
                                       [&](const PropAssetBinding& binding) {
                                           return binding.planned_asset_id == edit.asset_id && !binding.asset_id.empty() &&
                                                  !binding.project_path.empty();
                                       });
                });
            apply_preview["domain"] = "props";
            apply_preview["would_apply"] = all_props_bound;
            apply_preview["code"] = all_props_bound ? "creator_native_prop_command_ready"
                                                      : "creator_native_prop_bindings_missing";
            apply_preview["message"] = all_props_bound
                                            ? "The reviewed prop-only plan can be applied to the active native Map as one undoable command."
                                            : "Map prop bindings are missing for one or more planned asset IDs.";
        } else {
            const bool all_event_layers_bound = std::all_of(
                current_plan_.logic_edits.begin(), current_plan_.logic_edits.end(), [&](const urpg::ai::CreatorLogicEdit& edit) {
                    const auto planned_layer_id = edit.payload.value("layer_id", "");
                    return std::any_of(event_layer_bindings_.begin(), event_layer_bindings_.end(),
                                       [&](const EventLayerBinding& binding) {
                                           return binding.planned_layer_id == planned_layer_id &&
                                                  binding.layer_id == planned_layer_id && !binding.layer_id.empty();
                                       });
                });
            apply_preview["domain"] = "event_message";
            apply_preview["would_apply"] = all_event_layers_bound;
            apply_preview["code"] = all_event_layers_bound ? "creator_native_event_message_command_ready"
                                                             : "creator_native_event_layer_bindings_missing";
            apply_preview["message"] = all_event_layers_bound
                                            ? "The reviewed message-event plan can be applied to the active native Map as one undoable command."
                                            : "Map event-layer bindings are missing for the planned message event.";
        }
        reviewed_document_revision_ = map_workspace_->perspectiveDocumentRevision();
        apply_preview["source_revision"] = reviewed_document_revision_;
    }

    last_render_snapshot_ = {
        {"prompt", request_.prompt},
        {"selected_tile", {{"x", request_.tile_x}, {"y", request_.tile_y}, {"tile_id", request_.selected_tile_id},
                           {"paint_width", request_.tile_paint_width}, {"paint_height", request_.tile_paint_height}}},
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
    const bool is_event_message_plan = current_plan_.intent == "place_event_message" &&
                                       current_plan_.tile_edits.empty() && current_plan_.prop_edits.empty() &&
                                       !current_plan_.logic_edits.empty() &&
                                       std::all_of(current_plan_.logic_edits.begin(), current_plan_.logic_edits.end(),
                                                   [](const urpg::ai::CreatorLogicEdit& edit) {
                                                       return edit.kind == "message" && edit.trigger == "confirm_interact" &&
                                                              edit.payload.is_object() && edit.payload.contains("layer_id") &&
                                                              edit.payload.contains("label") && edit.payload.contains("text") &&
                                                              edit.payload["layer_id"].is_string() && edit.payload["label"].is_string() &&
                                                              edit.payload["text"].is_string();
                                                   });
    if ((!current_plan_.logic_edits.empty() && !is_event_message_plan) ||
        (!current_plan_.tile_edits.empty() && !current_plan_.prop_edits.empty())) {
        last_render_snapshot_["last_apply"] = {
            {"applied", false},
            {"code", "creator_native_domain_not_available"},
            {"message", "This plan includes event logic or mixed native domains, which require separate reviewed commands."},
        };
        return false;
    }
    if (map_workspace_ == nullptr) {
        last_render_snapshot_["last_apply"] = {
            {"applied", false},
            {"code", "creator_native_map_unbound"},
            {"message", "Open and bind a Perspective 2D Map before applying a reviewed creator command."},
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

    SpatialAuthoringWorkspace::Perspective2DNativeCommandResult result;
    if (!current_plan_.tile_edits.empty()) {
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
        result = map_workspace_->applyNativeTileEdits(reviewed_document_revision_, native_edits);
    } else if (!current_plan_.prop_edits.empty()) {
        std::vector<SpatialAuthoringWorkspace::Perspective2DNativePropEdit> native_edits;
        native_edits.reserve(current_plan_.prop_edits.size());
        for (const auto& edit : current_plan_.prop_edits) {
            const auto binding = std::find_if(prop_asset_bindings_.begin(), prop_asset_bindings_.end(),
                                              [&](const PropAssetBinding& candidate) {
                                                  return candidate.planned_asset_id == edit.asset_id;
                                              });
            if (binding == prop_asset_bindings_.end() || binding->asset_id.empty() || binding->project_path.empty()) {
                last_render_snapshot_["last_apply"] = {
                    {"applied", false},
                    {"code", "creator_native_prop_bindings_missing"},
                    {"message", "Resolve every planned prop asset ID to an active attached Map prop-palette entry before applying."},
                };
                return false;
            }
            native_edits.push_back({edit.id, binding->asset_id, binding->project_path, edit.tile_x, edit.tile_y});
        }
        result = map_workspace_->applyNativePropEdits(reviewed_document_revision_, native_edits);
    } else {
        std::vector<SpatialAuthoringWorkspace::Perspective2DNativeEventMessageEdit> native_edits;
        native_edits.reserve(current_plan_.logic_edits.size());
        for (const auto& edit : current_plan_.logic_edits) {
            const auto planned_layer_id = edit.payload.value("layer_id", "");
            const auto binding = std::find_if(event_layer_bindings_.begin(), event_layer_bindings_.end(),
                                              [&](const EventLayerBinding& candidate) {
                                                  return candidate.planned_layer_id == planned_layer_id &&
                                                         candidate.layer_id == planned_layer_id;
                                              });
            if (binding == event_layer_bindings_.end() || binding->layer_id.empty()) {
                last_render_snapshot_["last_apply"] = {
                    {"applied", false},
                    {"code", "creator_native_event_layer_bindings_missing"},
                    {"message", "Resolve the planned message event to a visible active Map event layer before applying."},
                };
                return false;
            }
            native_edits.push_back({edit.id, binding->layer_id, edit.payload.value("label", ""),
                                    edit.payload.value("text", ""), edit.tile_x, edit.tile_y});
        }
        result = map_workspace_->applyNativeEventMessageEdits(reviewed_document_revision_, native_edits);
    }
    last_render_snapshot_["last_apply"] = {
        {"applied", result.success},
        {"code", result.code},
        {"message", result.message},
        {"document_revision", result.document_revision},
        {"applied_tile_count", result.applied_tile_count},
        {"applied_prop_count", result.applied_prop_count},
        {"applied_event_count", result.applied_event_count},
    };
    return result.success;
}

const nlohmann::json& CreatorCommandPanel::lastRenderSnapshot() const {
    return last_render_snapshot_;
}

} // namespace urpg::editor
