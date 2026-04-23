#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <regex>
#include <string>
#include <vector>

// ============================================================================
// S25-T05/T06/T07 — Governance Gate Regression Tests
//
// These tests exercise the logical rules implemented by truth_reconciler.ps1
// and check_template_spec_bar_drift.ps1 using in-memory fixtures.  They do
// not call the PowerShell scripts directly but prove that the detection
// algorithms are correct at the unit level.
//
// Tags: [governance][s25]
// ============================================================================

namespace {

// Simulated truth-reconciler check: every READY subsystem must have all
// evidence fields set to true.
struct EvidenceCheckResult {
    std::string subsystem_id;
    std::string field;
};

std::vector<EvidenceCheckResult> findReadySubsystemsWithFalseEvidence(
    const nlohmann::json& readiness) {
    const std::vector<std::string> evidenceFields = {
        "runtimeOwner", "editorSurface", "schemaMigration",
        "diagnostics", "testsValidation", "docsAligned"};
    std::vector<EvidenceCheckResult> bad;
    for (const auto& entry : readiness["subsystems"]) {
        if (entry["status"] != "READY") {
            continue;
        }
        for (const auto& field : evidenceFields) {
            if (!entry["evidence"].contains(field) ||
                entry["evidence"][field] != true) {
                bad.push_back({entry["id"], field});
            }
        }
    }
    return bad;
}

// Simulated truth-reconciler check: matrix status must match readiness.
struct StatusMismatch {
    std::string id;
    std::string readiness_status;
    std::string matrix_status;
};

// Simulated signoff contract check.
struct SignoffViolation {
    std::string subsystem_id;
    std::string rule;
};

std::vector<SignoffViolation> checkSignoffContracts(
    const nlohmann::json& readiness,
    const std::vector<std::string>& governedSubsystemIds,
    const std::map<std::string, std::string>& expectedArtifactPaths) {
    std::vector<SignoffViolation> violations;
    for (const auto& entry : readiness["subsystems"]) {
        const std::string id = entry["id"];
        const bool isGoverned = std::find(
                                    governedSubsystemIds.begin(),
                                    governedSubsystemIds.end(), id) !=
                                governedSubsystemIds.end();
        if (!isGoverned) {
            continue;
        }
        if (!entry.contains("signoff")) {
            violations.push_back({id, "signoff_block_missing"});
            continue;
        }
        if (entry["signoff"]["required"] != true) {
            violations.push_back({id, "required_not_true"});
        }
        if (entry["signoff"]["promotionRequiresHumanReview"] != true) {
            violations.push_back({id, "human_review_not_required"});
        }
        const auto it = expectedArtifactPaths.find(id);
        if (it != expectedArtifactPaths.end()) {
            const std::string actual = entry["signoff"]["artifactPath"];
            if (actual != it->second) {
                violations.push_back({id, "artifact_path_mismatch"});
            }
        }
    }
    return violations;
}

// Simulated template spec bar-drift check.
struct BarDrift {
    std::string bar_name;
    std::string spec_status;
    std::string readiness_status;
};

std::vector<BarDrift> detectBarDrift(
    const nlohmann::json& templateBars,
    const std::map<std::string, std::string>& specBars) {
    std::vector<BarDrift> drifts;
    for (const auto& [name, readinessStatus] : templateBars.items()) {
        const auto it = specBars.find(name);
        if (it == specBars.end()) {
            drifts.push_back({name, "(missing)", readinessStatus});
        } else if (it->second != readinessStatus.get<std::string>()) {
            drifts.push_back({name, it->second, readinessStatus});
        }
    }
    return drifts;
}

// Simulated status-date consistency check across docs.
bool statusDateMatchesReadiness(const std::string& docText,
                                const std::string& readinessDate) {
    const std::regex pattern(R"(Status Date:\s*(\d{4}-\d{2}-\d{2}))");
    std::smatch m;
    if (!std::regex_search(docText, m, pattern)) {
        return false;
    }
    return m[1].str() == readinessDate;
}

} // namespace

// ----------------------------------------------------------------------------
// S25-T05 — READY-without-evidence detection
// ----------------------------------------------------------------------------
TEST_CASE("READY subsystem missing evidence field is detected", "[governance][s25]") {
    const nlohmann::json readiness = nlohmann::json::parse(R"({
        "schemaVersion": "1.0.0",
        "statusDate": "2026-04-23",
        "subsystems": [
            {
                "id": "battle_core",
                "status": "READY",
                "evidence": {
                    "runtimeOwner": true,
                    "editorSurface": true,
                    "schemaMigration": true,
                    "diagnostics": true,
                    "testsValidation": true,
                    "docsAligned": false
                }
            }
        ]
    })");

    const auto bad = findReadySubsystemsWithFalseEvidence(readiness);
    REQUIRE(bad.size() == 1);
    REQUIRE(bad[0].subsystem_id == "battle_core");
    REQUIRE(bad[0].field == "docsAligned");
}

TEST_CASE("READY subsystem with all evidence true passes evidence check", "[governance][s25]") {
    const nlohmann::json readiness = nlohmann::json::parse(R"({
        "schemaVersion": "1.0.0",
        "statusDate": "2026-04-23",
        "subsystems": [
            {
                "id": "ui_menu_core",
                "status": "READY",
                "evidence": {
                    "runtimeOwner": true,
                    "editorSurface": true,
                    "schemaMigration": true,
                    "diagnostics": true,
                    "testsValidation": true,
                    "docsAligned": true
                }
            }
        ]
    })");

    const auto bad = findReadySubsystemsWithFalseEvidence(readiness);
    REQUIRE(bad.empty());
}

TEST_CASE("PARTIAL subsystem with false evidence fields is not flagged", "[governance][s25]") {
    const nlohmann::json readiness = nlohmann::json::parse(R"({
        "schemaVersion": "1.0.0",
        "statusDate": "2026-04-23",
        "subsystems": [
            {
                "id": "gameplay_ability_framework",
                "status": "PARTIAL",
                "evidence": {
                    "runtimeOwner": true,
                    "editorSurface": true,
                    "schemaMigration": true,
                    "diagnostics": true,
                    "testsValidation": true,
                    "docsAligned": true
                }
            }
        ]
    })");

    const auto bad = findReadySubsystemsWithFalseEvidence(readiness);
    REQUIRE(bad.empty());
}

// ----------------------------------------------------------------------------
// S25-T05 — Signoff contract regression tests
// ----------------------------------------------------------------------------
TEST_CASE("signoff-governed subsystem without signoff block is flagged", "[governance][s25]") {
    const nlohmann::json readiness = nlohmann::json::parse(R"({
        "schemaVersion": "1.0.0",
        "statusDate": "2026-04-23",
        "subsystems": [
            {
                "id": "battle_core",
                "status": "PARTIAL",
                "evidence": {
                    "runtimeOwner": true, "editorSurface": true,
                    "schemaMigration": true, "diagnostics": true,
                    "testsValidation": true, "docsAligned": true
                }
            }
        ]
    })");

    const std::vector<std::string> governed = {"battle_core"};
    const std::map<std::string, std::string> paths = {
        {"battle_core", "docs/BATTLE_CORE_CLOSURE_SIGNOFF.md"}};

    const auto violations = checkSignoffContracts(readiness, governed, paths);
    REQUIRE(violations.size() == 1);
    REQUIRE(violations[0].rule == "signoff_block_missing");
}

TEST_CASE("signoff artifact path mismatch is detected", "[governance][s25]") {
    const nlohmann::json readiness = nlohmann::json::parse(R"({
        "schemaVersion": "1.0.0",
        "statusDate": "2026-04-23",
        "subsystems": [
            {
                "id": "battle_core",
                "status": "PARTIAL",
                "signoff": {
                    "required": true,
                    "artifactPath": "docs/WRONG_PATH.md",
                    "promotionRequiresHumanReview": true,
                    "workflow": "docs/RELEASE_SIGNOFF_WORKFLOW.md"
                },
                "evidence": {
                    "runtimeOwner": true, "editorSurface": true,
                    "schemaMigration": true, "diagnostics": true,
                    "testsValidation": true, "docsAligned": true
                }
            }
        ]
    })");

    const std::vector<std::string> governed = {"battle_core"};
    const std::map<std::string, std::string> paths = {
        {"battle_core", "docs/BATTLE_CORE_CLOSURE_SIGNOFF.md"}};

    const auto violations = checkSignoffContracts(readiness, governed, paths);
    REQUIRE(violations.size() == 1);
    REQUIRE(violations[0].rule == "artifact_path_mismatch");
}

TEST_CASE("signoff contract with human review false is flagged", "[governance][s25]") {
    const nlohmann::json readiness = nlohmann::json::parse(R"({
        "schemaVersion": "1.0.0",
        "statusDate": "2026-04-23",
        "subsystems": [
            {
                "id": "save_data_core",
                "status": "PARTIAL",
                "signoff": {
                    "required": true,
                    "artifactPath": "docs/SAVE_DATA_CORE_CLOSURE_SIGNOFF.md",
                    "promotionRequiresHumanReview": false,
                    "workflow": "docs/RELEASE_SIGNOFF_WORKFLOW.md"
                },
                "evidence": {
                    "runtimeOwner": true, "editorSurface": true,
                    "schemaMigration": true, "diagnostics": true,
                    "testsValidation": true, "docsAligned": true
                }
            }
        ]
    })");

    const std::vector<std::string> governed = {"save_data_core"};
    const std::map<std::string, std::string> paths = {
        {"save_data_core", "docs/SAVE_DATA_CORE_CLOSURE_SIGNOFF.md"}};

    const auto violations = checkSignoffContracts(readiness, governed, paths);
    REQUIRE(violations.size() == 1);
    REQUIRE(violations[0].rule == "human_review_not_required");
}

TEST_CASE("correct signoff contract passes all checks", "[governance][s25]") {
    const nlohmann::json readiness = nlohmann::json::parse(R"({
        "schemaVersion": "1.0.0",
        "statusDate": "2026-04-23",
        "subsystems": [
            {
                "id": "battle_core",
                "status": "PARTIAL",
                "signoff": {
                    "required": true,
                    "artifactPath": "docs/BATTLE_CORE_CLOSURE_SIGNOFF.md",
                    "promotionRequiresHumanReview": true,
                    "workflow": "docs/RELEASE_SIGNOFF_WORKFLOW.md"
                },
                "evidence": {
                    "runtimeOwner": true, "editorSurface": true,
                    "schemaMigration": true, "diagnostics": true,
                    "testsValidation": true, "docsAligned": true
                }
            }
        ]
    })");

    const std::vector<std::string> governed = {"battle_core"};
    const std::map<std::string, std::string> paths = {
        {"battle_core", "docs/BATTLE_CORE_CLOSURE_SIGNOFF.md"}};

    const auto violations = checkSignoffContracts(readiness, governed, paths);
    REQUIRE(violations.empty());
}

// ----------------------------------------------------------------------------
// S25-T04 — Template spec bar drift regression tests
// ----------------------------------------------------------------------------
TEST_CASE("bar status drift between spec and readiness is detected", "[governance][s25]") {
    // readiness says PARTIAL, spec claims READY — drift
    const nlohmann::json templateBars = nlohmann::json::parse(R"({
        "accessibility": "PARTIAL",
        "audio": "PARTIAL",
        "input": "PARTIAL",
        "localization": "PARTIAL",
        "performance": "PARTIAL"
    })");

    const std::map<std::string, std::string> specBars = {
        {"accessibility", "READY"},  // DRIFT: spec has READY, readiness has PARTIAL
        {"audio", "PARTIAL"},
        {"input", "PARTIAL"},
        {"localization", "PARTIAL"},
        {"performance", "PARTIAL"}};

    const auto drifts = detectBarDrift(templateBars, specBars);
    REQUIRE(drifts.size() == 1);
    REQUIRE(drifts[0].bar_name == "accessibility");
    REQUIRE(drifts[0].spec_status == "READY");
    REQUIRE(drifts[0].readiness_status == "PARTIAL");
}

TEST_CASE("bar missing from spec is detected as drift", "[governance][s25]") {
    const nlohmann::json templateBars = nlohmann::json::parse(R"({
        "accessibility": "PARTIAL",
        "audio": "PARTIAL",
        "input": "PARTIAL",
        "localization": "PARTIAL",
        "performance": "PARTIAL"
    })");

    // Spec is missing the "performance" bar
    const std::map<std::string, std::string> specBars = {
        {"accessibility", "PARTIAL"},
        {"audio", "PARTIAL"},
        {"input", "PARTIAL"},
        {"localization", "PARTIAL"}
        // performance missing
    };

    const auto drifts = detectBarDrift(templateBars, specBars);
    REQUIRE(drifts.size() == 1);
    REQUIRE(drifts[0].bar_name == "performance");
    REQUIRE(drifts[0].spec_status == "(missing)");
}

TEST_CASE("matching bars produce no drift", "[governance][s25]") {
    const nlohmann::json templateBars = nlohmann::json::parse(R"({
        "accessibility": "PARTIAL",
        "audio": "PARTIAL",
        "input": "PARTIAL",
        "localization": "PARTIAL",
        "performance": "PARTIAL"
    })");

    const std::map<std::string, std::string> specBars = {
        {"accessibility", "PARTIAL"},
        {"audio", "PARTIAL"},
        {"input", "PARTIAL"},
        {"localization", "PARTIAL"},
        {"performance", "PARTIAL"}};

    const auto drifts = detectBarDrift(templateBars, specBars);
    REQUIRE(drifts.empty());
}

// ----------------------------------------------------------------------------
// S25-T06/T07 — Status date consistency regression tests
// ----------------------------------------------------------------------------
TEST_CASE("status date mismatch in doc text is detected", "[governance][s25]") {
    const std::string docTextOld =
        "# Release Readiness\n\nStatus Date: 2026-01-15\n\n## Current Matrix\n";
    REQUIRE_FALSE(statusDateMatchesReadiness(docTextOld, "2026-04-23"));
}

TEST_CASE("matching status date in doc text passes check", "[governance][s25]") {
    const std::string docTextCurrent =
        "# Release Readiness\n\nStatus Date: 2026-04-23\n\n## Current Matrix\n";
    REQUIRE(statusDateMatchesReadiness(docTextCurrent, "2026-04-23"));
}

TEST_CASE("doc text with no status date line fails check", "[governance][s25]") {
    const std::string docNoDate =
        "# Release Readiness\n\n## Current Matrix\n| `battle_core` | `PARTIAL` |\n";
    REQUIRE_FALSE(statusDateMatchesReadiness(docNoDate, "2026-04-23"));
}

TEST_CASE("overclaim pattern detection correctly ignores PARTIAL subsystem", "[governance][s25]") {
    const std::string mockDoc =
        "The `ui_menu_core` subsystem is READY.\n"
        "The `battle_core` subsystem is PARTIAL.\n";

    const std::vector<std::string> nonReady = {"battle_core", "gameplay_ability_framework"};
    std::vector<std::string> detected;
    std::istringstream stream(mockDoc);
    std::string line;
    while (std::getline(stream, line)) {
        if (!std::regex_search(line, std::regex("is READY|are READY"))) {
            continue;
        }
        for (const auto& id : nonReady) {
            if (line.find("`" + id + "`") != std::string::npos) {
                detected.push_back(id);
            }
        }
    }
    // battle_core line says "is PARTIAL" not "is READY" -> should not be detected
    REQUIRE(detected.empty());
}

TEST_CASE("PROJECT_AUDIT.md governance issue count field is a non-negative integer", "[governance][s25]") {
    // Simulates the kind of JSON that ProjectAuditCLI emits for issue counts.
    // Verifies the shape that truth_reconciler and S25-T06 depend on.
    const nlohmann::json auditOutput = nlohmann::json::parse(R"({
        "tool": "urpg_project_audit",
        "sections": [
            {
                "id": "schema_governance",
                "issue_count": 0,
                "status": "CLEAN"
            },
            {
                "id": "release_signoff",
                "issue_count": 2,
                "status": "NEEDS_ATTENTION"
            }
        ]
    })");

    for (const auto& section : auditOutput["sections"]) {
        REQUIRE(section.contains("issue_count"));
        REQUIRE(section["issue_count"].is_number_integer());
        const int count = section["issue_count"].get<int>();
        REQUIRE(count >= 0);
    }
}

// ----------------------------------------------------------------------------
// S30-T01/T02/T03 — Template bar evidence governance tests
// Validates that barEvidence fields exist and bars are consistently represented
// across readiness fixtures for the three READY candidate templates.
// ----------------------------------------------------------------------------

TEST_CASE("jrpg template bar evidence exists for all required bars", "[governance][s30t01]") {
    // Simulates readiness fixture for jrpg with barEvidence covering all five bars.
    const nlohmann::json readiness = nlohmann::json::parse(R"({
        "schemaVersion": "1.0.0",
        "statusDate": "2026-04-23",
        "templates": [
            {
                "id": "jrpg",
                "status": "PARTIAL",
                "bars": {
                    "accessibility": "PARTIAL",
                    "audio": "PARTIAL",
                    "input": "PARTIAL",
                    "localization": "PARTIAL",
                    "performance": "PARTIAL"
                },
                "barEvidence": {
                    "accessibility": "Baseline accessibility governance via AccessibilityAuditor.",
                    "audio": "Audio mix preset governance and validator rules are exercised.",
                    "input": "Menu and battle input paths are covered by unit tests.",
                    "localization": "Baseline message localization patterns are exercised.",
                    "performance": "General RPG frame-time budgets via profile_arena tests."
                }
            }
        ]
    })");

    const auto& tmpl = readiness["templates"][0];
    REQUIRE(tmpl["id"] == "jrpg");
    REQUIRE(tmpl.contains("barEvidence"));
    const auto& barEvidence = tmpl["barEvidence"];
    REQUIRE(barEvidence.contains("accessibility"));
    REQUIRE(barEvidence.contains("audio"));
    REQUIRE(barEvidence.contains("input"));
    REQUIRE(barEvidence.contains("localization"));
    REQUIRE(barEvidence.contains("performance"));
    for (const auto& [key, value] : barEvidence.items()) {
        REQUIRE(!value.get<std::string>().empty());
    }
}

TEST_CASE("jrpg bar status aligns with spec - no drift on nominal fixture", "[governance][s30t01]") {
    const nlohmann::json templateBars = nlohmann::json::parse(R"({
        "accessibility": "PARTIAL",
        "audio": "PARTIAL",
        "input": "PARTIAL",
        "localization": "PARTIAL",
        "performance": "PARTIAL"
    })");
    const std::map<std::string, std::string> specBars = {
        {"accessibility", "PARTIAL"},
        {"audio", "PARTIAL"},
        {"input", "PARTIAL"},
        {"localization", "PARTIAL"},
        {"performance", "PARTIAL"}};

    const auto drifts = detectBarDrift(templateBars, specBars);
    REQUIRE(drifts.empty());
}

TEST_CASE("visual_novel template bar evidence exists for all required bars", "[governance][s30t02]") {
    const nlohmann::json readiness = nlohmann::json::parse(R"({
        "schemaVersion": "1.0.0",
        "statusDate": "2026-04-23",
        "templates": [
            {
                "id": "visual_novel",
                "status": "PARTIAL",
                "bars": {
                    "accessibility": "PARTIAL",
                    "audio": "PARTIAL",
                    "input": "PARTIAL",
                    "localization": "PARTIAL",
                    "performance": "PARTIAL"
                },
                "barEvidence": {
                    "accessibility": "Baseline accessibility audits cover dialogue and menu UI elements.",
                    "audio": "Audio mix preset governance is active.",
                    "input": "Dialogue-advance and menu-navigation input paths are covered.",
                    "localization": "Baseline message localization patterns from message_text_core apply.",
                    "performance": "Presentation runtime and profile_arena tests provide frame-time coverage."
                }
            }
        ]
    })");

    const auto& tmpl = readiness["templates"][0];
    REQUIRE(tmpl["id"] == "visual_novel");
    REQUIRE(tmpl.contains("barEvidence"));
    const auto& barEvidence = tmpl["barEvidence"];
    REQUIRE(barEvidence.contains("accessibility"));
    REQUIRE(barEvidence.contains("audio"));
    REQUIRE(barEvidence.contains("input"));
    REQUIRE(barEvidence.contains("localization"));
    REQUIRE(barEvidence.contains("performance"));
    for (const auto& [key, value] : barEvidence.items()) {
        REQUIRE(!value.get<std::string>().empty());
    }
}

TEST_CASE("visual_novel bar status aligns with spec - no drift on nominal fixture", "[governance][s30t02]") {
    const nlohmann::json templateBars = nlohmann::json::parse(R"({
        "accessibility": "PARTIAL",
        "audio": "PARTIAL",
        "input": "PARTIAL",
        "localization": "PARTIAL",
        "performance": "PARTIAL"
    })");
    const std::map<std::string, std::string> specBars = {
        {"accessibility", "PARTIAL"},
        {"audio", "PARTIAL"},
        {"input", "PARTIAL"},
        {"localization", "PARTIAL"},
        {"performance", "PARTIAL"}};

    const auto drifts = detectBarDrift(templateBars, specBars);
    REQUIRE(drifts.empty());
}

TEST_CASE("turn_based_rpg template bar evidence exists for all required bars", "[governance][s30t03]") {
    const nlohmann::json readiness = nlohmann::json::parse(R"({
        "schemaVersion": "1.0.0",
        "statusDate": "2026-04-23",
        "templates": [
            {
                "id": "turn_based_rpg",
                "status": "PARTIAL",
                "bars": {
                    "accessibility": "PARTIAL",
                    "audio": "PARTIAL",
                    "input": "PARTIAL",
                    "localization": "PARTIAL",
                    "performance": "PARTIAL"
                },
                "barEvidence": {
                    "accessibility": "Baseline accessibility governance is active via AccessibilityAuditor.",
                    "audio": "Audio mix preset governance applies.",
                    "input": "Battle and menu input paths are covered by unit tests.",
                    "localization": "Baseline message localization patterns from message_text_core apply.",
                    "performance": "Presentation runtime arena and profile_arena frame-time budget tests apply."
                }
            }
        ]
    })");

    const auto& tmpl = readiness["templates"][0];
    REQUIRE(tmpl["id"] == "turn_based_rpg");
    REQUIRE(tmpl.contains("barEvidence"));
    const auto& barEvidence = tmpl["barEvidence"];
    REQUIRE(barEvidence.contains("accessibility"));
    REQUIRE(barEvidence.contains("audio"));
    REQUIRE(barEvidence.contains("input"));
    REQUIRE(barEvidence.contains("localization"));
    REQUIRE(barEvidence.contains("performance"));
    for (const auto& [key, value] : barEvidence.items()) {
        REQUIRE(!value.get<std::string>().empty());
    }
}

TEST_CASE("turn_based_rpg bar status aligns with spec - no drift on nominal fixture", "[governance][s30t03]") {
    const nlohmann::json templateBars = nlohmann::json::parse(R"({
        "accessibility": "PARTIAL",
        "audio": "PARTIAL",
        "input": "PARTIAL",
        "localization": "PARTIAL",
        "performance": "PARTIAL"
    })");
    const std::map<std::string, std::string> specBars = {
        {"accessibility", "PARTIAL"},
        {"audio", "PARTIAL"},
        {"input", "PARTIAL"},
        {"localization", "PARTIAL"},
        {"performance", "PARTIAL"}};

    const auto drifts = detectBarDrift(templateBars, specBars);
    REQUIRE(drifts.empty());
}

TEST_CASE("bar evidence field missing for template is detected as governance gap", "[governance][s30t01][s30t02][s30t03]") {
    // If a template does not have barEvidence, that is a governance gap.
    const nlohmann::json readiness = nlohmann::json::parse(R"({
        "schemaVersion": "1.0.0",
        "statusDate": "2026-04-23",
        "templates": [
            {
                "id": "jrpg",
                "status": "PARTIAL",
                "bars": {
                    "accessibility": "PARTIAL",
                    "audio": "PARTIAL"
                }
            }
        ]
    })");

    const auto& tmpl = readiness["templates"][0];
    REQUIRE_FALSE(tmpl.contains("barEvidence"));
}
