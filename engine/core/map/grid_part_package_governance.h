#pragma once

#include "engine/core/map/grid_part_dependency_graph.h"
#include "engine/core/map/grid_part_validator.h"

#include <cstdint>
#include <string>
#include <vector>

namespace urpg::map {

struct CreatorPackagePermissions {
    bool can_edit = true;
    bool can_remix = false;
    bool can_export = true;
    bool can_redistribute = false;
    bool requires_original_assets = true;
    bool contains_private_assets = false;
    bool contains_third_party_assets = false;
};

enum class GridPartReadinessLevel : uint8_t {
    Draft = 0,
    Playable,
    Validated,
    Publishable,
    Exportable,
    Certified,
};

struct GridPartDependencyManifestEntry {
    GridPartDependencyType type = GridPartDependencyType::Asset;
    std::string id;
    bool available = true;
    bool approved_for_export = true;
    bool redistribution_allowed = true;
    bool private_asset = false;
    bool third_party_asset = false;
    std::string license_evidence;
};

struct GridPartPackageManifest {
    std::string package_id;
    CreatorPackagePermissions permissions;
    std::vector<GridPartDependencyManifestEntry> dependencies;
};

struct GridPartReadinessEvidence {
    bool has_player_spawn = false;
    bool has_objective = false;
    bool reachability_passed = false;
    bool target_export_checks_passed = false;
    bool accessibility_checks_passed = false;
    bool performance_budget_passed = false;
    bool human_review_required = false;
    bool human_review_passed = false;
};

struct GridPartPackageGovernanceResult {
    GridPartReadinessLevel readiness = GridPartReadinessLevel::Draft;
    std::vector<GridPartDependency> dependencies;
    std::vector<GridPartDiagnostic> diagnostics;

    bool canPublish() const;
    bool canExport() const;
};

GridPartPackageManifest BuildGridPartPackageManifest(const GridPartDocument& document, const GridPartCatalog& catalog,
                                                     CreatorPackagePermissions permissions = {});

GridPartPackageGovernanceResult ValidateGridPartPackageGovernance(const GridPartDocument& document,
                                                                  const GridPartCatalog& catalog,
                                                                  const GridPartPackageManifest& manifest,
                                                                  const GridPartReadinessEvidence& evidence);

} // namespace urpg::map
