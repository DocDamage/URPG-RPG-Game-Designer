// test_s31_template_evidence_snapshots.cpp
//
// S31-T09 — One end-to-end evidence snapshot per S31 template.
//
// Provides a single top-level evidence snapshot test per template introduced
// in Sprint 31. Each test verifies that the template's readiness record in
// content/readiness/readiness_status.json:
//   1. Is present and has the expected fields (id, status, bars, barEvidence).
//   2. Has non-empty barEvidence for all bars present in the bars map.
//   3. Has a non-empty safeScope and at least one mainBlocker.
//
// These snapshot tests serve as a governance contract: if a template is
// removed or its evidence is stripped, the test fails with a clear signal.

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>
#include <fstream>
#include <string>
#include <vector>

using nlohmann::json;

namespace {

// Load the readiness_status.json relative to the build working directory.
// In CI, tests run from the build root, so we try common relative paths.
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

json findTemplate(const json& readiness, const std::string& templateId) {
    if (!readiness.contains("templates")) return json{};
    for (const auto& tmpl : readiness["templates"]) {
        if (tmpl.value("id", "") == templateId) {
            return tmpl;
        }
    }
    return json{};
}

void assertTemplateEvidenceSnapshot(
    const json& readiness,
    const std::string& templateId,
    const std::string& expectedStatus)
{
    const json tmpl = findTemplate(readiness, templateId);
    INFO("Template: " << templateId);
    REQUIRE(!tmpl.empty());
    REQUIRE(tmpl.value("id", "") == templateId);
    REQUIRE(tmpl.value("status", "") == expectedStatus);
    REQUIRE(tmpl.contains("bars"));
    REQUIRE(tmpl.contains("barEvidence"));

    // Every bar that exists in "bars" must have a corresponding non-empty
    // entry in "barEvidence".
    for (const auto& [bar, statusVal] : tmpl["bars"].items()) {
        INFO("Bar: " << bar);
        REQUIRE(tmpl["barEvidence"].contains(bar));
        REQUIRE(!tmpl["barEvidence"][bar].get<std::string>().empty());
    }

    REQUIRE(!tmpl.value("safeScope", "").empty());
    REQUIRE(tmpl.contains("mainBlockers"));
    REQUIRE(!tmpl["mainBlockers"].empty());
}

} // namespace

TEST_CASE("tactics_rpg: readiness record has evidence snapshot for all bars",
          "[template][snapshot][s31t09]") {
    const json readiness = loadReadinessStatus();
    assertTemplateEvidenceSnapshot(readiness, "tactics_rpg", "READY");
}

TEST_CASE("arpg: readiness record has evidence snapshot for all bars",
          "[template][snapshot][s31t09]") {
    const json readiness = loadReadinessStatus();
    assertTemplateEvidenceSnapshot(readiness, "arpg", "READY");
}

TEST_CASE("monster_collector_rpg: readiness record has evidence snapshot for all bars",
          "[template][snapshot][s31t09]") {
    const json readiness = loadReadinessStatus();
    assertTemplateEvidenceSnapshot(readiness, "monster_collector_rpg", "READY");
}

TEST_CASE("cozy_life_rpg: readiness record has evidence snapshot for all bars",
          "[template][snapshot][s31t09]") {
    const json readiness = loadReadinessStatus();
    assertTemplateEvidenceSnapshot(readiness, "cozy_life_rpg", "READY");
}

TEST_CASE("metroidvania_lite: readiness record has evidence snapshot for all bars",
          "[template][snapshot][s31t09]") {
    const json readiness = loadReadinessStatus();
    assertTemplateEvidenceSnapshot(readiness, "metroidvania_lite", "READY");
}

TEST_CASE("2_5d_rpg: readiness record has evidence snapshot for all bars",
          "[template][snapshot][s31t09]") {
    const json readiness = loadReadinessStatus();
    assertTemplateEvidenceSnapshot(readiness, "2_5d_rpg", "READY");
}

TEST_CASE("all S31 templates have non-empty mainBlockers with specific wording",
          "[template][snapshot][s31t09]") {
    const json readiness = loadReadinessStatus();
    const std::vector<std::string> s31Templates = {
        "tactics_rpg", "arpg", "monster_collector_rpg",
        "cozy_life_rpg", "metroidvania_lite", "2_5d_rpg"
    };
    for (const auto& id : s31Templates) {
        INFO("Template: " << id);
        const json tmpl = findTemplate(readiness, id);
        REQUIRE(!tmpl.empty());
        REQUIRE(tmpl.contains("mainBlockers"));
        // Each template must have at least one blocker with a non-empty string
        bool hasNonEmptyBlocker = false;
        for (const auto& blocker : tmpl["mainBlockers"]) {
            if (!blocker.get<std::string>().empty()) {
                hasNonEmptyBlocker = true;
                break;
            }
        }
        REQUIRE(hasNonEmptyBlocker);
    }
}
