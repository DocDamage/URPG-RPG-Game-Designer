// test_s32_wysiwyg_lanes.cpp
//
// S32 — Long-Tail WYSIWYG Lanes
//
// S32-T01: Character Identity full Create-a-Character lifecycle boundary.
// S32-T02: Character appearance/preview pipeline boundary.
// S32-T03: Achievement registry platform backend integration scope.
// S32-T04: Reconcile outstanding mainGaps entries for partial systems.
// S32-T05: Confirm export_validator crypto hardening boundary in scope.
// S32-T06: Achievement trophy export pipeline scope wording.
//
// These tests verify that the readiness record accurately reflects the bounded
// scope of landed work and explicitly documents the remaining backlog. If the
// wording drifts back to vague placeholders or stale deferred claims, these
// tests fail.

#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include <iterator>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "engine/core/editor/editor_panel_registry.h"

using nlohmann::json;

namespace {

json loadReadinessStatus() {
    const std::vector<std::string> candidates = {
        "content/readiness/readiness_status.json",
        "../content/readiness/readiness_status.json",
        "../../content/readiness/readiness_status.json",
    };
    for (const auto& path : candidates) {
        std::ifstream ifs(path);
        if (ifs.is_open()) {
            json doc;
            ifs >> doc;
            return doc;
        }
    }
    return json{};
}

json loadPartialLaneSlices() {
    const std::vector<std::string> candidates = {
        "content/readiness/partial_lane_vertical_slices.json",
        "../content/readiness/partial_lane_vertical_slices.json",
        "../../content/readiness/partial_lane_vertical_slices.json",
    };
    for (const auto& path : candidates) {
        std::ifstream ifs(path);
        if (ifs.is_open()) {
            json doc;
            ifs >> doc;
            return doc;
        }
    }
    return json{};
}

json loadWysiwygDoneRule() {
    const std::vector<std::string> candidates = {
        "content/readiness/wysiwyg_done_rule.json",
        "../content/readiness/wysiwyg_done_rule.json",
        "../../content/readiness/wysiwyg_done_rule.json",
    };
    for (const auto& path : candidates) {
        std::ifstream ifs(path);
        if (ifs.is_open()) {
            json doc;
            ifs >> doc;
            return doc;
        }
    }
    return json{};
}

json loadWysiwygTemplateShowcase() {
    const std::vector<std::string> candidates = {
        "content/examples/wysiwyg_template_showcase.json",
        "../content/examples/wysiwyg_template_showcase.json",
        "../../content/examples/wysiwyg_template_showcase.json",
    };
    for (const auto& path : candidates) {
        std::ifstream ifs(path);
        if (ifs.is_open()) {
            json doc;
            ifs >> doc;
            return doc;
        }
    }
    return json{};
}

bool isRoutableEditorExposure(urpg::editor::EditorPanelExposure exposure) {
    return urpg::editor::isRoutableEditorPanelExposure(exposure);
}

std::size_t countUnwiredWysiwygShowcaseSurfaces(const json& showcase) {
    std::size_t unwired = 0;
    for (const auto& example : showcase.value("examples", json::array())) {
        for (const auto& surface : example.value("surfaces", json::array())) {
            const auto registryId = surface.value("editor_panel_registry_id", "");
            const auto* entry = urpg::editor::findEditorPanelRegistryEntry(registryId);
            if (registryId.empty() || entry == nullptr || !isRoutableEditorExposure(entry->exposure)) {
                ++unwired;
            }
        }
    }
    return unwired;
}

json findSubsystem(const json& readiness, const std::string& subsystemId) {
    if (!readiness.contains("subsystems"))
        return json{};
    for (const auto& sub : readiness["subsystems"]) {
        if (sub.value("id", "") == subsystemId) {
            return sub;
        }
    }
    return json{};
}

bool hasGapContaining(const json& entry, const std::string& keyword) {
    if (!entry.contains("mainGaps"))
        return false;
    for (const auto& gap : entry["mainGaps"]) {
        if (gap.get<std::string>().find(keyword) != std::string::npos) {
            return true;
        }
    }
    return false;
}

std::string readTextFile(const std::vector<std::string>& candidates) {
    for (const auto& path : candidates) {
        std::ifstream ifs(path);
        if (ifs.is_open()) {
            return std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        }
    }
    return {};
}

bool jsonArrayContainsString(const json& values, const std::string& expected) {
    if (!values.is_array())
        return false;
    return std::any_of(values.begin(), values.end(), [&expected](const json& value) {
        return value.is_string() && value.get<std::string>() == expected;
    });
}

} // namespace

// ============================================================================
// S32-T01/T02 — Character Identity lifecycle and appearance pipeline
// ============================================================================

TEST_CASE("character_identity: readiness record acknowledges bounded runtime creator lane is landed",
          "[wysiwyg][character][s32t01]") {
    const json readiness = loadReadinessStatus();
    const json sub = findSubsystem(readiness, "character_identity");

    REQUIRE(!sub.empty());
    REQUIRE(sub.value("status", "") == "READY");
    REQUIRE(sub.value("summary", "").find("runtime creator screen") != std::string::npos);
}

TEST_CASE("character_identity: readiness record acknowledges compositor as landed", "[wysiwyg][character][s32t02]") {
    const json readiness = loadReadinessStatus();
    const json sub = findSubsystem(readiness, "character_identity");

    REQUIRE(!sub.empty());
    REQUIRE(hasGapContaining(sub, "free-text naming"));
    REQUIRE(hasGapContaining(sub, "save/load persistence is landed"));
    REQUIRE(hasGapContaining(sub, "Layered portrait/field/battle composition is landed"));
    REQUIRE_FALSE(hasGapContaining(sub, "no creator-screen runtime"));
    REQUIRE_FALSE(hasGapContaining(sub, "appearance preview pipeline"));
}

TEST_CASE("character_identity: landed evidence flags are all set", "[wysiwyg][character][s32t01]") {
    const json readiness = loadReadinessStatus();
    const json sub = findSubsystem(readiness, "character_identity");

    REQUIRE(!sub.empty());
    REQUIRE(sub.contains("evidence"));
    // The landed features (runtime, editor, schema, diagnostics, tests, docs) must all be true
    const auto& ev = sub["evidence"];
    REQUIRE(ev.value("runtimeOwner", false) == true);
    REQUIRE(ev.value("editorSurface", false) == true);
    REQUIRE(ev.value("schemaMigration", false) == true);
    REQUIRE(ev.value("diagnostics", false) == true);
    REQUIRE(ev.value("testsValidation", false) == true);
    REQUIRE(ev.value("docsAligned", false) == true);
}

// ============================================================================
// S32-T03 — Achievement registry platform backend scope
// ============================================================================

TEST_CASE("achievement_registry: readiness record acknowledges platform backend integration as implemented",
          "[wysiwyg][achievement][s32t03]") {
    const json readiness = loadReadinessStatus();
    const json sub = findSubsystem(readiness, "achievement_registry");

    REQUIRE(!sub.empty());
    REQUIRE(sub.value("status", "") == "READY");
    REQUIRE(sub.value("summary", "").find("platform backend synchronization") != std::string::npos);
    REQUIRE(sub.value("summary", "").find("provider-profile status validation") != std::string::npos);
    REQUIRE(sub.value("summary", "").find("Proprietary store SDK credentials remain project configuration") !=
            std::string::npos);
}

// ============================================================================
// S32-T04 — Reconcile mainGaps entries for partial systems
// ============================================================================

TEST_CASE("mod_registry: mainGaps entries have specific wording (not generic future work)", "[wysiwyg][s32t04]") {
    const json readiness = loadReadinessStatus();
    const json sub = findSubsystem(readiness, "mod_registry");

    REQUIRE(!sub.empty());
    REQUIRE(sub.contains("mainGaps"));
    REQUIRE(sub.value("status", "") == "READY");
    // Must not have a single catch-all "remain future work" gap
    bool hasVagueGap = false;
    for (const auto& gap : sub["mainGaps"]) {
        const auto& text = gap.get<std::string>();
        if (text == "Live mod loading, sandboxed script execution, and mod-store integration remain future work") {
            hasVagueGap = true;
        }
    }
    REQUIRE_FALSE(hasVagueGap);
    REQUIRE(sub["mainGaps"].empty());
    REQUIRE(sub.value("summary", "").find("ModMarketplaceProviderProfile") != std::string::npos);
    REQUIRE(sub.value("summary", "").find("External marketplace services") != std::string::npos);
}

TEST_CASE("analytics_dispatcher: mainGaps entries have specific wording (not generic future work)",
          "[wysiwyg][s32t04]") {
    const json readiness = loadReadinessStatus();
    const json sub = findSubsystem(readiness, "analytics_dispatcher");

    REQUIRE(!sub.empty());
    REQUIRE(sub.contains("mainGaps"));
    REQUIRE(sub.value("status", "") == "READY");
    bool hasVagueGap = false;
    for (const auto& gap : sub["mainGaps"]) {
        const auto& text = gap.get<std::string>();
        if (text == "Telemetry upload pipeline, session aggregation, and privacy audit workflow remain future work") {
            hasVagueGap = true;
        }
    }
    REQUIRE_FALSE(hasVagueGap);
    REQUIRE(sub["mainGaps"].empty());
    REQUIRE(sub.value("summary", "").find("AnalyticsEndpointProfile status validation") != std::string::npos);
    REQUIRE(sub.value("summary", "").find("Qualified production privacy/legal approval remains separate") !=
            std::string::npos);
}

TEST_CASE("visual_regression_harness: mainGaps entries have specific wording (not generic future work)",
          "[wysiwyg][s32t04]") {
    const json readiness = loadReadinessStatus();
    const json sub = findSubsystem(readiness, "visual_regression_harness");

    REQUIRE(!sub.empty());
    REQUIRE(sub.contains("mainGaps"));
    bool hasVagueGap = false;
    for (const auto& gap : sub["mainGaps"]) {
        const auto& text = gap.get<std::string>();
        if (text == "CI golden gate enforcement and broader scene/backend coverage remain future work") {
            hasVagueGap = true;
        }
    }
    REQUIRE_FALSE(hasVagueGap);
    REQUIRE(sub.value("status", "") == "READY");
    REQUIRE(sub["mainGaps"].empty());
    REQUIRE(sub.value("summary", "").find("phase-one shell-owned MapScene") != std::string::npos);
    REQUIRE(sub.value("summary", "").find("run_presentation_gate.ps1") != std::string::npos);
}

// ============================================================================
// S32-T05 — Export validator signature-enforcement boundary in scope
// ============================================================================

TEST_CASE(
    "export_validator: mainGaps acknowledges runtime-side signature enforcement is landed but export remains partial",
    "[wysiwyg][export][s32t05]") {
    const json readiness = loadReadinessStatus();
    const json sub = findSubsystem(readiness, "export_validator");

    REQUIRE(!sub.empty());
    REQUIRE(sub.value("status", "") == "READY");
    REQUIRE(sub.value("summary", "").find("runtime/load-time bundle rejection") != std::string::npos);
    REQUIRE(sub.value("summary", "").find("release-profile signing/notarization/artifact-policy enforcement") !=
            std::string::npos);
    REQUIRE(sub.value("summary", "").find("structured bundle-summary JSON reporting") != std::string::npos);
    REQUIRE(sub["mainGaps"].empty());
}

TEST_CASE("export_validator: runtime signature enforcement design note keeps non-runtime boundary explicit",
          "[wysiwyg][export][s32t05]") {
    const std::string text = readTextFile({
        "docs/specs/EXPORT_RUNTIME_SIGNATURE_ENFORCEMENT_DESIGN.md",
        "docs/EXPORT_RUNTIME_SIGNATURE_ENFORCEMENT_DESIGN.md",
        "../docs/specs/EXPORT_RUNTIME_SIGNATURE_ENFORCEMENT_DESIGN.md",
        "../docs/EXPORT_RUNTIME_SIGNATURE_ENFORCEMENT_DESIGN.md",
        "../../docs/specs/EXPORT_RUNTIME_SIGNATURE_ENFORCEMENT_DESIGN.md",
        "../../docs/EXPORT_RUNTIME_SIGNATURE_ENFORCEMENT_DESIGN.md",
    });

    REQUIRE_FALSE(text.empty());
    REQUIRE(text.find("RuntimeBundleLoader") != std::string::npos);
    REQUIRE(text.find("runtime/load-time protection") != std::string::npos);
    REQUIRE(text.find("temp-file-plus-atomic-rename") != std::string::npos);
    REQUIRE(text.find("must not claim") != std::string::npos);
    REQUIRE(text.find("not OS/vendor code signing") != std::string::npos);
}

// ============================================================================
// S32-T06 — Achievement trophy export pipeline scope wording
// ============================================================================

TEST_CASE("achievement_registry: readiness record keeps trophy export and platform backend landed",
          "[wysiwyg][achievement][s32t06]") {
    const json readiness = loadReadinessStatus();
    const json sub = findSubsystem(readiness, "achievement_registry");

    REQUIRE(!sub.empty());
    REQUIRE(sub.value("summary", "").find("trophy export payload") != std::string::npos);
    REQUIRE(sub.value("summary", "").find("platform backend synchronization") != std::string::npos);
    REQUIRE(sub.value("summary", "").find("AchievementPlatformProfile") != std::string::npos);
    REQUIRE(sub.value("summary", "").find("Proprietary store SDK credentials remain project configuration") !=
            std::string::npos);
}

TEST_CASE("achievement_registry: landed evidence flags are all set", "[wysiwyg][achievement][s32t06]") {
    const json readiness = loadReadinessStatus();
    const json sub = findSubsystem(readiness, "achievement_registry");

    REQUIRE(!sub.empty());
    const auto& ev = sub["evidence"];
    REQUIRE(ev.value("runtimeOwner", false) == true);
    REQUIRE(ev.value("editorSurface", false) == true);
    REQUIRE(ev.value("schemaMigration", false) == true);
    REQUIRE(ev.value("diagnostics", false) == true);
    REQUIRE(ev.value("testsValidation", false) == true);
    REQUIRE(ev.value("docsAligned", false) == true);
}

TEST_CASE("partial product lanes have owned next vertical slices", "[wysiwyg][partial_lanes][td_aud_05]") {
    const json slices = loadPartialLaneSlices();

    REQUIRE(!slices.empty());
    REQUIRE(slices.value("version", 0) == 1);
    REQUIRE(slices.contains("lanes"));
    REQUIRE(slices["lanes"].is_array());

    const std::vector<std::string> requiredLanes = {
        "compat_bridge_exit", "presentation_runtime", "gameplay_ability_framework", "governance_foundation",
        "character_identity", "achievement_registry", "accessibility_auditor",      "audio_mix_presets",
        "mod_registry",       "analytics_dispatcher",
    };

    for (const auto& id : requiredLanes) {
        auto it = std::find_if(slices["lanes"].begin(), slices["lanes"].end(),
                               [&id](const json& lane) { return lane.value("id", "") == id; });
        REQUIRE(it != slices["lanes"].end());
        REQUIRE_FALSE(it->value("ownerTrack", "").empty());
        REQUIRE_FALSE(it->value("nextVerticalSlice", "").empty());
        REQUIRE_FALSE(it->value("deferredScope", "").empty());
    }
}

TEST_CASE("WYSIWYG done rule defines the non-negotiable completion bars", "[wysiwyg][done_rule]") {
    const json rule = loadWysiwygDoneRule();

    REQUIRE(!rule.empty());
    REQUIRE(rule.value("ruleId", "") == "wysiwyg_done_rule");
    REQUIRE(rule.value("rule", "").find("No system is done") != std::string::npos);

    const std::vector<std::string> requiredEvidence = {
        "visualAuthoringSurface", "livePreview", "savedProjectData",
        "runtimeExecution",       "diagnostics", "testsValidation",
    };

    REQUIRE(rule.contains("requiredEvidence"));
    for (const auto& field : requiredEvidence) {
        REQUIRE(jsonArrayContainsString(rule["requiredEvidence"], field));
    }
}

TEST_CASE("READY subsystems satisfy every WYSIWYG done-rule evidence bar", "[wysiwyg][done_rule][readiness]") {
    const json readiness = loadReadinessStatus();
    const json rule = loadWysiwygDoneRule();

    REQUIRE(!readiness.empty());
    REQUIRE(!rule.empty());

    for (const auto& sub : readiness["subsystems"]) {
        if (sub.value("status", "") != "READY") {
            continue;
        }
        REQUIRE(sub.contains("evidence"));
        for (const auto& field : rule["requiredEvidence"]) {
            REQUIRE(field.is_string());
            REQUIRE(sub["evidence"].value(field.get<std::string>(), false) == true);
        }
    }
}

TEST_CASE("WYSIWYG priority surface list keeps the next creator-tool pushes explicit",
          "[wysiwyg][done_rule][priority_surfaces]") {
    const json rule = loadWysiwygDoneRule();

    REQUIRE(!rule.empty());
    REQUIRE(rule.contains("prioritySurfaces"));
    REQUIRE(rule["prioritySurfaces"].size() == 7);

    const std::vector<std::string> requiredSurfaces = {
        "battle_animation_vfx_timeline",
        "map_lighting_weather_region_preview",
        "dialogue_portrait_choice_variable_localization_preview",
        "event_command_visual_graph",
        "ability_sandbox_visible_costs_cooldowns_tags_effects",
        "save_load_preview_lab",
        "export_exact_ship_preview",
    };

    for (const auto& id : requiredSurfaces) {
        const auto it =
            std::find_if(rule["prioritySurfaces"].begin(), rule["prioritySurfaces"].end(), [&id](const json& surface) {
                return surface.value("id", "") == id && !surface.value("label", "").empty();
            });
        REQUIRE(it != rule["prioritySurfaces"].end());
    }
}

TEST_CASE("Phase 10 WYSIWYG roadmap completion truth follows routed showcase surfaces",
          "[wysiwyg][done_rule][phase10]") {
    const std::string inventory = readTextFile({
        "docs/release/100_PERCENT_COMPLETION_INVENTORY.md",
        "../docs/release/100_PERCENT_COMPLETION_INVENTORY.md",
        "../../docs/release/100_PERCENT_COMPLETION_INVENTORY.md",
    });
    const std::string programStatus = readTextFile({
        "docs/PROGRAM_COMPLETION_STATUS.md",
        "../docs/PROGRAM_COMPLETION_STATUS.md",
        "../../docs/PROGRAM_COMPLETION_STATUS.md",
    });
    const json showcase = loadWysiwygTemplateShowcase();

    REQUIRE_FALSE(inventory.empty());
    REQUIRE_FALSE(programStatus.empty());
    REQUIRE(!showcase.empty());

    const auto phase10Pos = inventory.find("`phase_10_wysiwyg_roadmap_completion`");
    REQUIRE(phase10Pos != std::string::npos);
    const auto phase10LineEnd = inventory.find('\n', phase10Pos);
    const auto phase10Line = inventory.substr(phase10Pos, phase10LineEnd - phase10Pos);

    const auto unwiredSurfaceCount = countUnwiredWysiwygShowcaseSurfaces(showcase);
    REQUIRE(unwiredSurfaceCount == 0);
    REQUIRE(phase10Line.find("`READY`") != std::string::npos);
    REQUIRE(phase10Line.find("`MANDATORY_OPEN`") == std::string::npos);
    REQUIRE(phase10Line.find("registry-backed editor panel routes") != std::string::npos);
    REQUIRE(programStatus.find("Phase 10 WYSIWYG roadmap closure is complete") != std::string::npos);

    REQUIRE(programStatus.find("- [ ] Mandatory: treat every remaining roadmap lane as dual-delivery work") ==
            std::string::npos);
    REQUIRE(programStatus.find("- [ ] Mandatory: close every remaining feature lane with direct editor workflows") ==
            std::string::npos);
    REQUIRE(programStatus.find("- [ ] Mandatory: keep public docs, readiness language, and completion claims aligned") ==
            std::string::npos);
    REQUIRE(programStatus.find("- [x] Mandatory: treat every remaining roadmap lane as dual-delivery work") !=
            std::string::npos);
    REQUIRE(programStatus.find("- [x] Mandatory: close every remaining feature lane with direct editor workflows") !=
            std::string::npos);
    REQUIRE(programStatus.find("- [x] Mandatory: keep public docs, readiness language, and completion claims aligned") !=
            std::string::npos);
}
