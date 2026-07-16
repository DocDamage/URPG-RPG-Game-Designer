#include "editor/spatial/grid_part_placement_panel.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <optional>
#include <set>
#include <sstream>
#include <utility>

namespace urpg::editor {
namespace {

bool footprintsOverlap(const urpg::map::PlacedPartInstance& left, const urpg::map::PlacedPartInstance& right) {
    return left.grid_x < right.grid_x + right.width && left.grid_x + left.width > right.grid_x &&
           left.grid_y < right.grid_y + right.height && left.grid_y + left.height > right.grid_y;
}

std::set<std::string> splitTags(const std::string& value) {
    std::set<std::string> tags;
    std::istringstream stream(value);
    std::string tag;
    while (std::getline(stream, tag, ',')) {
        if (!tag.empty()) tags.insert(tag);
    }
    return tags;
}

bool hasSharedTag(const std::vector<std::string>& expected, const std::set<std::string>& actual) {
    return std::any_of(expected.begin(), expected.end(), [&](const std::string& tag) { return actual.contains(tag); });
}

std::optional<std::string> resolveParameterValue(const std::string& value,
                                                 const std::unordered_map<std::string, std::string>& parameters) {
    std::string resolved = value;
    size_t begin = 0;
    while ((begin = resolved.find("${", begin)) != std::string::npos) {
        const size_t end = resolved.find('}', begin + 2);
        if (end == std::string::npos) {
            return std::nullopt;
        }
        const auto parameter = parameters.find(resolved.substr(begin + 2, end - begin - 2));
        if (parameter == parameters.end()) {
            return std::nullopt;
        }
        resolved.replace(begin, end - begin + 1, parameter->second);
        begin += parameter->second.size();
    }
    return resolved;
}

} // namespace

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
    if (catalog_ == nullptr ||
        (!selected_smart_prefab_id_.empty() && catalog_->findSmartPrefab(selected_smart_prefab_id_) == nullptr)) {
        selected_smart_prefab_id_.clear();
    }
    last_smart_prefab_result_ = {};
    last_rectangle_fill_result_ = {};
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
    last_rectangle_fill_result_ = {};
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

GridPartPlacementPanel::RectangleFillResult GridPartPlacementPanel::PreviewSelectedPartRectangle(
    int32_t min_x, int32_t min_y, int32_t max_x, int32_t max_y) const {
    RectangleFillResult result;
    if (document_ == nullptr || catalog_ == nullptr || selected_part_id_.empty()) {
        result.code = "rectangle_fill_owner_unavailable";
        result.message = "Open a Grid Part document and select a catalog part before reviewing a rectangle fill.";
        return result;
    }

    const auto* definition = catalog_->find(selected_part_id_);
    if (definition == nullptr) {
        result.code = "rectangle_fill_missing_definition";
        result.message = "The selected catalog part is no longer available.";
        return result;
    }

    const int32_t left = std::min(min_x, max_x);
    const int32_t right = std::max(min_x, max_x);
    const int32_t top = std::min(min_y, max_y);
    const int32_t bottom = std::max(min_y, max_y);
    if (!document_->inBounds(left, top) || !document_->inBounds(right, bottom)) {
        result.code = "rectangle_fill_out_of_bounds";
        result.message = "The reviewed rectangle must stay within the active Grid Part document.";
        return result;
    }

    size_t operation_count = 0;
    for (int32_t y = top; y <= bottom; ++y) {
        for (int32_t x = left; x <= right; ++x) {
            const auto instance = makeInstance(*definition, x, y);
            if (!document_->footprintInBounds(instance)) {
                result.code = "rectangle_fill_footprint_out_of_bounds";
                result.message = "The selected part footprint would extend outside the active Grid Part document.";
                return result;
            }
            ++operation_count;
        }
    }

    result.accepted = operation_count != 0;
    result.code = result.accepted ? "rectangle_fill_ready" : "rectangle_fill_empty";
    result.message = result.accepted ? "The selected Grid Part rectangle is ready to apply as one undoable Map operation."
                                    : "The selected Grid Part rectangle contains no placement cells.";
    result.operation_count = operation_count;
    return result;
}

bool GridPartPlacementPanel::FillSelectedPartRectangle(int32_t min_x, int32_t min_y, int32_t max_x, int32_t max_y) {
    last_rectangle_fill_result_ = PreviewSelectedPartRectangle(min_x, min_y, max_x, max_y);
    if (!last_rectangle_fill_result_.accepted || document_ == nullptr || catalog_ == nullptr) {
        captureRenderSnapshot();
        return false;
    }

    const auto* definition = catalog_->find(selected_part_id_);
    if (definition == nullptr) {
        last_rectangle_fill_result_ = {false, "rectangle_fill_missing_definition",
                                       "The selected catalog part is no longer available."};
        captureRenderSnapshot();
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
            commands.push_back(std::make_unique<urpg::map::PlacePartCommand>(std::move(instance)));
        }
    }

    const bool filled = history_.execute(
        *document_, std::make_unique<urpg::map::BulkGridPartCommand>(std::move(commands), "Fill Grid Parts"));
    if (filled) {
        last_rectangle_fill_result_.code = "rectangle_fill_applied";
        last_rectangle_fill_result_.message =
            "Applied the reviewed Grid Part rectangle as one undoable native Map operation.";
    } else {
        last_rectangle_fill_result_.accepted = false;
        last_rectangle_fill_result_.code = "rectangle_fill_apply_rejected";
        last_rectangle_fill_result_.message =
            "The Grid Part rectangle was rejected without leaving partial Map changes.";
        last_rectangle_fill_result_.operation_count = 0;
    }
    captureRenderSnapshot();
    return filled;
}

bool GridPartPlacementPanel::SetSelectedSmartPrefabId(const std::string& prefab_id) {
    if (catalog_ == nullptr || catalog_->findSmartPrefab(prefab_id) == nullptr) {
        return false;
    }
    selected_smart_prefab_id_ = prefab_id;
    last_smart_prefab_result_ = {};
    captureRenderSnapshot();
    return true;
}

GridPartPlacementPanel::SmartPrefabPlacementResult GridPartPlacementPanel::PreviewSelectedSmartPrefabAtGrid(
    int32_t grid_x, int32_t grid_y, const std::unordered_map<std::string, std::string>& parameter_values) const {
    SmartPrefabPlacementResult result;
    if (catalog_ == nullptr || selected_smart_prefab_id_.empty()) {
        result.code = "smart_prefab_missing_selection";
        result.message = "Select a native smart prefab before reviewing its placement.";
        return result;
    }
    const auto* prefab = catalog_->findSmartPrefab(selected_smart_prefab_id_);
    if (prefab == nullptr) {
        result.code = "smart_prefab_missing_definition";
        result.message = "The selected smart prefab is no longer present in the active catalog.";
        return result;
    }
    std::vector<urpg::map::PlacedPartInstance> ignored;
    return buildSmartPrefabInstances(*prefab, grid_x, grid_y, parameter_values, ignored);
}

bool GridPartPlacementPanel::PlaceSelectedSmartPrefabAtGrid(
    int32_t grid_x, int32_t grid_y, const std::unordered_map<std::string, std::string>& parameter_values) {
    if (catalog_ == nullptr || document_ == nullptr || selected_smart_prefab_id_.empty()) {
        last_smart_prefab_result_ = {false, "smart_prefab_owner_unavailable",
                                     "Open a Grid Part document and select a smart prefab before applying it."};
        captureRenderSnapshot();
        return false;
    }
    const auto* prefab = catalog_->findSmartPrefab(selected_smart_prefab_id_);
    if (prefab == nullptr) {
        last_smart_prefab_result_ = {false, "smart_prefab_missing_definition",
                                     "The selected smart prefab is no longer present in the active catalog."};
        captureRenderSnapshot();
        return false;
    }

    std::vector<urpg::map::PlacedPartInstance> instances;
    last_smart_prefab_result_ = buildSmartPrefabInstances(*prefab, grid_x, grid_y, parameter_values, instances);
    if (!last_smart_prefab_result_.accepted) {
        captureRenderSnapshot();
        return false;
    }

    std::vector<std::unique_ptr<urpg::map::IGridPartCommand>> commands;
    commands.reserve(instances.size());
    for (auto& instance : instances) {
        commands.push_back(std::make_unique<urpg::map::PlacePartCommand>(std::move(instance)));
    }
    if (!history_.execute(*document_, std::make_unique<urpg::map::BulkGridPartCommand>(
                                         std::move(commands), "Place Smart Prefab: " + prefab->display_name))) {
        last_smart_prefab_result_.accepted = false;
        last_smart_prefab_result_.code = "smart_prefab_apply_rejected";
        last_smart_prefab_result_.message = "The smart prefab was rejected without leaving partial Map changes.";
        last_smart_prefab_result_.accepted_operation_count = 0;
        last_smart_prefab_result_.reviewed_operation_ids.clear();
        captureRenderSnapshot();
        return false;
    }
    last_smart_prefab_result_.code = "smart_prefab_applied";
    last_smart_prefab_result_.message = "Applied the reviewed smart prefab as one undoable native Map operation.";
    captureRenderSnapshot();
    return true;
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
    last_render_snapshot_.selected_smart_prefab_id = selected_smart_prefab_id_;
    last_render_snapshot_.last_smart_prefab_result = last_smart_prefab_result_;
    last_render_snapshot_.last_rectangle_fill_result = last_rectangle_fill_result_;
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
    if (catalog_ != nullptr) {
        for (const auto& prefab : catalog_->allSmartPrefabs()) {
            SmartPrefabSnapshot prefab_snapshot;
            prefab_snapshot.prefab_id = prefab.prefab_id;
            prefab_snapshot.version = prefab.version;
            prefab_snapshot.display_name = prefab.display_name;
            prefab_snapshot.description = prefab.description;
            prefab_snapshot.operation_count = prefab.operations.size();
            prefab_snapshot.dependencies = prefab.dependencies;
            prefab_snapshot.conflict_tags = prefab.conflict_tags;
            prefab_snapshot.selected = prefab.prefab_id == selected_smart_prefab_id_;
            for (const auto& parameter : prefab.parameters) {
                prefab_snapshot.parameters.push_back(
                    {parameter.key, parameter.default_value, parameter.required, parameter.allowed_values});
            }
            last_render_snapshot_.smart_prefabs.push_back(std::move(prefab_snapshot));
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

GridPartPlacementPanel::SmartPrefabPlacementResult GridPartPlacementPanel::buildSmartPrefabInstances(
    const urpg::map::GridPartSmartPrefab& prefab, const int32_t grid_x, const int32_t grid_y,
    const std::unordered_map<std::string, std::string>& parameter_values,
    std::vector<urpg::map::PlacedPartInstance>& instances) const {
    SmartPrefabPlacementResult result;
    if (document_ == nullptr || catalog_ == nullptr) {
        result.code = "smart_prefab_owner_unavailable";
        result.message = "Open a Grid Part document and catalog before reviewing a smart prefab.";
        return result;
    }
    for (const auto& dependency : prefab.dependencies) {
        if (catalog_->find(dependency) == nullptr) {
            result.code = "smart_prefab_dependency_missing";
            result.message = "A declared smart prefab part dependency is missing from the active catalog.";
            return result;
        }
    }

    std::unordered_map<std::string, std::string> parameters;
    for (const auto& parameter : prefab.parameters) {
        if (parameter.key.empty() || parameters.contains(parameter.key)) {
            result.code = "smart_prefab_parameter_invalid";
            result.message = "The smart prefab declares a missing or duplicate parameter key.";
            return result;
        }
        const auto supplied = parameter_values.find(parameter.key);
        parameters[parameter.key] = supplied == parameter_values.end() ? parameter.default_value : supplied->second;
        const auto& value = parameters[parameter.key];
        if (parameter.required && value.empty()) {
            result.code = "smart_prefab_parameter_required";
            result.message = "A required smart prefab parameter is empty.";
            return result;
        }
        if (!parameter.allowed_values.empty() &&
            std::find(parameter.allowed_values.begin(), parameter.allowed_values.end(), value) ==
                parameter.allowed_values.end()) {
            result.code = "smart_prefab_parameter_value_invalid";
            result.message = "A smart prefab parameter is outside its declared allowed values.";
            return result;
        }
    }
    for (const auto& [key, _] : parameter_values) {
        if (!parameters.contains(key)) {
            result.code = "smart_prefab_parameter_unknown";
            result.message = "The review supplied a parameter not declared by the selected smart prefab.";
            return result;
        }
    }

    const std::set<std::string> incoming_tags(prefab.conflict_tags.begin(), prefab.conflict_tags.end());
    for (const auto& existing : document_->parts()) {
        const auto tags = existing.properties.find("smart_prefab.conflict_tags");
        if (tags != existing.properties.end() && hasSharedTag(prefab.conflict_tags, splitTags(tags->second))) {
            result.code = "smart_prefab_conflict_tag";
            result.message = "The smart prefab conflicts with an existing declared Map prefab tag.";
            return result;
        }
    }

    std::set<std::string> operation_ids;
    for (const auto& operation : prefab.operations) {
        if (operation.operation_id.empty() || !operation_ids.insert(operation.operation_id).second) {
            result.rejected_operation_ids.push_back(operation.operation_id);
            continue;
        }
        const auto* definition = catalog_->find(operation.part_id);
        if (definition == nullptr) {
            result.rejected_operation_ids.push_back(operation.operation_id);
            continue;
        }
        auto instance = makeInstance(*definition, grid_x + operation.offset_x, grid_y + operation.offset_y);
        instance.grid_z = operation.offset_z;
        instance.instance_id = document_->mapId() + ":smart_prefab:" + prefab.prefab_id + ":" +
                               operation.operation_id + ":" + std::to_string(grid_x) + ":" + std::to_string(grid_y);
        if (document_->hasInstanceId(instance.instance_id) || !document_->footprintInBounds(instance)) {
            result.rejected_operation_ids.push_back(operation.operation_id);
            continue;
        }
        bool property_overrides_valid = true;
        for (const auto& [key, value] : operation.property_overrides) {
            const auto resolved = resolveParameterValue(value, parameters);
            if (!resolved.has_value()) {
                property_overrides_valid = false;
                break;
            }
            instance.properties[key] = *resolved;
        }
        if (!property_overrides_valid) {
            result.rejected_operation_ids.push_back(operation.operation_id);
            continue;
        }
        instance.properties["smart_prefab.id"] = prefab.prefab_id;
        instance.properties["smart_prefab.version"] = prefab.version;
        instance.properties["smart_prefab.operation_id"] = operation.operation_id;
        instance.properties["smart_prefab.anchor"] = std::to_string(grid_x) + "," + std::to_string(grid_y);
        instance.properties["smart_prefab.group_id"] = document_->mapId() + ":smart_prefab:" + prefab.prefab_id + ":" +
                                                       std::to_string(grid_x) + ":" + std::to_string(grid_y);
        if (!incoming_tags.empty()) {
            std::string serialized_tags;
            for (const auto& tag : incoming_tags) {
                if (!serialized_tags.empty()) serialized_tags += ',';
                serialized_tags += tag;
            }
            instance.properties["smart_prefab.conflict_tags"] = std::move(serialized_tags);
        }
        for (const auto& [key, value] : parameters) {
            instance.properties["smart_prefab.parameter." + key] = value;
        }
        bool overlaps_existing_part = false;
        for (const auto& existing : document_->parts()) {
            const auto* existing_definition = catalog_->find(existing.part_id);
            if (existing_definition != nullptr && footprintsOverlap(instance, existing) &&
                !definition->footprint.allow_overlap && !existing_definition->footprint.allow_overlap) {
                overlaps_existing_part = true;
                break;
            }
        }
        if (overlaps_existing_part) {
            result.rejected_operation_ids.push_back(operation.operation_id);
            continue;
        }
        bool overlaps_planned_part = false;
        for (const auto& planned : instances) {
            const auto* planned_definition = catalog_->find(planned.part_id);
            if (planned_definition != nullptr && footprintsOverlap(instance, planned) &&
                !definition->footprint.allow_overlap && !planned_definition->footprint.allow_overlap) {
                overlaps_planned_part = true;
                break;
            }
        }
        if (overlaps_planned_part) {
            result.rejected_operation_ids.push_back(operation.operation_id);
            continue;
        }
        result.reviewed_operation_ids.push_back(operation.operation_id);
        instances.push_back(std::move(instance));
    }
    if (!result.rejected_operation_ids.empty()) {
        instances.clear();
        result.code = "smart_prefab_operation_rejected";
        result.message = "One or more smart prefab operations failed native Map preflight.";
        return result;
    }
    result.accepted = !instances.empty();
    result.code = result.accepted ? "smart_prefab_ready" : "smart_prefab_operations_empty";
    result.message = result.accepted ? "Smart prefab operations passed native Map preflight."
                                    : "The smart prefab has no operations to apply.";
    result.accepted_operation_count = instances.size();
    return result;
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
