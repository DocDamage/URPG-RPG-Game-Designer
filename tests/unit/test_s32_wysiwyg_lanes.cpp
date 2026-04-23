// test_s32_wysiwyg_lanes.cpp
//
// S32 — Long-Tail WYSIWYG Lanes
//
// S32-T01: Character Identity full Create-a-Character lifecycle boundary.
// S32-T02: Character appearance/preview pipeline boundary.
// S32-T03: Achievement registry platform backend integration scope.
// S32-T04: Reconcile outstanding mainGaps entries for partial systems.
// S32-T05: Confirm export_validator crypto hardening roadmap in scope.
// S32-T06: Achievement trophy export pipeline scope wording.
//
// These tests verify that the readiness record accurately reflects the bounded
// scope of landed work and explicitly documents the remaining backlog. If the
// wording drifts back to vague placeholders or stale deferred claims, these
// tests fail.

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

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

json findSubsystem(const json& readiness, const std::string& subsystemId) {
    if (!readiness.contains("subsystems")) return json{};
    for (const auto& sub : readiness["subsystems"]) {
        if (sub.value("id", "") == subsystemId) {
            return sub;
        }
    }
    return json{};
}

bool hasGapContaining(const json& entry, const std::string& keyword) {
    if (!entry.contains("mainGaps")) return false;
    for (const auto& gap : entry["mainGaps"]) {
        if (gap.get<std::string>().find(keyword) != std::string::npos) {
            return true;
        }
    }
    return false;
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
    REQUIRE(sub.value("status", "") == "PARTIAL");
    REQUIRE(sub.value("summary", "").find("runtime creator screen") != std::string::npos);
}

TEST_CASE("character_identity: readiness record narrows remaining gaps to rules/persistence/compositor depth",
          "[wysiwyg][character][s32t02]") {
    const json readiness = loadReadinessStatus();
    const json sub = findSubsystem(readiness, "character_identity");

    REQUIRE(!sub.empty());
    REQUIRE(hasGapContaining(sub, "free-text naming"));
    REQUIRE(hasGapContaining(sub, "save/load persistence"));
    REQUIRE(hasGapContaining(sub, "asset compositor"));
    REQUIRE_FALSE(hasGapContaining(sub, "no creator-screen runtime"));
    REQUIRE_FALSE(hasGapContaining(sub, "appearance preview pipeline"));
}

TEST_CASE("character_identity: landed evidence flags are all set",
          "[wysiwyg][character][s32t01]") {
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

TEST_CASE("achievement_registry: readiness record acknowledges platform backend integration as out-of-tree",
          "[wysiwyg][achievement][s32t03]") {
    const json readiness = loadReadinessStatus();
    const json sub = findSubsystem(readiness, "achievement_registry");

    REQUIRE(!sub.empty());
    REQUIRE(sub.value("status", "") == "PARTIAL");
    // The mainGaps must explicitly state platform backend integration is out-of-tree
    REQUIRE(hasGapContaining(sub, "out-of-tree"));
}

// ============================================================================
// S32-T04 — Reconcile mainGaps entries for partial systems
// ============================================================================

TEST_CASE("mod_registry: mainGaps entries have specific wording (not generic future work)",
          "[wysiwyg][s32t04]") {
    const json readiness = loadReadinessStatus();
    const json sub = findSubsystem(readiness, "mod_registry");

    REQUIRE(!sub.empty());
    REQUIRE(sub.contains("mainGaps"));
    // Must not have a single catch-all "remain future work" gap
    bool hasVagueGap = false;
    for (const auto& gap : sub["mainGaps"]) {
        const auto& text = gap.get<std::string>();
        if (text == "Live mod loading, sandboxed script execution, and mod-store integration remain future work") {
            hasVagueGap = true;
        }
    }
    REQUIRE_FALSE(hasVagueGap);
    // Must have at least 3 specific gap entries
    REQUIRE(sub["mainGaps"].size() >= 3);
}

TEST_CASE("analytics_dispatcher: mainGaps entries have specific wording (not generic future work)",
          "[wysiwyg][s32t04]") {
    const json readiness = loadReadinessStatus();
    const json sub = findSubsystem(readiness, "analytics_dispatcher");

    REQUIRE(!sub.empty());
    REQUIRE(sub.contains("mainGaps"));
    bool hasVagueGap = false;
    for (const auto& gap : sub["mainGaps"]) {
        const auto& text = gap.get<std::string>();
        if (text == "Telemetry upload pipeline, session aggregation, and privacy audit workflow remain future work") {
            hasVagueGap = true;
        }
    }
    REQUIRE_FALSE(hasVagueGap);
    REQUIRE(sub["mainGaps"].size() >= 3);
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
    REQUIRE(sub["mainGaps"].size() >= 2);
}

// ============================================================================
// S32-T05 — Export validator signature-enforcement roadmap in scope
// ============================================================================

TEST_CASE("export_validator: mainGaps acknowledges runtime-side signature enforcement as mandatory pre-READY backlog",
          "[wysiwyg][export][s32t05]") {
    const json readiness = loadReadinessStatus();
    const json sub = findSubsystem(readiness, "export_validator");

    REQUIRE(!sub.empty());
    REQUIRE(sub.value("status", "") == "PARTIAL");
    // The mainGaps must still call out stronger runtime-side signature enforcement
    REQUIRE(hasGapContaining(sub, "signature"));
    REQUIRE(hasGapContaining(sub, "pre-READY"));
}

// ============================================================================
// S32-T06 — Achievement trophy export pipeline scope wording
// ============================================================================

TEST_CASE("achievement_registry: mainGaps acknowledges trophy export pipeline as deferred",
          "[wysiwyg][achievement][s32t06]") {
    const json readiness = loadReadinessStatus();
    const json sub = findSubsystem(readiness, "achievement_registry");

    REQUIRE(!sub.empty());
    REQUIRE(hasGapContaining(sub, "Trophy export pipeline"));
    REQUIRE(hasGapContaining(sub, "deferred"));
}

TEST_CASE("achievement_registry: landed evidence flags are all set",
          "[wysiwyg][achievement][s32t06]") {
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
