#include "engine/core/map/grid_part_objective.h"

#include <algorithm>
#include <utility>
#include <vector>

namespace urpg::map {

namespace {

GridPartDiagnostic makeObjectiveDiagnostic(GridPartSeverity severity, std::string code, std::string message,
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

std::vector<GridPartCategory> expectedCategories(MapObjectiveType type) {
    switch (type) {
    case MapObjectiveType::ReachExit:
        return {GridPartCategory::Door, GridPartCategory::Trigger};
    case MapObjectiveType::DefeatBoss:
    case MapObjectiveType::DefeatAllEnemies:
        return {GridPartCategory::Enemy};
    case MapObjectiveType::CollectQuestItem:
        return {GridPartCategory::QuestItem};
    case MapObjectiveType::TalkToNpc:
        return {GridPartCategory::Npc};
    case MapObjectiveType::TriggerEvent:
        return {GridPartCategory::Trigger, GridPartCategory::CutsceneZone};
    case MapObjectiveType::SolvePuzzle:
        return {GridPartCategory::Trigger, GridPartCategory::Door, GridPartCategory::QuestItem};
    case MapObjectiveType::OpenChest:
        return {GridPartCategory::TreasureChest};
    case MapObjectiveType::ReachRegion:
        return {GridPartCategory::Trigger, GridPartCategory::CutsceneZone};
    case MapObjectiveType::SurviveWaves:
    case MapObjectiveType::None:
        return {};
    }
    return {};
}

bool categoryAllowed(GridPartCategory category, const std::vector<GridPartCategory>& allowed) {
    return allowed.empty() || std::find(allowed.begin(), allowed.end(), category) != allowed.end();
}

bool hasCompletionSignal(const MapObjective& objective) {
    return !objective.target_instance_id.empty() || !objective.target_event_id.empty() ||
           !objective.required_flag.empty();
}

void sortDiagnostics(std::vector<GridPartDiagnostic>& diagnostics) {
    std::sort(diagnostics.begin(), diagnostics.end(),
              [](const GridPartDiagnostic& left, const GridPartDiagnostic& right) {
                  if (left.severity != right.severity) {
                      return static_cast<uint8_t>(left.severity) > static_cast<uint8_t>(right.severity);
                  }
                  if (left.code != right.code) {
                      return left.code < right.code;
                  }
                  if (left.instance_id != right.instance_id) {
                      return left.instance_id < right.instance_id;
                  }
                  return left.target < right.target;
              });
}

} // namespace

GridPartValidationResult ValidateMapObjective(const GridPartDocument& document, const GridPartCatalog& catalog,
                                              const MapObjective& objective) {
    (void)catalog;
    GridPartValidationResult result;

    if (!objective.required_for_publish && objective.type == MapObjectiveType::None && objective.objective_id.empty()) {
        return result;
    }

    if (objective.type == MapObjectiveType::None || objective.objective_id.empty()) {
        result.diagnostics.push_back(makeObjectiveDiagnostic(GridPartSeverity::Error, "objective_missing",
                                                             "A publishable grid part map requires an objective."));
        result.ok = false;
        return result;
    }

    if (!hasCompletionSignal(objective)) {
        result.diagnostics.push_back(makeObjectiveDiagnostic(
            GridPartSeverity::Error, "objective_has_no_completion_signal",
            "Map objective needs a target instance, target event, or required flag completion signal."));
    }

    if (!objective.target_instance_id.empty()) {
        const auto* target = document.findPart(objective.target_instance_id);
        if (target == nullptr) {
            result.diagnostics.push_back(makeObjectiveDiagnostic(GridPartSeverity::Error, "objective_target_missing",
                                                                 "Map objective references a missing target instance.",
                                                                 nullptr, objective.target_instance_id));
        } else {
            const auto allowed = expectedCategories(objective.type);
            if (!categoryAllowed(target->category, allowed)) {
                result.diagnostics.push_back(
                    makeObjectiveDiagnostic(GridPartSeverity::Error, "objective_target_wrong_category",
                                            "Map objective target category does not satisfy the objective type.",
                                            target, objective.target_instance_id));
            }
        }
    }

    sortDiagnostics(result.diagnostics);
    result.ok = !result.hasErrors();
    return result;
}

} // namespace urpg::map
