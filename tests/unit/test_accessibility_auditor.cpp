#include <catch2/catch_test_macros.hpp>
#include "engine/core/accessibility/accessibility_auditor.h"
#include <nlohmann/json.hpp>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

using namespace urpg::accessibility;
using nlohmann::json;

TEST_CASE("AccessibilityAuditor: Empty ingest returns no issues", "[accessibility]") {
    AccessibilityAuditor auditor;
    auditor.ingestElements({});
    auto issues = auditor.audit();

    REQUIRE(issues.empty());
    REQUIRE(auditor.getIssueCount() == 0);
}

TEST_CASE("AccessibilityAuditor: Focusable element without label produces MissingLabel error", "[accessibility]") {
    AccessibilityAuditor auditor;
    auditor.ingestElements({
        UiElementSnapshot{"btn_ok", "", true, 1, 4.5f}
    });
    auto issues = auditor.audit();

    REQUIRE(issues.size() == 1);
    REQUIRE(issues[0].severity == IssueSeverity::Error);
    REQUIRE(issues[0].category == IssueCategory::MissingLabel);
    REQUIRE(issues[0].elementId == "btn_ok");
}

TEST_CASE("AccessibilityAuditor: Duplicate focus order produces FocusOrder warning", "[accessibility]") {
    AccessibilityAuditor auditor;
    auditor.ingestElements({
        UiElementSnapshot{"btn_a", "Button A", true, 1, 4.5f},
        UiElementSnapshot{"btn_b", "Button B", true, 1, 4.5f}
    });
    auto issues = auditor.audit();

    REQUIRE(issues.size() == 2);
    for (const auto& issue : issues) {
        REQUIRE(issue.severity == IssueSeverity::Warning);
        REQUIRE(issue.category == IssueCategory::FocusOrder);
    }
}

TEST_CASE("AccessibilityAuditor: Low contrast ratio produces Contrast error", "[accessibility]") {
    AccessibilityAuditor auditor;
    auditor.ingestElements({
        UiElementSnapshot{"text_1", "Low Contrast Text", false, 0, 2.5f},
        UiElementSnapshot{"btn_ok", "OK", true, 1, 4.5f}
    });
    auto issues = auditor.audit();

    REQUIRE(issues.size() == 1);
    REQUIRE(issues[0].severity == IssueSeverity::Error);
    REQUIRE(issues[0].category == IssueCategory::Contrast);
    REQUIRE(issues[0].elementId == "text_1");
}

TEST_CASE("AccessibilityAuditor: No focusable elements produces Navigation warning", "[accessibility]") {
    AccessibilityAuditor auditor;
    auditor.ingestElements({
        UiElementSnapshot{"label_1", "Read Only Label", false, 0, 4.5f}
    });
    auto issues = auditor.audit();

    REQUIRE(issues.size() == 1);
    REQUIRE(issues[0].severity == IssueSeverity::Warning);
    REQUIRE(issues[0].category == IssueCategory::Navigation);
    REQUIRE(issues[0].message == "No focusable elements detected");
}

TEST_CASE("AccessibilityAuditor: Clear resets issues", "[accessibility]") {
    AccessibilityAuditor auditor;
    auditor.ingestElements({
        UiElementSnapshot{"btn_ok", "", true, 1, 4.5f}
    });
    auditor.audit();
    REQUIRE(auditor.getIssueCount() == 1);

    auditor.clear();
    REQUIRE(auditor.getIssueCount() == 0);
}

TEST_CASE("AccessibilityAuditor: CI governance script validates artifacts", "[accessibility][project_audit_cli]") {
    const auto repoRoot = std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
    const auto scriptPath = repoRoot / "tools" / "ci" / "check_accessibility_governance.ps1";
    const auto outputPath = std::filesystem::temp_directory_path() / "urpg_accessibility_gov_out.json";

    const std::string command =
        "powershell -ExecutionPolicy Bypass -File \"" + scriptPath.string() +
        "\" > \"" + outputPath.string() + "\"";

    REQUIRE(std::system(command.c_str()) == 0);

    std::ifstream resultFile(outputPath);
    REQUIRE(resultFile.is_open());

    std::string jsonStr((std::istreambuf_iterator<char>(resultFile)),
                         std::istreambuf_iterator<char>());
    resultFile.close();

    auto result = json::parse(jsonStr);
    REQUIRE(result["passed"].get<bool>() == true);
    REQUIRE(result["errors"].is_array());
    REQUIRE(result["errors"].empty());
}
