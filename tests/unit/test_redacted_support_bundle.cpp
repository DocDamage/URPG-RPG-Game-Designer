#include "engine/core/diagnostics/redacted_support_bundle.h"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <fstream>

namespace {

std::filesystem::path supportTempRoot() {
    return std::filesystem::temp_directory_path() /
        ("urpg_support_bundle_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
}

} // namespace

TEST_CASE("Support bundle preview includes required evidence and excludes project data by default",
          "[diagnostics][support_bundle][pcq507]") {
    using namespace urpg::diagnostics;
    RedactedSupportBundleInput input;
    input.logs = {"runtime started", "event failed"};
    input.diagnostics = {{{"code", "event_failed"}, {"severity", "error"}}};
    input.versions = {{"editor", "2.0.0"}, {"runtime", "2.0.0"}};
    input.platform_capabilities = {{"renderer", "opengl"}, {"audio", true}};
    input.project_manifest_hashes = {{"project.json", "abc123"}, {"content/maps/town.json", "def456"}};
    input.replay = {{"id", "failure"}, {"seed", 42}};
    input.selected_project_data = {{"dialogue", "private draft"}};

    const auto preview = RedactedSupportBundleBuilder{}.preview(input);
    REQUIRE(preview.valid);
    REQUIRE(preview.code == "support_bundle_preview_ready");
    REQUIRE(preview.bundle["upload_performed"] == false);
    REQUIRE(preview.bundle.contains("logs"));
    REQUIRE(preview.bundle.contains("diagnostics"));
    REQUIRE(preview.bundle.contains("versions"));
    REQUIRE(preview.bundle.contains("platform_capabilities"));
    REQUIRE(preview.bundle.contains("project_manifest_hashes"));
    REQUIRE(preview.bundle.contains("replay"));
    REQUIRE_FALSE(preview.bundle.contains("selected_project_data"));
    REQUIRE(preview.excluded_sections == std::vector<std::string>{"selected_project_data"});
}

TEST_CASE("Support bundle secret path and PII corpus is conservatively redacted",
          "[diagnostics][support_bundle][pcq507]") {
    using namespace urpg::diagnostics;
    RedactedSupportBundleInput input;
    input.logs = {"Authorization: Bearer super-secret", "opened C:\\Users\\Ada\\project.json",
                  "contact ada@example.test"};
    input.diagnostics = {{"token", "token-value"}, {"nested", {{"password", "hunter2"}}},
                         {"source_path", "/home/ada/project/dialogue.json"}, {"username", "ada"}};
    input.versions = {{"runtime", "2.0"}};
    input.platform_capabilities = {{"home", "C:\\Users\\Ada"}};
    input.project_manifest_hashes = {{"project", "safe-hash"}};
    input.selected_project_data = {{"email", "ada@example.test"}, {"safe", "retained"}};
    input.include_selected_project_data = true;

    const auto preview = RedactedSupportBundleBuilder{}.preview(input);
    const auto serialized = preview.bundle.dump();
    for (const auto& forbidden : {"super-secret", "C:\\Users\\Ada", "ada@example.test", "token-value",
                                  "hunter2", "/home/ada", "\"ada\""}) {
        REQUIRE(serialized.find(forbidden) == std::string::npos);
    }
    REQUIRE(serialized.find("safe-hash") != std::string::npos);
    REQUIRE(serialized.find("retained") != std::string::npos);
    REQUIRE(preview.redactions.size() == 9);
    REQUIRE(preview.bundle["redaction_count"] == 9);
}

TEST_CASE("Support bundle requires explicit preview approval and writes only locally",
          "[diagnostics][support_bundle][pcq507]") {
    using namespace urpg::diagnostics;
    const auto root = supportTempRoot();
    RedactedSupportBundleInput input;
    input.versions = {{"runtime", "2.0"}};
    const auto preview = RedactedSupportBundleBuilder{}.preview(input);
    const auto rejected = RedactedSupportBundleBuilder{}.writeApproved(preview, root, false);
    REQUIRE_FALSE(rejected.success);
    REQUIRE(rejected.code == "support_bundle_preview_not_approved");
    REQUIRE_FALSE(std::filesystem::exists(root / "redacted_support_bundle.json"));

    const auto written = RedactedSupportBundleBuilder{}.writeApproved(preview, root, true);
    REQUIRE(written.success);
    REQUIRE(written.code == "support_bundle_written");
    REQUIRE(std::filesystem::is_regular_file(written.path));
    std::ifstream stream(written.path);
    const auto loaded = nlohmann::json::parse(stream);
    stream.close();
    REQUIRE(loaded["upload_performed"] == false);
    REQUIRE(loaded["preview_required"] == true);
    std::filesystem::remove_all(root);
}
