#include <catch2/catch_test_macros.hpp>

#include "editor/ai/ai_assistant_panel.h"
#include "editor/collaboration/local_review_panel.h"
#include "editor/diagnostics/diagnostics_bundle_panel.h"
#include "editor/mod/mod_sdk_panel.h"
#include "engine/core/ai/ai_assistant_config.h"
#include "engine/core/ai/ai_suggestion_record.h"
#include "engine/core/collaboration/local_review_bundle.h"
#include "engine/core/diagnostics/diagnostics_bundle_exporter.h"
#include "engine/core/mod/mod_sdk_sample_validator.h"

#include <chrono>
#include <filesystem>
#include <fstream>

namespace {

std::filesystem::path tempRoot(const std::string& name) {
    const auto suffix = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    auto root = std::filesystem::temp_directory_path() / ("urpg_" + name + "_" + suffix);
    std::filesystem::create_directories(root);
    return root;
}

void writeText(const std::filesystem::path& path, const std::string& text) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path);
    out << text;
}

} // namespace

TEST_CASE("DiagnosticsBundleExporter excludes ignored secret files", "[diagnostics_bundle]") {
    const auto root = tempRoot("diagnostics_bundle");
    writeText(root / "logs" / "editor.log", "ok");
    writeText(root / ".env", "TOKEN=secret");
    writeText(root / "reports" / "asset_report.json", "{}");

    urpg::diagnostics::DiagnosticsBundleInput input;
    input.projectRoot = root;
    input.logs = {root / "logs" / "editor.log", root / ".env"};
    input.assetReports = {root / "reports" / "asset_report.json"};
    input.systemInfo = {{"os", "test"}};

    urpg::diagnostics::DiagnosticsBundleExporter exporter;
    const auto result = exporter.exportBundle(input, root / "bundle");

    REQUIRE(result.success);
    REQUIRE(std::filesystem::exists(result.manifestPath));
    REQUIRE(result.manifest["logs"].size() == 1);
    REQUIRE(result.manifest["logs"][0]["path"] == "logs/editor.log");
    REQUIRE(result.manifest["logs"][0]["bundle_path"] == "logs/editor.log");
    REQUIRE(std::filesystem::exists(root / "bundle" / "logs" / "editor.log"));
    REQUIRE_FALSE(std::filesystem::exists(root / "bundle" / "logs" / ".env"));
    REQUIRE(result.manifest["asset_reports"].size() == 1);
    REQUIRE(result.manifest["asset_reports"][0]["bundle_path"] == "reports/asset_report.json");
    REQUIRE(std::filesystem::exists(root / "bundle" / "reports" / "asset_report.json"));
    REQUIRE(result.manifest["excluded_paths"].size() == 1);
    REQUIRE(result.manifest["contains_secret_files"] == false);
}

TEST_CASE("Mod SDK sample passes validation and forbidden permission fails", "[mod_sdk]") {
    urpg::mod::ModSdkSampleValidator validator;
    const auto repoRoot = std::filesystem::path(URPG_SOURCE_DIR);

    const auto valid = validator.validateSample(repoRoot / "content" / "fixtures" / "mod_sdk_sample");
    REQUIRE(valid.passed);
    REQUIRE(valid.issues.empty());

    const auto badRoot = tempRoot("bad_mod_sdk");
    writeText(badRoot / "README.md", "bad");
    writeText(badRoot / "expected_diagnostics.json", "{}");
    writeText(badRoot / "manifest.json",
              R"({"id":"bad","name":"Bad","version":"1.0.0","entryPoint":"main.js","permissions":["network"]})");

    const auto invalid = validator.validateSample(badRoot);
    REQUIRE_FALSE(invalid.passed);
    REQUIRE(invalid.issues.size() == 1);
    REQUIRE(invalid.issues[0].code == "forbidden_permission");

    writeText(badRoot / "manifest.json",
              R"({"id":"bad","name":"Bad","version":"1.0.0","entryPoint":"main.js","permissions":[7]})");
    const auto malformed = validator.validateSample(badRoot);
    REQUIRE_FALSE(malformed.passed);
    REQUIRE(malformed.issues.size() == 1);
    REQUIRE(malformed.issues[0].code == "invalid_permission");
}

TEST_CASE("Local review summary falls back to file manifest without Git", "[collaboration]") {
    const auto root = tempRoot("local_review");
    writeText(root / "docs" / "note.md", "change");
    writeText(root / ".git" / "ignored", "ignore");

    urpg::collaboration::LocalReviewInput input;
    input.workspaceRoot = root;
    input.gitAvailable = false;
    input.comments = {{"docs/note.md", "Looks ready.", 1}};
    input.checklist = {"Tests run"};

    urpg::collaboration::LocalReviewBundleBuilder builder;
    const auto bundle = builder.build(input);

    REQUIRE(bundle.summary["source"] == "file_manifest");
    REQUIRE(bundle.summary["changed_files"].size() == 1);
    REQUIRE(bundle.summary["changed_files"][0] == "docs/note.md");
    REQUIRE(bundle.summary["comments"].size() == 1);
    REQUIRE(bundle.handoff["ready_for_async_review"] == true);
}

TEST_CASE("AI assistant disabled state is explicit and non-error", "[ai_assistant]") {
    urpg::ai::AiAssistantConfig config;
    urpg::ai::AiAssistantConfigValidator validator;

    const auto status = validator.evaluate(config, false);

    REQUIRE_FALSE(status.available);
    REQUIRE_FALSE(status.error);
    REQUIRE(status.reason == "disabled");
}

TEST_CASE("AI suggestions require approval and protect runtime status docs", "[ai_assistant]") {
    urpg::ai::AiSuggestionPolicy policy;

    urpg::ai::AiSuggestionRecord draft;
    draft.id = "s1";
    draft.targetPath = "docs/new_design.md";
    draft.reviewState = urpg::ai::AiSuggestionReviewState::NeedsReview;
    REQUIRE_FALSE(policy.canApply(draft));

    draft.reviewState = urpg::ai::AiSuggestionReviewState::Approved;
    REQUIRE(policy.canApply(draft));

    draft.targetPath = "docs/PROGRAM_COMPLETION_STATUS.md";
    REQUIRE_FALSE(policy.canApply(draft));
    REQUIRE(policy.toJson(draft)["generated_content"] == true);
}

TEST_CASE("FFS-16 editor panels expose deterministic snapshots", "[diagnostics_bundle][mod_sdk][collaboration][ai_assistant]") {
    urpg::editor::AiAssistantPanel aiPanel;
    aiPanel.setConfig({}, false);
    aiPanel.render();
    REQUIRE(aiPanel.lastRenderSnapshot()["status"]["reason"] == "disabled");

    urpg::editor::LocalReviewPanel reviewPanel;
    urpg::collaboration::LocalReviewInput reviewInput;
    reviewInput.workspaceRoot = tempRoot("review_panel");
    reviewInput.gitAvailable = true;
    reviewInput.changedFiles = {reviewInput.workspaceRoot / "a.cpp"};
    reviewPanel.buildBundle(reviewInput);
    reviewPanel.render();
    REQUIRE(reviewPanel.lastRenderSnapshot()["changed_file_count"] == 1);

    urpg::editor::DiagnosticsBundlePanel diagnosticsPanel;
    diagnosticsPanel.render();
    REQUIRE(diagnosticsPanel.lastRenderSnapshot()["ready"] == false);

    urpg::editor::ModSdkPanel modPanel;
    modPanel.validateSample(std::filesystem::path(URPG_SOURCE_DIR) / "content" / "fixtures" / "mod_sdk_sample");
    modPanel.render();
    REQUIRE(modPanel.lastRenderSnapshot()["passed"] == true);
}
