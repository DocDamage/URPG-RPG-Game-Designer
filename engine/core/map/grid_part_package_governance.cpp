#include "engine/core/map/grid_part_package_governance.h"

#include <algorithm>
#include <cstdint>
#include <cctype>
#include <utility>

namespace urpg::map {

namespace {

GridPartDiagnostic makePackageDiagnostic(GridPartSeverity severity, std::string code, std::string message,
                                         const GridPartDependency* dependency = nullptr) {
    GridPartDiagnostic diagnostic;
    diagnostic.severity = severity;
    diagnostic.code = std::move(code);
    diagnostic.message = std::move(message);
    if (dependency != nullptr) {
        diagnostic.instance_id = dependency->source_instance_id;
        diagnostic.target = dependency->id;
    }
    return diagnostic;
}

bool sameDependency(const GridPartDependency& dependency, const GridPartDependencyManifestEntry& entry) {
    return dependency.type == entry.type && dependency.id == entry.id;
}

bool manifestEntryIsReferenced(const GridPartDependencyManifestEntry& entry,
                               const std::vector<GridPartDependency>& dependencies) {
    return std::any_of(dependencies.begin(), dependencies.end(),
                       [&](const GridPartDependency& dependency) { return sameDependency(dependency, entry); });
}

const GridPartDependencyManifestEntry* findManifestEntry(const GridPartDependency& dependency,
                                                        const GridPartPackageManifest& manifest) {
    const auto found = std::find_if(manifest.dependencies.begin(), manifest.dependencies.end(),
                                    [&](const GridPartDependencyManifestEntry& entry) {
                                        return sameDependency(dependency, entry);
                                    });
    return found == manifest.dependencies.end() ? nullptr : &(*found);
}

bool dependencyNeedsLicenseEvidence(GridPartDependencyType type) {
    switch (type) {
    case GridPartDependencyType::Asset:
    case GridPartDependencyType::Audio:
    case GridPartDependencyType::Animation:
    case GridPartDependencyType::Tileset:
    case GridPartDependencyType::Prefab:
        return true;
    default:
        return false;
    }
}

bool isBlank(const std::string& value) {
    return std::all_of(value.begin(), value.end(), [](unsigned char ch) { return std::isspace(ch) != 0; });
}

bool hasSeverityAtLeast(const std::vector<GridPartDiagnostic>& diagnostics, GridPartSeverity severity) {
    return std::any_of(diagnostics.begin(), diagnostics.end(), [&](const GridPartDiagnostic& diagnostic) {
        return static_cast<uint8_t>(diagnostic.severity) >= static_cast<uint8_t>(severity);
    });
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

GridPartReadinessLevel determineReadiness(const std::vector<GridPartDiagnostic>& diagnostics,
                                          const CreatorPackagePermissions& permissions,
                                          const GridPartReadinessEvidence& evidence) {
    if (hasSeverityAtLeast(diagnostics, GridPartSeverity::Blocker)) {
        return GridPartReadinessLevel::Draft;
    }
    if (!evidence.has_player_spawn) {
        return GridPartReadinessLevel::Draft;
    }
    if (hasSeverityAtLeast(diagnostics, GridPartSeverity::Error)) {
        return GridPartReadinessLevel::Playable;
    }
    if (!evidence.has_objective || !evidence.reachability_passed) {
        return GridPartReadinessLevel::Playable;
    }
    if (!permissions.can_export || hasSeverityAtLeast(diagnostics, GridPartSeverity::Warning)) {
        return GridPartReadinessLevel::Validated;
    }
    if (!evidence.target_export_checks_passed) {
        return GridPartReadinessLevel::Publishable;
    }
    if (!evidence.accessibility_checks_passed || !evidence.performance_budget_passed ||
        (evidence.human_review_required && !evidence.human_review_passed)) {
        return GridPartReadinessLevel::Exportable;
    }
    return GridPartReadinessLevel::Certified;
}

} // namespace

bool GridPartPackageGovernanceResult::canPublish() const {
    return static_cast<uint8_t>(readiness) >= static_cast<uint8_t>(GridPartReadinessLevel::Publishable);
}

bool GridPartPackageGovernanceResult::canExport() const {
    return static_cast<uint8_t>(readiness) >= static_cast<uint8_t>(GridPartReadinessLevel::Exportable);
}

GridPartPackageManifest BuildGridPartPackageManifest(const GridPartDocument& document, const GridPartCatalog& catalog,
                                                     CreatorPackagePermissions permissions) {
    GridPartPackageManifest manifest;
    manifest.package_id = document.mapId();
    manifest.permissions = permissions;

    for (const auto& dependency : CollectGridPartDependencies(document, catalog)) {
        const auto duplicate = std::find_if(manifest.dependencies.begin(), manifest.dependencies.end(),
                                            [&](const GridPartDependencyManifestEntry& entry) {
                                                return dependency.type == entry.type && dependency.id == entry.id;
                                            });
        if (duplicate != manifest.dependencies.end()) {
            continue;
        }

        GridPartDependencyManifestEntry entry;
        entry.type = dependency.type;
        entry.id = dependency.id;
        manifest.dependencies.push_back(std::move(entry));
    }
    return manifest;
}

GridPartPackageGovernanceResult ValidateGridPartPackageGovernance(const GridPartDocument& document,
                                                                  const GridPartCatalog& catalog,
                                                                  const GridPartPackageManifest& manifest,
                                                                  const GridPartReadinessEvidence& evidence) {
    GridPartPackageGovernanceResult result;
    result.dependencies = CollectGridPartDependencies(document, catalog);

    const auto validation = ValidateGridPartDocument(document, catalog);
    result.diagnostics = validation.diagnostics;

    if (manifest.package_id.empty()) {
        result.diagnostics.push_back(makePackageDiagnostic(GridPartSeverity::Blocker, "package_id_missing",
                                                          "Grid part package is missing a stable package id."));
    } else if (manifest.package_id != document.mapId()) {
        result.diagnostics.push_back(makePackageDiagnostic(GridPartSeverity::Blocker, "package_id_mismatch",
                                                          "Grid part package id does not match the document map id."));
    }
    if (!manifest.permissions.can_export) {
        result.diagnostics.push_back(makePackageDiagnostic(GridPartSeverity::Blocker, "package_export_forbidden",
                                                          "Creator package permissions do not allow export."));
    }
    if (manifest.permissions.contains_private_assets && manifest.permissions.can_redistribute) {
        result.diagnostics.push_back(makePackageDiagnostic(
            GridPartSeverity::Blocker, "package_redistribution_forbidden",
            "Creator package marks private assets while also allowing redistribution."));
    }

    for (size_t leftIndex = 0; leftIndex < manifest.dependencies.size(); ++leftIndex) {
        for (size_t rightIndex = leftIndex + 1; rightIndex < manifest.dependencies.size(); ++rightIndex) {
            if (manifest.dependencies[leftIndex].type == manifest.dependencies[rightIndex].type &&
                manifest.dependencies[leftIndex].id == manifest.dependencies[rightIndex].id) {
                GridPartDependency dependency;
                dependency.type = manifest.dependencies[leftIndex].type;
                dependency.id = manifest.dependencies[leftIndex].id;
                result.diagnostics.push_back(makePackageDiagnostic(
                    GridPartSeverity::Blocker, "package_dependency_duplicate",
                    "Creator package manifest contains duplicate dependency entries.", &dependency));
            }
        }
    }

    for (const auto& entry : manifest.dependencies) {
        GridPartDependency dependency;
        dependency.type = entry.type;
        dependency.id = entry.id;

        if (isBlank(entry.id)) {
            result.diagnostics.push_back(makePackageDiagnostic(
                GridPartSeverity::Blocker, "package_dependency_invalid",
                "Creator package manifest contains a dependency with a blank id.", &dependency));
            continue;
        }
        if (!manifestEntryIsReferenced(entry, result.dependencies)) {
            result.diagnostics.push_back(makePackageDiagnostic(
                GridPartSeverity::Error, "package_dependency_unreferenced",
                "Creator package manifest contains a dependency not referenced by the grid part document.",
                &dependency));
        }
        if (entry.private_asset) {
            result.diagnostics.push_back(makePackageDiagnostic(GridPartSeverity::Blocker,
                                                              "package_contains_private_asset",
                                                              "Creator package includes a private asset dependency.",
                                                              &dependency));
        }
        if (manifest.permissions.can_redistribute && !entry.redistribution_allowed) {
            result.diagnostics.push_back(makePackageDiagnostic(GridPartSeverity::Blocker,
                                                              "package_redistribution_forbidden",
                                                              "Creator package dependency forbids redistribution.",
                                                              &dependency));
        }
    }

    for (const auto& dependency : result.dependencies) {
        const auto* entry = findManifestEntry(dependency, manifest);
        if (entry == nullptr || !entry->available) {
            result.diagnostics.push_back(makePackageDiagnostic(GridPartSeverity::Error, "package_dependency_missing",
                                                              "Creator package dependency is missing.", &dependency));
            continue;
        }

        if (!entry->approved_for_export) {
            result.diagnostics.push_back(makePackageDiagnostic(GridPartSeverity::Error,
                                                              "package_uses_unapproved_asset",
                                                              "Creator package uses an asset not approved for export.",
                                                              &dependency));
        }
        if (manifest.permissions.requires_original_assets && dependencyNeedsLicenseEvidence(entry->type) &&
            isBlank(entry->license_evidence)) {
            result.diagnostics.push_back(makePackageDiagnostic(GridPartSeverity::Error,
                                                              "package_missing_license_evidence",
                                                              "Creator package dependency is missing license evidence.",
                                                              &dependency));
        }
    }

    sortDiagnostics(result.diagnostics);
    result.readiness = determineReadiness(result.diagnostics, manifest.permissions, evidence);
    return result;
}

} // namespace urpg::map
