#include "engine/core/testing/visual_regression_harness.h"
#include "engine/core/engine_shell.h"
#include "engine/core/scene/map_scene.h"
#include "engine/core/scene/scene_manager.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <unordered_map>

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

std::vector<urpg::FrameRenderCommand> makeMetadataCaptureCommands() {
    urpg::FrameRenderCommand clear;
    clear.type = urpg::RenderCmdType::Clear;

    urpg::RectCommand rect;
    rect.x = 1.0f;
    rect.y = 1.0f;
    rect.w = 2.0f;
    rect.h = 2.0f;
    rect.r = 0.8f;
    rect.g = 0.1f;
    rect.b = 0.2f;
    rect.a = 1.0f;
    rect.zOrder = 1;

    return {
        clear,
        urpg::toFrameRenderCommand(rect),
    };
}

void clearSceneStack() {
    auto& sceneManager = urpg::scene::SceneManager::getInstance();
    while (sceneManager.stackSize() > 0) {
        sceneManager.popScene();
    }
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

TEST_CASE("VisualRegressionHarness writes compact run-length goldens",
          "[testing][visual_regression]") {
    VisualRegressionHarness harness;
    harness.setGoldenRoot(getGoldenRoot().string());

    const std::string testName = "CompactRunLengthTest";
    const std::string snapshotId = "snapshot_01";

    cleanupGolden(testName, snapshotId);

    SceneSnapshot original = makeTestSnapshot(8, 2);
    original.pixels[8] = {64, 96, 128, 255};

    REQUIRE(harness.saveGolden(testName, snapshotId, original));

    const auto goldenPath = getGoldenRoot() / (testName + "_" + snapshotId + ".golden.json");
    nlohmann::json metadata;
    {
        std::ifstream file(goldenPath);
        REQUIRE(file.good());
        metadata = nlohmann::json::parse(file);
    }

    REQUIRE(metadata.contains("format"));
    REQUIRE(metadata["format"] == "urpg.visual_golden.rle.v1");
    REQUIRE(metadata.contains("pixelRuns"));
    REQUIRE_FALSE(metadata.contains("pixels"));
    REQUIRE(metadata["pixelRuns"].size() == 3);
    REQUIRE(metadata["pixelRuns"][0] == nlohmann::json::array({8, 128, 128, 128, 255}));

    auto loaded = harness.loadGolden(testName, snapshotId);
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->pixels == original.pixels);

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
    REQUIRE(VisualRegressionHarness::captureBackendToString(CaptureBackend::SoftwareReference) == "software_reference");
}

TEST_CASE("VisualRegressionHarness reports local renderer backend parity matrix",
          "[testing][visual_regression][task5]") {
    const auto matrix = VisualRegressionHarness::buildLocalBackendParityMatrix();
    REQUIRE(matrix.size() == 3);

    const auto openGl = std::find_if(matrix.begin(), matrix.end(), [](const auto& entry) {
        return entry.backend == CaptureBackend::OpenGL;
    });
    const auto headless = std::find_if(matrix.begin(), matrix.end(), [](const auto& entry) {
        return entry.backend == CaptureBackend::Headless;
    });
    const auto reference = std::find_if(matrix.begin(), matrix.end(), [](const auto& entry) {
        return entry.backend == CaptureBackend::SoftwareReference;
    });

    REQUIRE(openGl != matrix.end());
    REQUIRE(headless != matrix.end());
    REQUIRE(reference != matrix.end());
    REQUIRE(headless->local_runnable);
    REQUIRE(headless->frame_capture_supported);
    REQUIRE(headless->scene_capture_supported);
    REQUIRE(headless->engine_tick_capture_supported);
    REQUIRE_FALSE(headless->boundary_note.empty());
    REQUIRE(reference->local_runnable);
    REQUIRE(reference->frame_capture_supported);
    REQUIRE(reference->scene_capture_supported);
    REQUIRE(reference->engine_tick_capture_supported);
#ifdef URPG_HEADLESS
    REQUIRE_FALSE(openGl->local_runnable);
    REQUIRE_FALSE(openGl->frame_capture_supported);
#else
    REQUIRE(openGl->local_runnable);
    REQUIRE(openGl->frame_capture_supported);
    REQUIRE(openGl->scene_capture_supported);
    REQUIRE(openGl->engine_tick_capture_supported);
#endif
}

TEST_CASE("VisualRegressionHarness generic capture API returns backend metadata for local capture modes",
          "[testing][visual_regression]") {
    VisualRegressionHarness harness;
    const auto commands = makeMetadataCaptureCommands();

    for (const CaptureBackend backend : {
#ifndef URPG_HEADLESS
             CaptureBackend::OpenGL,
#endif
             CaptureBackend::Headless,
             CaptureBackend::SoftwareReference,
         }) {
        std::string errorMessage;
        const auto capture = harness.captureFrameResult(backend, commands, 8, 6, &errorMessage);
        INFO(errorMessage);
        REQUIRE(capture.has_value());
        REQUIRE(capture->backendId == VisualRegressionHarness::captureBackendToString(backend));
        REQUIRE(capture->commandCount == commands.size());
        REQUIRE(capture->snapshot.width == 8);
        REQUIRE(capture->snapshot.height == 6);
        REQUIRE(capture->snapshot.pixels.size() == 48);
        REQUIRE(capture->stableHash != 0);
    }
}

TEST_CASE("VisualRegressionHarness non-OpenGL scene capture consumes command streams",
          "[testing][visual_regression]") {
    VisualRegressionHarness harness;

    for (const CaptureBackend backend : {CaptureBackend::Headless, CaptureBackend::SoftwareReference}) {
        std::string errorMessage;
        const auto snapshot = harness.captureScene(
            backend,
            [](urpg::RendererBackend& renderer) {
                renderer.beginFrame();
                renderer.processFrameCommands(makeMetadataCaptureCommands());
                renderer.endFrame();
            },
            8,
            6,
            &errorMessage);

        INFO(errorMessage);
        REQUIRE(snapshot.has_value());
        REQUIRE(snapshot->width == 8);
        REQUIRE(snapshot->height == 6);
        REQUIRE(snapshot->pixels.size() == 48);
    }
}

TEST_CASE("VisualRegressionHarness headless EngineShell capture consumes MapScene commands",
          "[testing][visual_regression]") {
    VisualRegressionHarness harness;
    clearSceneStack();
    urpg::RenderLayer::getInstance().flush();

    std::string errorMessage;
    const auto snapshot = harness.captureEngineTick(
        CaptureBackend::Headless,
        [](urpg::EngineShell& /*shell*/) {
            auto map = std::make_shared<urpg::scene::MapScene>("VisualHarnessHeadlessMap", 2, 2);
            map->setTile(0, 0, 1, true);
            map->setTile(1, 0, 2, true);
            urpg::scene::SceneManager::getInstance().pushScene(map);
        },
        96,
        96,
        &errorMessage);

    INFO(errorMessage);
    REQUIRE(snapshot.has_value());
    REQUIRE(snapshot->width == 96);
    REQUIRE(snapshot->height == 96);
    REQUIRE(snapshot->pixels.size() == 9216);

    clearSceneStack();
    urpg::RenderLayer::getInstance().flush();
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

TEST_CASE("VisualRegressionHarness oversized committed goldens are governance-listed",
          "[testing][visual_regression]") {
    const auto sourceRoot = std::filesystem::path(URPG_SOURCE_DIR);
    const auto policyPath = sourceRoot / "content" / "fixtures" / "visual_golden_governance.json";

    std::ifstream policyFile(policyPath);
    REQUIRE(policyFile.good());

    nlohmann::json policy = nlohmann::json::parse(policyFile);
    REQUIRE(policy["version"] == 1);
    REQUIRE(policy["maxUngovernedGoldenBytes"].is_number_unsigned());

    const auto maxUngovernedBytes = policy["maxUngovernedGoldenBytes"].get<std::uintmax_t>();
    REQUIRE(maxUngovernedBytes > 0);
    REQUIRE(policy["oversizedGoldens"].is_array());

    std::unordered_map<std::string, nlohmann::json> governed;
    for (const auto& entry : policy["oversizedGoldens"]) {
        REQUIRE(entry["path"].is_string());
        REQUIRE(entry["maxBytes"].is_number_unsigned());
        REQUIRE(entry["reason"].is_string());
        REQUIRE(entry["reviewStrategy"].is_string());
        REQUIRE(entry["humanReviewArtifact"].is_string());
        REQUIRE_FALSE(entry["reason"].get<std::string>().empty());
        REQUIRE_FALSE(entry["reviewStrategy"].get<std::string>().empty());
        REQUIRE_FALSE(entry["humanReviewArtifact"].get<std::string>().empty());
        governed.emplace(entry["path"].get<std::string>(), entry);
    }

    std::size_t oversizedCount = 0;
    const auto goldenRoot = getGoldenRoot();
    for (const auto& item : std::filesystem::recursive_directory_iterator(goldenRoot)) {
        if (!item.is_regular_file() || item.path().extension() != ".json") {
            continue;
        }
        const auto filename = item.path().filename().string();
        if (filename.find(".golden.") == std::string::npos) {
            continue;
        }

        const auto size = item.file_size();
        if (size <= maxUngovernedBytes) {
            continue;
        }

        oversizedCount += 1;
        const auto relative = std::filesystem::relative(item.path(), sourceRoot).generic_string();
        const auto it = governed.find(relative);
        REQUIRE(it != governed.end());
        REQUIRE(size <= it->second["maxBytes"].get<std::uintmax_t>());
    }

    REQUIRE(oversizedCount == governed.size());
}
