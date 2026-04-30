#include "editor/spatial/grid_part_placement_panel.h"

#include <algorithm>
#include <cmath>
#include <memory>

namespace urpg::editor {

void GridPartPlacementPanel::Render(const urpg::FrameContext& context) {
    (void)context;
    if (!m_visible) {
        return;
    }

    captureRenderSnapshot();
}

void GridPartPlacementPanel::SetTargets(urpg::map::GridPartDocument* document,
                                        const urpg::map::GridPartCatalog* catalog,
                                        urpg::presentation::SpatialMapOverlay* overlay) {
    document_ = document;
    catalog_ = catalog;
    overlay_ = overlay;
    if (catalog_ == nullptr || (!selected_part_id_.empty() && catalog_->find(selected_part_id_) == nullptr)) {
        selected_part_id_.clear();
    }
    hover_active_ = false;
    captureRenderSnapshot();
}

void GridPartPlacementPanel::SetProjectionSettings(const PropPlacementPanel::ScreenProjectionSettings& settings) {
    projection_settings_ = settings;
    captureRenderSnapshot();
}

bool GridPartPlacementPanel::SetSelectedPartId(const std::string& part_id) {
    if (catalog_ == nullptr || catalog_->find(part_id) == nullptr) {
        return false;
    }

    selected_part_id_ = part_id;
    hover_active_ = false;
    captureRenderSnapshot();
    return true;
}

bool GridPartPlacementPanel::HoverSelectedPartAtGrid(int32_t grid_x, int32_t grid_y) {
    hover_active_ = true;
    hover_x_ = grid_x;
    hover_y_ = grid_y;
    hover_valid_ = false;
    hover_reason_.clear();

    if (document_ == nullptr) {
        hover_reason_ = "missing_document";
    } else if (catalog_ == nullptr) {
        hover_reason_ = "missing_catalog";
    } else if (selected_part_id_.empty()) {
        hover_reason_ = "missing_selection";
    } else {
        const auto* definition = catalog_->find(selected_part_id_);
        if (definition == nullptr) {
            hover_reason_ = "missing_definition";
        } else {
            const auto instance = makeInstance(*definition, grid_x, grid_y);
            hover_valid_ = document_->footprintInBounds(instance);
            hover_reason_ = hover_valid_ ? std::string{} : "out_of_bounds";
        }
    }

    captureRenderSnapshot();
    return hover_valid_;
}

bool GridPartPlacementPanel::HoverSelectedPartFromScreen(float screen_x, float screen_y) {
    int32_t grid_x = -1;
    int32_t grid_y = -1;
    if (!projectScreenToGrid(screen_x, screen_y, grid_x, grid_y)) {
        hover_active_ = true;
        hover_valid_ = false;
        hover_reason_ = "projection_missed";
        hover_x_ = -1;
        hover_y_ = -1;
        captureRenderSnapshot();
        return false;
    }

    return HoverSelectedPartAtGrid(grid_x, grid_y);
}

bool GridPartPlacementPanel::PlaceSelectedPartAtGrid(int32_t grid_x, int32_t grid_y) {
    if (!HoverSelectedPartAtGrid(grid_x, grid_y)) {
        return false;
    }

    const auto* definition = catalog_->find(selected_part_id_);
    if (definition == nullptr || document_ == nullptr) {
        return false;
    }

    auto command = std::make_unique<urpg::map::PlacePartCommand>(makeInstance(*definition, grid_x, grid_y));
    const bool placed = history_.execute(*document_, std::move(command));
    captureRenderSnapshot();
    return placed;
}

bool GridPartPlacementPanel::PlaceSelectedPartFromScreen(float screen_x, float screen_y) {
    int32_t grid_x = -1;
    int32_t grid_y = -1;
    if (!projectScreenToGrid(screen_x, screen_y, grid_x, grid_y)) {
        return false;
    }
    return PlaceSelectedPartAtGrid(grid_x, grid_y);
}

bool GridPartPlacementPanel::FillSelectedPartRectangle(int32_t min_x, int32_t min_y, int32_t max_x, int32_t max_y) {
    if (document_ == nullptr || catalog_ == nullptr || selected_part_id_.empty()) {
        return false;
    }

    const auto* definition = catalog_->find(selected_part_id_);
    if (definition == nullptr) {
        return false;
    }

    const int32_t left = std::min(min_x, max_x);
    const int32_t right = std::max(min_x, max_x);
    const int32_t top = std::min(min_y, max_y);
    const int32_t bottom = std::max(min_y, max_y);

    std::vector<std::unique_ptr<urpg::map::IGridPartCommand>> commands;
    for (int32_t y = top; y <= bottom; ++y) {
        for (int32_t x = left; x <= right; ++x) {
            auto instance = makeInstance(*definition, x, y);
            if (!document_->footprintInBounds(instance)) {
                captureRenderSnapshot();
                return false;
            }
            commands.push_back(std::make_unique<urpg::map::PlacePartCommand>(std::move(instance)));
        }
    }

    const bool filled = history_.execute(
        *document_, std::make_unique<urpg::map::BulkGridPartCommand>(std::move(commands), "Fill Grid Parts"));
    captureRenderSnapshot();
    return filled;
}

bool GridPartPlacementPanel::Undo() {
    if (document_ == nullptr) {
        return false;
    }

    const bool undone = history_.undo(*document_);
    captureRenderSnapshot();
    return undone;
}

bool GridPartPlacementPanel::Redo() {
    if (document_ == nullptr) {
        return false;
    }

    const bool redone = history_.redo(*document_);
    captureRenderSnapshot();
    return redone;
}

void GridPartPlacementPanel::captureRenderSnapshot() {
    last_render_snapshot_ = {};
    last_render_snapshot_.visible = m_visible;
    last_render_snapshot_.has_document = document_ != nullptr;
    last_render_snapshot_.has_catalog = catalog_ != nullptr;
    last_render_snapshot_.has_spatial_overlay = overlay_ != nullptr;
    last_render_snapshot_.selected_part_id = selected_part_id_;
    last_render_snapshot_.hover_active = hover_active_;
    last_render_snapshot_.hover_valid = hover_valid_;
    last_render_snapshot_.hover_reason = hover_reason_;
    last_render_snapshot_.hover_x = hover_x_;
    last_render_snapshot_.hover_y = hover_y_;
    last_render_snapshot_.can_undo = history_.canUndo();
    last_render_snapshot_.can_redo = history_.canRedo();

    if (document_ != nullptr) {
        last_render_snapshot_.placed_count = document_->parts().size();
    }
    if (catalog_ != nullptr && !selected_part_id_.empty()) {
        if (const auto* definition = catalog_->find(selected_part_id_); definition != nullptr) {
            last_render_snapshot_.footprint_width = definition->footprint.width;
            last_render_snapshot_.footprint_height = definition->footprint.height;
        }
    }
    if (document_ != nullptr && catalog_ != nullptr) {
        last_render_snapshot_.diagnostic_count =
            urpg::map::ValidateGridPartDocument(*document_, *catalog_).diagnostics.size();
    }
}

bool GridPartPlacementPanel::projectScreenToGrid(float screen_x, float screen_y, int32_t& out_grid_x,
                                                 int32_t& out_grid_y) const {
    if (overlay_ == nullptr) {
        return false;
    }

    float world_x = 0.0f;
    float world_y = 0.0f;
    float world_z = 0.0f;
    if (!PropPlacementPanel::TryProjectScreenToGround(*overlay_, screen_x, screen_y, projection_settings_, world_x,
                                                      world_y, world_z)) {
        return false;
    }

    out_grid_x = static_cast<int32_t>(std::floor(world_x));
    out_grid_y = static_cast<int32_t>(std::floor(world_z));
    return true;
}

urpg::map::PlacedPartInstance GridPartPlacementPanel::makeInstance(const urpg::map::GridPartDefinition& definition,
                                                                   int32_t grid_x, int32_t grid_y) const {
    urpg::map::PlacedPartInstance instance;
    instance.instance_id = makeInstanceId(definition.part_id, grid_x, grid_y);
    instance.part_id = definition.part_id;
    instance.category = definition.category;
    instance.layer = definition.default_layer;
    instance.grid_x = grid_x;
    instance.grid_y = grid_y;
    instance.width = definition.footprint.width;
    instance.height = definition.footprint.height;
    instance.properties = definition.default_properties;
    if (!definition.asset_id.empty()) {
        instance.properties["assetId"] = definition.asset_id;
    }
    if (!definition.prefab_path.empty()) {
        instance.properties["prefabPath"] = definition.prefab_path;
    }
    return instance;
}

std::string GridPartPlacementPanel::makeInstanceId(const std::string& part_id, int32_t grid_x, int32_t grid_y) const {
    const std::string map_id = document_ == nullptr ? std::string{} : document_->mapId();
    const std::string base = map_id + ":" + part_id + ":" + std::to_string(grid_x) + ":" + std::to_string(grid_y);
    if (document_ == nullptr || !document_->hasInstanceId(base)) {
        return base;
    }

    int32_t suffix = 1;
    while (document_->hasInstanceId(base + ":" + std::to_string(suffix))) {
        ++suffix;
    }
    return base + ":" + std::to_string(suffix);
}

} // namespace urpg::editor
