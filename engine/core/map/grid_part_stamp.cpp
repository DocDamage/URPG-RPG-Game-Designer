#include "engine/core/map/grid_part_stamp.h"

#include <memory>
#include <utility>

namespace urpg::map {

namespace {

GridPartDiagnostic makeStampDiagnostic(GridPartSeverity severity, std::string code, std::string message,
                                       const PlacedPartInstance* instance = nullptr, std::string target = {}) {
    GridPartDiagnostic diagnostic;
    diagnostic.severity = severity;
    diagnostic.code = std::move(code);
    diagnostic.message = std::move(message);
    diagnostic.target = std::move(target);
    if (instance != nullptr) {
        diagnostic.instance_id = instance->instance_id;
        diagnostic.part_id = instance->part_id;
        diagnostic.x = instance->grid_x;
        diagnostic.y = instance->grid_y;
    }
    return diagnostic;
}

std::string makeFreshInstanceId(const GridPartDocument& document, const std::string& part_id, int32_t grid_x,
                                int32_t grid_y) {
    const std::string base =
        document.mapId() + ":" + part_id + ":" + std::to_string(grid_x) + ":" + std::to_string(grid_y);
    if (!document.hasInstanceId(base)) {
        return base;
    }

    int32_t suffix = 1;
    while (document.hasInstanceId(base + ":" + std::to_string(suffix))) {
        ++suffix;
    }
    return base + ":" + std::to_string(suffix);
}

} // namespace

GridPartStampPlacementResult PlaceGridPartStamp(GridPartDocument& document, const GridPartCatalog& catalog,
                                                const GridPartStamp& stamp, int32_t origin_x, int32_t origin_y) {
    GridPartStampPlacementResult result;
    if (stamp.parts.empty()) {
        result.diagnostics.push_back(
            makeStampDiagnostic(GridPartSeverity::Error, "stamp_empty", "Grid part stamp has no parts."));
        return result;
    }

    std::vector<std::unique_ptr<IGridPartCommand>> commands;
    commands.reserve(stamp.parts.size());

    for (const auto& source : stamp.parts) {
        if (catalog.find(source.part_id) == nullptr) {
            result.diagnostics.push_back(makeStampDiagnostic(GridPartSeverity::Error, "part_definition_missing",
                                                             "Grid part stamp references a missing catalog definition.",
                                                             &source, source.part_id));
            return result;
        }

        auto placed = source;
        placed.grid_x = origin_x + source.grid_x;
        placed.grid_y = origin_y + source.grid_y;
        placed.instance_id = makeFreshInstanceId(document, placed.part_id, placed.grid_x, placed.grid_y);
        if (!document.footprintInBounds(placed)) {
            result.diagnostics.push_back(makeStampDiagnostic(
                GridPartSeverity::Blocker, "part_footprint_out_of_bounds",
                "Grid part stamp placement would place a part outside the document bounds.", &placed, stamp.stamp_id));
            return result;
        }

        result.placed_instance_ids.push_back(placed.instance_id);
        commands.push_back(std::make_unique<PlacePartCommand>(std::move(placed)));
    }

    BulkGridPartCommand bulk(std::move(commands), "Place Grid Part Stamp");
    result.ok = bulk.apply(document);
    if (!result.ok) {
        result.placed_instance_ids.clear();
        result.diagnostics.push_back(makeStampDiagnostic(GridPartSeverity::Error, "stamp_place_failed",
                                                         "Grid part stamp placement failed atomically.", nullptr,
                                                         stamp.stamp_id));
    }
    return result;
}

} // namespace urpg::map
