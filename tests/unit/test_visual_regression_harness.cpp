#include "engine/core/testing/visual_regression_harness.h"

#include <catch2/catch_test_macros.hpp>

#include <cstdlib>
#include <filesystem>
#include <fstream>

using namespace urpg::testing;

namespace {

std::filesystem::path getGoldenRoot() {
    return std::filesystem::path(URPG_SOURCE_DIR) / "tests" / "snapshot" / "goldens";
}

void cleanupGolden(const std::string& testName, const std::string& snapshotId) {
    std::filesystem::path path = getGoldenRoot() / (testName + "_" + snapshotId + ".golden.json");
    if (std::filesystem::exists(path)) {
        std::filesystem::remove(path);
    }
}

SceneSnapshot makeTestSnapshot(int width, int height) {
    SceneSnapshot snapshot;
    snapshot.width = width;
    snapshot.height = height;
    snapshot.pixels.assign(static_cast<size_t>(width) * height, {128, 128, 128, 255});
    return snapshot;
}

} // namespace

TEST_CASE("VisualRegressionHarness saveGolden and loadGolden round-trip", "[testing][visual_regression]") {
    VisualRegressionHarness harness;
    harness.setGoldenRoot(getGoldenRoot().string());

    const std::string testName = "RoundTripTest";
    const std::string snapshotId = "snapshot_01";

    cleanupGolden(testName, snapshotId);

    SceneSnapshot original = makeTestSnapshot(2, 2);
    original.pixels[0] = {255, 0, 0, 255};
    original.pixels[1] = {0, 255, 0, 255};
    original.pixels[2] = {0, 0, 255, 255};
    original.pixels[3] = {255, 255, 255, 255};

    REQUIRE(harness.saveGolden(testName, snapshotId, original));

    auto loaded = harness.loadGolden(testName, snapshotId);
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->width == original.width);
    REQUIRE(loaded->height == original.height);
    REQUIRE(loaded->pixels.size() == original.pixels.size());
    REQUIRE(loaded->pixels == original.pixels);
    REQUIRE(loaded->testName == testName);
    REQUIRE(loaded->snapshotId == snapshotId);

    cleanupGolden(testName, snapshotId);
}

TEST_CASE("VisualRegressionHarness compareAgainstGolden returns match when identical", "[testing][visual_regression]") {
    VisualRegressionHarness harness;
    harness.setGoldenRoot(getGoldenRoot().string());

    const std::string testName = "IdenticalTest";
    const std::string snapshotId = "snapshot_01";

    cleanupGolden(testName, snapshotId);

    SceneSnapshot golden = makeTestSnapshot(3, 3);
    REQUIRE(harness.saveGolden(testName, snapshotId, golden));

    SceneSnapshot current = makeTestSnapshot(3, 3);
    auto result = harness.compareAgainstGolden(testName, snapshotId, current);
    REQUIRE(result.matches);
    REQUIRE(result.errorPercentage == 0.0f);

    cleanupGolden(testName, snapshotId);
}

TEST_CASE("VisualRegressionHarness compareAgainstGolden returns mismatch when pixels differ", "[testing][visual_regression]") {
    VisualRegressionHarness harness;
    harness.setGoldenRoot(getGoldenRoot().string());

    const std::string testName = "MismatchTest";
    const std::string snapshotId = "snapshot_01";

    cleanupGolden(testName, snapshotId);

    SceneSnapshot golden = makeTestSnapshot(2, 2);
    REQUIRE(harness.saveGolden(testName, snapshotId, golden));

    SceneSnapshot current = makeTestSnapshot(2, 2);
    current.pixels[0] = {0, 0, 0, 255};

    auto result = harness.compareAgainstGolden(testName, snapshotId, current, 0.0f);
    REQUIRE_FALSE(result.matches);
    REQUIRE(result.errorPercentage == 25.0f);

    cleanupGolden(testName, snapshotId);
}

TEST_CASE("VisualRegressionHarness compareAgainstGolden returns match when threshold covers difference", "[testing][visual_regression]") {
    VisualRegressionHarness harness;
    harness.setGoldenRoot(getGoldenRoot().string());

    const std::string testName = "ThresholdTest";
    const std::string snapshotId = "snapshot_01";

    cleanupGolden(testName, snapshotId);

    SceneSnapshot golden = makeTestSnapshot(10, 10);
    REQUIRE(harness.saveGolden(testName, snapshotId, golden));

    SceneSnapshot current = makeTestSnapshot(10, 10);
    current.pixels[0] = {0, 0, 0, 255};

    auto match = harness.compareAgainstGolden(testName, snapshotId, current, 1.0f);
    REQUIRE(match.matches);

    auto fail = harness.compareAgainstGolden(testName, snapshotId, current, 0.5f);
    REQUIRE_FALSE(fail.matches);

    cleanupGolden(testName, snapshotId);
}

TEST_CASE("VisualRegressionHarness generateDiffHeatmap produces correct colors", "[testing][visual_regression]") {
    SceneSnapshot golden = makeTestSnapshot(2, 2);
    golden.pixels[0] = {255, 0, 0, 255};
    golden.pixels[1] = {0, 255, 0, 255};
    golden.pixels[2] = {0, 0, 255, 255};
    golden.pixels[3] = {255, 255, 255, 255};

    SceneSnapshot current = golden;
    current.pixels[1] = {0, 254, 0, 255};

    SceneSnapshot heatmap = VisualRegressionHarness::generateDiffHeatmap(golden, current);
    REQUIRE(heatmap.width == golden.width);
    REQUIRE(heatmap.height == golden.height);
    REQUIRE(heatmap.pixels.size() == golden.pixels.size());

    REQUIRE(heatmap.pixels[0] == SnapshotPixel{0, 0, 0, 255});
    REQUIRE(heatmap.pixels[1] == SnapshotPixel{255, 0, 0, 255});
    REQUIRE(heatmap.pixels[2] == SnapshotPixel{0, 0, 0, 255});
    REQUIRE(heatmap.pixels[3] == SnapshotPixel{0, 0, 0, 255});
}

TEST_CASE("VisualRegressionHarness buildReportJson contains expected fields", "[testing][visual_regression]") {
    VisualRegressionHarness harness;
    harness.setGoldenRoot(getGoldenRoot().string());

    SnapshotComparisonResult result;
    result.matches = false;
    result.errorPercentage = 12.5f;

    nlohmann::json report = harness.buildReportJson("MyTest", result);
    REQUIRE(report["testName"] == "MyTest");
    REQUIRE(report["matches"] == false);
    REQUIRE(report["errorPercentage"] == 12.5f);
}

TEST_CASE("VisualRegressionHarness exposes stable backend names", "[testing][visual_regression]") {
    REQUIRE(VisualRegressionHarness::captureBackendToString(CaptureBackend::OpenGL) == "OpenGL");
    REQUIRE(VisualRegressionHarness::captureBackendToString(CaptureBackend::Headless) == "Headless");
}

TEST_CASE("VisualRegressionHarness generic capture API rejects unsupported headless pixel capture",
          "[testing][visual_regression]") {
    VisualRegressionHarness harness;
    std::string errorMessage;

    const auto frame = harness.captureFrame(CaptureBackend::Headless, {}, 4, 4, &errorMessage);
    REQUIRE_FALSE(frame.has_value());
    REQUIRE(errorMessage.find("Headless backend") != std::string::npos);
}

TEST_CASE("VisualRegressionHarness approval script writes a golden consumable by the harness",
          "[testing][visual_regression]") {
    const std::filesystem::path tempRoot = std::filesystem::temp_directory_path() / "urpg_visual_regression_approval";
    std::filesystem::remove_all(tempRoot);
    std::filesystem::create_directories(tempRoot);

    const std::filesystem::path sourcePath = tempRoot / "candidate.golden.json";
    const std::filesystem::path goldenRoot = tempRoot / "goldens";
    const std::filesystem::path scriptPath =
        std::filesystem::path(URPG_SOURCE_DIR) / "tools" / "visual_regression" / "approve_golden.ps1";

    std::ofstream source(sourcePath);
    REQUIRE(source.good());
    source << R"({
  "width": 2,
  "height": 2,
  "pixels": [
    { "r": 255, "g": 0, "b": 0, "a": 255 },
    { "r": 0, "g": 255, "b": 0, "a": 255 },
    { "r": 0, "g": 0, "b": 255, "a": 255 },
    { "r": 255, "g": 255, "b": 255, "a": 255 }
  ]
})";
    source.close();

    const std::string command =
        "powershell -ExecutionPolicy Bypass -File \"" + scriptPath.string() +
        "\" -TestName \"ApprovalScriptTest\" -SnapshotId \"snapshot_01\" -SourcePath \"" + sourcePath.string() +
        "\" -GoldenRoot \"" + goldenRoot.string() + "\"";
    REQUIRE(std::system(command.c_str()) == 0);

    VisualRegressionHarness harness;
    harness.setGoldenRoot(goldenRoot.string());

    auto loaded = harness.loadGolden("ApprovalScriptTest", "snapshot_01");
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->width == 2);
    REQUIRE(loaded->height == 2);
    REQUIRE(loaded->pixels.size() == 4);

    SceneSnapshot current;
    current.width = 2;
    current.height = 2;
    current.pixels = loaded->pixels;

    const auto result = harness.compareAgainstGolden("ApprovalScriptTest", "snapshot_01", current);
    REQUIRE(result.matches);
    REQUIRE(result.errorPercentage == 0.0f);

    std::filesystem::remove_all(tempRoot);
}
