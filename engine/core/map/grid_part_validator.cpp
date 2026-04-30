#include "engine/core/map/grid_part_validator.h"

#include <algorithm>
#include <string>
#include <unordered_map>

namespace urpg::map {

namespace {

GridPartDiagnostic makeDiagnostic(GridPartSeverity severity, std::string code, std::string message,
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

bool supportsRuleset(const GridPartDefinition& definition, GridPartRuleset ruleset) {
    return std::find(definition.supported_rulesets.begin(), definition.supported_rulesets.end(), ruleset) !=
           definition.supported_rulesets.end();
}

bool footprintMatchesDefinition(const PlacedPartInstance& instance, const GridPartDefinition& definition) {
    return instance.width == definition.footprint.width && instance.height == definition.footprint.height;
}

bool footprintsOverlap(const PlacedPartInstance& left, const PlacedPartInstance& right) {
    const auto leftMaxX = left.grid_x + left.width;
    const auto leftMaxY = left.grid_y + left.height;
    const auto rightMaxX = right.grid_x + right.width;
    const auto rightMaxY = right.grid_y + right.height;
    return left.grid_x < rightMaxX && leftMaxX > right.grid_x && left.grid_y < rightMaxY && leftMaxY > right.grid_y;
}

bool allowsOverlap(const PlacedPartInstance& instance, const GridPartCatalog& catalog) {
    const auto* definition = catalog.find(instance.part_id);
    if (definition == nullptr) {
        return false;
    }
    return definition->footprint.allow_overlap;
}

void sortDiagnostics(std::vector<GridPartDiagnostic>& diagnostics) {
    std::sort(diagnostics.begin(), diagnostics.end(),
              [](const GridPartDiagnostic& left, const GridPartDiagnostic& right) {
                  if (left.severity != right.severity) {
                      return static_cast<uint8_t>(left.severity) > static_cast<uint8_t>(right.severity);
                  }
                  if (left.instance_id != right.instance_id) {
                      return left.instance_id < right.instance_id;
                  }
                  if (left.code != right.code) {
                      return left.code < right.code;
                  }
                  if (left.y != right.y) {
                      return left.y < right.y;
                  }
                  if (left.x != right.x) {
                      return left.x < right.x;
                  }
                  return left.target < right.target;
              });
}

void validateInstanceBasics(const GridPartDocument& document, const PlacedPartInstance& instance,
                            std::vector<GridPartDiagnostic>& diagnostics) {
    if (instance.instance_id.empty()) {
        diagnostics.push_back(makeDiagnostic(GridPartSeverity::Blocker, "part_instance_id_missing",
                                             "Placed grid part is missing a stable instance id.", &instance));
    }
    if (instance.part_id.empty()) {
        diagnostics.push_back(makeDiagnostic(GridPartSeverity::Blocker, "part_id_missing",
                                             "Placed grid part is missing a part id.", &instance));
    }
    if (!document.footprintInBounds(instance)) {
        diagnostics.push_back(makeDiagnostic(GridPartSeverity::Blocker, "part_footprint_out_of_bounds",
                                             "Placed grid part footprint is outside the document bounds.", &instance));
    }
}

void validateAgainstDefinition(const PlacedPartInstance& instance, const GridPartDefinition& definition,
                               const GridPartValidationOptions& options, std::vector<GridPartDiagnostic>& diagnostics) {
    if (instance.category != definition.category) {
        diagnostics.push_back(makeDiagnostic(GridPartSeverity::Error, "part_category_mismatch",
                                             "Placed grid part category does not match its catalog definition.",
                                             &instance, definition.part_id));
    }
    if (instance.layer != definition.default_layer) {
        diagnostics.push_back(makeDiagnostic(GridPartSeverity::Warning, "part_layer_mismatch",
                                             "Placed grid part layer differs from its catalog default layer.",
                                             &instance, definition.part_id));
    }
    if (!footprintMatchesDefinition(instance, definition)) {
        diagnostics.push_back(makeDiagnostic(GridPartSeverity::Warning, "part_footprint_mismatch",
                                             "Placed grid part footprint differs from its catalog definition.",
                                             &instance, definition.part_id));
    }
    if (options.ruleset.has_value() && !supportsRuleset(definition, *options.ruleset)) {
        diagnostics.push_back(makeDiagnostic(GridPartSeverity::Error, "part_ruleset_incompatible",
                                             "Placed grid part is not supported by the requested ruleset.", &instance,
                                             definition.part_id));
    }
}

} // namespace

bool GridPartValidationResult::hasErrors() const {
    return std::any_of(diagnostics.begin(), diagnostics.end(), [](const GridPartDiagnostic& diagnostic) {
        return diagnostic.severity == GridPartSeverity::Error || diagnostic.severity == GridPartSeverity::Blocker;
    });
}

bool GridPartValidationResult::hasBlockers() const {
    return std::any_of(diagnostics.begin(), diagnostics.end(), [](const GridPartDiagnostic& diagnostic) {
        return diagnostic.severity == GridPartSeverity::Blocker;
    });
}

GridPartValidationResult ValidateGridPartDocument(const GridPartDocument& document, const GridPartCatalog& catalog,
                                                  const GridPartValidationOptions& options) {
    GridPartValidationResult result;

    if (document.width() <= 0 || document.height() <= 0) {
        result.diagnostics.push_back(makeDiagnostic(GridPartSeverity::Blocker, "invalid_document_dimensions",
                                                    "Grid part document dimensions must be positive."));
    }

    std::unordered_map<std::string, size_t> instanceCounts;
    for (const auto& instance : document.parts()) {
        ++instanceCounts[instance.instance_id];
    }

    for (const auto& instance : document.parts()) {
        validateInstanceBasics(document, instance, result.diagnostics);
        if (!instance.instance_id.empty() && instanceCounts[instance.instance_id] > 1) {
            result.diagnostics.push_back(makeDiagnostic(GridPartSeverity::Blocker, "duplicate_instance_id",
                                                        "Placed grid part instance id is duplicated.", &instance));
        }

        const auto* definition = catalog.find(instance.part_id);
        if (definition == nullptr) {
            result.diagnostics.push_back(makeDiagnostic(GridPartSeverity::Error, "part_definition_missing",
                                                        "Placed grid part references a missing catalog definition.",
                                                        &instance, instance.part_id));
            continue;
        }

        validateAgainstDefinition(instance, *definition, options, result.diagnostics);
    }

    const auto& parts = document.parts();
    for (size_t leftIndex = 0; leftIndex < parts.size(); ++leftIndex) {
        for (size_t rightIndex = leftIndex + 1; rightIndex < parts.size(); ++rightIndex) {
            const auto& left = parts[leftIndex];
            const auto& right = parts[rightIndex];
            if (!footprintsOverlap(left, right)) {
                continue;
            }
            if (catalog.find(left.part_id) == nullptr || catalog.find(right.part_id) == nullptr) {
                continue;
            }
            if (!allowsOverlap(left, catalog) && !allowsOverlap(right, catalog)) {
                result.diagnostics.push_back(
                    makeDiagnostic(GridPartSeverity::Error, "part_overlap_conflict",
                                   "Placed grid parts overlap but neither catalog definition permits overlap.", &right,
                                   left.instance_id));
            }
        }
    }

    sortDiagnostics(result.diagnostics);
    result.ok = !result.hasErrors();
    return result;
}

} // namespace urpg::map
