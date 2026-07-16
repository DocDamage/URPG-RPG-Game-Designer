#include "engine/core/map/grid_part_prefab_lifecycle.h"

#include <algorithm>
#include <charconv>
#include <memory>
#include <set>

namespace urpg::map {
namespace {
bool anchorOf(const PlacedPartInstance& instance, int32_t& x, int32_t& y) {
    const auto found = instance.properties.find("smart_prefab.anchor");
    if (found == instance.properties.end()) return false;
    const auto separator = found->second.find(',');
    if (separator == std::string::npos) return false;
    const auto xResult = std::from_chars(found->second.data(), found->second.data() + separator, x);
    const auto yResult = std::from_chars(found->second.data() + separator + 1,
                                         found->second.data() + found->second.size(), y);
    return xResult.ec == std::errc{} && yResult.ec == std::errc{};
}
std::string value(const PlacedPartInstance& instance, const char* key) {
    const auto found = instance.properties.find(key); return found == instance.properties.end() ? "" : found->second;
}
}

GridPartPrefabUpdatePreview previewGridPartPrefabUpdate(const GridPartDocument& document,
                                                        const GridPartCatalog& catalog,
                                                        const GridPartSmartPrefab& next) {
    GridPartPrefabUpdatePreview result;
    result.prefab_id = next.prefab_id; result.to_version = next.version;
    if (next.prefab_id.empty() || next.version.empty() || next.operations.empty()) {
        result.code = "prefab_update_definition_invalid"; return result;
    }
    std::set<std::string> closure(next.dependencies.begin(), next.dependencies.end());
    std::set<std::string> groups;
    for (const auto& instance : document.parts()) {
        if (value(instance, "smart_prefab.id") != next.prefab_id) continue;
        if (result.from_version.empty()) result.from_version = value(instance, "smart_prefab.version");
        groups.insert(value(instance, "smart_prefab.group_id"));
        const auto operationId = value(instance, "smart_prefab.operation_id");
        const auto operation = std::find_if(next.operations.begin(), next.operations.end(), [&](const auto& candidate) {
            return candidate.operation_id == operationId;
        });
        if (operation == next.operations.end()) { result.removals.push_back(instance.instance_id); continue; }
        const auto* definition = catalog.find(operation->part_id);
        if (definition == nullptr) { result.code = "prefab_update_dependency_missing"; return result; }
        closure.insert("part:" + operation->part_id);
        auto replacement = instance;
        replacement.part_id = definition->part_id; replacement.category = definition->category;
        replacement.layer = definition->default_layer; replacement.width = definition->footprint.width;
        replacement.height = definition->footprint.height;
        int32_t anchorX = 0, anchorY = 0;
        if (!anchorOf(instance, anchorX, anchorY)) { result.code = "prefab_update_anchor_invalid"; return result; }
        replacement.grid_x = anchorX + operation->offset_x; replacement.grid_y = anchorY + operation->offset_y;
        replacement.grid_z = operation->offset_z; replacement.properties["smart_prefab.version"] = next.version;
        for (const auto& [key, nextValue] : operation->property_overrides) {
            const auto override = instance.properties.find("smart_prefab.override." + key);
            if (override != instance.properties.end()) { replacement.properties[key] = override->second; ++result.preserved_override_count; }
            else replacement.properties[key] = nextValue;
        }
        if (!document.footprintInBounds(replacement)) { result.code = "prefab_update_out_of_bounds"; return result; }
        result.replacements.push_back(std::move(replacement));
    }
    if (groups.empty() || groups.contains("")) { result.code = "prefab_update_instances_missing"; return result; }
    for (const auto& group : groups) {
        const auto sample = std::find_if(document.parts().begin(), document.parts().end(), [&](const auto& instance) {
            return value(instance, "smart_prefab.group_id") == group;
        });
        int32_t anchorX = 0, anchorY = 0;
        if (sample == document.parts().end() || !anchorOf(*sample, anchorX, anchorY)) { result.code = "prefab_update_anchor_invalid"; return result; }
        for (const auto& operation : next.operations) {
            const bool exists = std::any_of(document.parts().begin(), document.parts().end(), [&](const auto& instance) {
                return value(instance, "smart_prefab.group_id") == group && value(instance, "smart_prefab.operation_id") == operation.operation_id;
            });
            if (exists) continue;
            const auto* definition = catalog.find(operation.part_id);
            if (definition == nullptr) { result.code = "prefab_update_dependency_missing"; return result; }
            closure.insert("part:" + operation.part_id);
            PlacedPartInstance addition;
            addition.instance_id = document.mapId() + ":smart_prefab:" + next.prefab_id + ":" + operation.operation_id + ":" +
                                   std::to_string(anchorX) + ":" + std::to_string(anchorY);
            addition.part_id = definition->part_id; addition.category = definition->category; addition.layer = definition->default_layer;
            addition.width = definition->footprint.width; addition.height = definition->footprint.height;
            addition.grid_x = anchorX + operation.offset_x; addition.grid_y = anchorY + operation.offset_y; addition.grid_z = operation.offset_z;
            addition.properties = operation.property_overrides;
            addition.properties["smart_prefab.id"] = next.prefab_id; addition.properties["smart_prefab.version"] = next.version;
            addition.properties["smart_prefab.operation_id"] = operation.operation_id;
            addition.properties["smart_prefab.anchor"] = std::to_string(anchorX) + "," + std::to_string(anchorY);
            addition.properties["smart_prefab.group_id"] = group;
            if (!document.footprintInBounds(addition)) { result.code = "prefab_update_out_of_bounds"; return result; }
            result.additions.push_back(std::move(addition));
        }
    }
    result.affected_instance_count = result.replacements.size() + result.additions.size() + result.removals.size();
    result.package_closure.assign(closure.begin(), closure.end()); result.valid = result.affected_instance_count > 0;
    result.code = result.valid ? "prefab_update_ready" : "prefab_update_no_change"; return result;
}

bool applyGridPartPrefabUpdate(GridPartDocument& document, GridPartCommandHistory& history,
                               const GridPartPrefabUpdatePreview& preview) {
    if (!preview.valid) return false;
    std::vector<std::unique_ptr<IGridPartCommand>> commands;
    for (const auto& replacement : preview.replacements) commands.push_back(std::make_unique<ReplacePartCommand>(replacement));
    for (const auto& id : preview.removals) commands.push_back(std::make_unique<RemovePartCommand>(id));
    for (const auto& addition : preview.additions) commands.push_back(std::make_unique<PlacePartCommand>(addition));
    return history.execute(document, std::make_unique<BulkGridPartCommand>(std::move(commands), "Update Smart Prefab"));
}

bool detachGridPartPrefabGroup(GridPartDocument& document, GridPartCommandHistory& history, const std::string& group_id) {
    std::vector<std::unique_ptr<IGridPartCommand>> commands;
    for (const auto& instance : document.parts()) {
        if (value(instance, "smart_prefab.group_id") != group_id) continue;
        auto detached = instance;
        for (auto iterator = detached.properties.begin(); iterator != detached.properties.end();) {
            if (iterator->first.rfind("smart_prefab.", 0) == 0) iterator = detached.properties.erase(iterator);
            else ++iterator;
        }
        commands.push_back(std::make_unique<ReplacePartCommand>(std::move(detached)));
    }
    return !commands.empty() && history.execute(document, std::make_unique<BulkGridPartCommand>(std::move(commands), "Detach Smart Prefab"));
}

} // namespace urpg::map
