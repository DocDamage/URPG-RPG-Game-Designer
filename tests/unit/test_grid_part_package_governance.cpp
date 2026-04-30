#include "engine/core/map/grid_part_package_governance.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <string>
#include <utility>

using namespace urpg::map;

namespace {

GridPartDefinition makeDefinition(std::string partId, GridPartCategory category = GridPartCategory::Prop) {
    GridPartDefinition definition;
    definition.part_id = std::move(partId);
    definition.display_name = definition.part_id;
    definition.description = "Package governance test part";
    definition.category = category;
    definition.default_layer = GridPartLayer::Object;
    definition.footprint.width = 1;
    definition.footprint.height = 1;
    definition.asset_id = definition.part_id + ".asset";
    definition.supported_rulesets = {GridPartRuleset::TopDownJRPG};
    return definition;
}

PlacedPartInstance makePart(std::string instanceId, std::string partId, GridPartCategory category = GridPartCategory::Prop) {
    PlacedPartInstance part;
    part.instance_id = std::move(instanceId);
    part.part_id = std::move(partId);
    part.category = category;
    part.layer = GridPartLayer::Object;
    part.grid_x = 1;
    part.grid_y = 1;
    return part;
}

GridPartReadinessEvidence publishableEvidence() {
    GridPartReadinessEvidence evidence;
    evidence.has_player_spawn = true;
    evidence.has_objective = true;
    evidence.reachability_passed = true;
    return evidence;
}

GridPartPackageManifest licensedManifest(const GridPartDocument& document, const GridPartCatalog& catalog) {
    auto manifest = BuildGridPartPackageManifest(document, catalog);
    for (auto& dependency : manifest.dependencies) {
        dependency.license_evidence = "licenses/" + dependency.id + ".txt";
    }
    return manifest;
}

bool hasCode(const std::vector<GridPartDiagnostic>& diagnostics, const std::string& code) {
    return std::any_of(diagnostics.begin(), diagnostics.end(),
                       [&](const GridPartDiagnostic& diagnostic) { return diagnostic.code == code; });
}

} // namespace

TEST_CASE("Grid part package governance builds dependency manifest entries",
          "[grid_part][package_governance]") {
    GridPartCatalog catalog;
    auto prop = makeDefinition("prop.crate");
    prop.prefab_path = "prefabs/crate.json";
    REQUIRE(catalog.addDefinition(prop));

    GridPartDocument document("map001", 8, 8);
    REQUIRE(document.placePart(makePart("map001:prop.crate:1:1", "prop.crate")));

    const auto manifest = BuildGridPartPackageManifest(document, catalog);

    REQUIRE(manifest.package_id == "map001");
    REQUIRE(manifest.dependencies.size() == 2);
    REQUIRE(manifest.dependencies[0].type == GridPartDependencyType::Asset);
    REQUIRE(manifest.dependencies[0].id == "prop.crate.asset");
    REQUIRE(manifest.dependencies[1].type == GridPartDependencyType::Prefab);
    REQUIRE(manifest.dependencies[1].id == "prefabs/crate.json");
}

TEST_CASE("Grid part package governance de-duplicates shared manifest dependencies",
          "[grid_part][package_governance]") {
    GridPartCatalog catalog;
    auto first = makeDefinition("prop.crate.a");
    first.asset_id = "shared.crate.asset";
    REQUIRE(catalog.addDefinition(first));
    auto second = makeDefinition("prop.crate.b");
    second.asset_id = "shared.crate.asset";
    REQUIRE(catalog.addDefinition(second));

    GridPartDocument document("map001", 8, 8);
    REQUIRE(document.placePart(makePart("map001:prop.crate.a:1:1", "prop.crate.a")));
    REQUIRE(document.placePart(makePart("map001:prop.crate.b:2:1", "prop.crate.b")));

    const auto manifest = BuildGridPartPackageManifest(document, catalog);

    REQUIRE(manifest.dependencies.size() == 1);
    REQUIRE(manifest.dependencies[0].type == GridPartDependencyType::Asset);
    REQUIRE(manifest.dependencies[0].id == "shared.crate.asset");
}

TEST_CASE("Grid part package governance rejects a manifest for a different map",
          "[grid_part][package_governance]") {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeDefinition("prop.crate")));

    GridPartDocument document("map001", 8, 8);
    REQUIRE(document.placePart(makePart("map001:prop.crate:1:1", "prop.crate")));
    auto manifest = licensedManifest(document, catalog);
    manifest.package_id = "map999";

    const auto result = ValidateGridPartPackageGovernance(document, catalog, manifest, publishableEvidence());

    REQUIRE(result.readiness == GridPartReadinessLevel::Draft);
    REQUIRE(hasCode(result.diagnostics, "package_id_mismatch"));
}

TEST_CASE("Grid part package governance rejects duplicate user manifest dependencies",
          "[grid_part][package_governance]") {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeDefinition("prop.crate")));

    GridPartDocument document("map001", 8, 8);
    REQUIRE(document.placePart(makePart("map001:prop.crate:1:1", "prop.crate")));
    auto manifest = licensedManifest(document, catalog);
    manifest.dependencies.push_back(manifest.dependencies.front());

    const auto result = ValidateGridPartPackageGovernance(document, catalog, manifest, publishableEvidence());

    REQUIRE(result.readiness == GridPartReadinessLevel::Draft);
    REQUIRE(hasCode(result.diagnostics, "package_dependency_duplicate"));
}

TEST_CASE("Grid part package governance rejects blank user manifest dependency ids",
          "[grid_part][package_governance]") {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeDefinition("prop.crate")));

    GridPartDocument document("map001", 8, 8);
    REQUIRE(document.placePart(makePart("map001:prop.crate:1:1", "prop.crate")));
    auto manifest = licensedManifest(document, catalog);
    manifest.dependencies[0].id = "  ";

    const auto result = ValidateGridPartPackageGovernance(document, catalog, manifest, publishableEvidence());

    REQUIRE(result.readiness == GridPartReadinessLevel::Draft);
    REQUIRE(hasCode(result.diagnostics, "package_dependency_invalid"));
}

TEST_CASE("Grid part package governance rejects unreferenced user manifest dependencies",
          "[grid_part][package_governance]") {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeDefinition("prop.crate")));

    GridPartDocument document("map001", 8, 8);
    REQUIRE(document.placePart(makePart("map001:prop.crate:1:1", "prop.crate")));
    auto manifest = licensedManifest(document, catalog);

    GridPartDependencyManifestEntry extra;
    extra.type = GridPartDependencyType::Asset;
    extra.id = "private.unused.asset";
    extra.private_asset = true;
    extra.license_evidence = "licenses/private.unused.asset.txt";
    manifest.dependencies.push_back(extra);

    const auto result = ValidateGridPartPackageGovernance(document, catalog, manifest, publishableEvidence());

    REQUIRE(result.readiness == GridPartReadinessLevel::Draft);
    REQUIRE(hasCode(result.diagnostics, "package_dependency_unreferenced"));
    REQUIRE(hasCode(result.diagnostics, "package_contains_private_asset"));
}

TEST_CASE("Grid part package governance blocks missing dependencies and missing license evidence",
          "[grid_part][package_governance]") {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeDefinition("prop.crate")));

    GridPartDocument document("map001", 8, 8);
    REQUIRE(document.placePart(makePart("map001:prop.crate:1:1", "prop.crate")));

    GridPartPackageManifest manifest;
    manifest.package_id = "map001";

    auto result = ValidateGridPartPackageGovernance(document, catalog, manifest, publishableEvidence());
    REQUIRE(result.readiness == GridPartReadinessLevel::Playable);
    REQUIRE_FALSE(result.canPublish());
    REQUIRE(hasCode(result.diagnostics, "package_dependency_missing"));

    manifest = BuildGridPartPackageManifest(document, catalog);
    result = ValidateGridPartPackageGovernance(document, catalog, manifest, publishableEvidence());
    REQUIRE(hasCode(result.diagnostics, "package_missing_license_evidence"));

    manifest.dependencies[0].license_evidence = "   ";
    result = ValidateGridPartPackageGovernance(document, catalog, manifest, publishableEvidence());
    REQUIRE(hasCode(result.diagnostics, "package_missing_license_evidence"));
}

TEST_CASE("Grid part package governance blocks private and forbidden redistribution assets",
          "[grid_part][package_governance]") {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeDefinition("prop.crate")));

    GridPartDocument document("map001", 8, 8);
    REQUIRE(document.placePart(makePart("map001:prop.crate:1:1", "prop.crate")));

    auto manifest = licensedManifest(document, catalog);
    manifest.permissions.can_redistribute = true;
    manifest.dependencies[0].redistribution_allowed = false;

    auto result = ValidateGridPartPackageGovernance(document, catalog, manifest, publishableEvidence());
    REQUIRE(result.readiness == GridPartReadinessLevel::Draft);
    REQUIRE(hasCode(result.diagnostics, "package_redistribution_forbidden"));

    manifest = licensedManifest(document, catalog);
    manifest.dependencies[0].private_asset = true;
    result = ValidateGridPartPackageGovernance(document, catalog, manifest, publishableEvidence());
    REQUIRE(result.readiness == GridPartReadinessLevel::Draft);
    REQUIRE(hasCode(result.diagnostics, "package_contains_private_asset"));
}

TEST_CASE("Grid part package governance advances publish export and certification readiness",
          "[grid_part][package_governance]") {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeDefinition("prop.crate")));

    GridPartDocument document("map001", 8, 8);
    REQUIRE(document.placePart(makePart("map001:prop.crate:1:1", "prop.crate")));
    auto manifest = licensedManifest(document, catalog);

    auto evidence = publishableEvidence();
    auto result = ValidateGridPartPackageGovernance(document, catalog, manifest, evidence);
    REQUIRE(result.readiness == GridPartReadinessLevel::Publishable);
    REQUIRE(result.canPublish());
    REQUIRE_FALSE(result.canExport());

    evidence.target_export_checks_passed = true;
    result = ValidateGridPartPackageGovernance(document, catalog, manifest, evidence);
    REQUIRE(result.readiness == GridPartReadinessLevel::Exportable);
    REQUIRE(result.canExport());

    evidence.accessibility_checks_passed = true;
    evidence.performance_budget_passed = true;
    result = ValidateGridPartPackageGovernance(document, catalog, manifest, evidence);
    REQUIRE(result.readiness == GridPartReadinessLevel::Certified);
}
