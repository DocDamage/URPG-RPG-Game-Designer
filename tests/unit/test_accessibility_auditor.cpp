#include <catch2/catch_test_macros.hpp>
#include "engine/core/accessibility/accessibility_auditor.h"
#include "engine/core/accessibility/render_contrast_adapter.h"
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

TEST_CASE("Render contrast adapter derives WYSIWYG contrast from frame commands",
          "[accessibility][render][wysiwyg]") {
    urpg::RectCommand background;
    background.x = 10.0f;
    background.y = 20.0f;
    background.w = 240.0f;
    background.h = 64.0f;
    background.r = 0.02f;
    background.g = 0.02f;
    background.b = 0.02f;
    background.zOrder = 1;

    urpg::TextCommand label;
    label.x = 24.0f;
    label.y = 34.0f;
    label.text = "Readable command";
    label.fontSize = 20;
    label.maxWidth = 180;
    label.r = 255;
    label.g = 255;
    label.b = 255;
    label.zOrder = 2;

    const std::vector<urpg::FrameRenderCommand> commands = {
        urpg::toFrameRenderCommand(background),
        urpg::toFrameRenderCommand(label),
    };

    RenderContrastAdapterOptions options;
    options.text_is_focusable = true;
    const auto elements = ingestRendererContrastElements(commands, options);
    REQUIRE(elements.size() == 1);
    REQUIRE(elements[0].id == "render.text.0");
    REQUIRE(elements[0].label == "Readable command");
    REQUIRE(elements[0].contrastRatio > 10.0f);

    AccessibilityAuditor auditor;
    auditor.ingestElements(elements);
    REQUIRE(auditor.audit().empty());
}

TEST_CASE("Render contrast adapter exposes low renderer-derived text contrast to auditor",
          "[accessibility][render][wysiwyg]") {
    urpg::RectCommand background;
    background.x = 0.0f;
    background.y = 0.0f;
    background.w = 200.0f;
    background.h = 40.0f;
    background.r = 0.5f;
    background.g = 0.5f;
    background.b = 0.5f;
    background.zOrder = 1;

    urpg::TextCommand label;
    label.x = 8.0f;
    label.y = 8.0f;
    label.text = "Low contrast";
    label.fontSize = 18;
    label.maxWidth = 160;
    label.r = 128;
    label.g = 128;
    label.b = 128;
    label.zOrder = 2;

    const std::vector<urpg::FrameRenderCommand> commands = {
        urpg::toFrameRenderCommand(background),
        urpg::toFrameRenderCommand(label),
    };

    AccessibilityAuditor auditor;
    RenderContrastAdapterOptions options;
    options.text_is_focusable = true;
    auditor.ingestElements(ingestRendererContrastElements(commands, options));
    const auto issues = auditor.audit();

    REQUIRE(issues.size() == 1);
    REQUIRE(issues[0].category == IssueCategory::Contrast);
    REQUIRE(issues[0].elementId == "render.text.0");
    REQUIRE(issues[0].sourceFile == "engine/core/render/render_layer.h");
}
