#include "engine/core/testing/visual_regression_harness.h"

#include <fstream>
#include <filesystem>

namespace urpg::testing {

void VisualRegressionHarness::setGoldenRoot(const std::string& path) {
    m_goldenRoot = path;
}

std::string VisualRegressionHarness::buildGoldenPath(const std::string& testName,
                                                     const std::string& snapshotId) const {
    std::filesystem::path root(m_goldenRoot);
    std::string filename = testName + "_" + snapshotId + ".golden.json";
    return (root / filename).string();
}

std::optional<GoldenSnapshot> VisualRegressionHarness::loadGolden(const std::string& testName,
                                                                  const std::string& snapshotId) {
    std::string path = buildGoldenPath(testName, snapshotId);
    if (!std::filesystem::exists(path)) {
        return std::nullopt;
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        return std::nullopt;
    }

    nlohmann::json j;
    try {
        file >> j;
    } catch (...) {
        return std::nullopt;
    }

    GoldenSnapshot golden;
    golden.testName = testName;
    golden.snapshotId = snapshotId;

    if (!j.contains("width") || !j.contains("height") || !j.contains("pixels")) {
        return std::nullopt;
    }

    golden.width = j["width"].get<int>();
    golden.height = j["height"].get<int>();

    const auto& pixels = j["pixels"];
    golden.pixels.reserve(pixels.size());
    for (const auto& p : pixels) {
        SnapshotPixel pixel;
        pixel.r = p.value("r", 0);
        pixel.g = p.value("g", 0);
        pixel.b = p.value("b", 0);
        pixel.a = p.value("a", 0);
        golden.pixels.push_back(pixel);
    }

    return golden;
}

bool VisualRegressionHarness::saveGolden(const std::string& testName,
                                         const std::string& snapshotId,
                                         const SceneSnapshot& snapshot) {
    std::string path = buildGoldenPath(testName, snapshotId);

    std::filesystem::path dir = std::filesystem::path(path).parent_path();
    if (!dir.empty() && !std::filesystem::exists(dir)) {
        std::filesystem::create_directories(dir);
    }

    nlohmann::json j;
    j["width"] = snapshot.width;
    j["height"] = snapshot.height;

    nlohmann::json pixels = nlohmann::json::array();
    for (const auto& p : snapshot.pixels) {
        nlohmann::json pixel;
        pixel["r"] = p.r;
        pixel["g"] = p.g;
        pixel["b"] = p.b;
        pixel["a"] = p.a;
        pixels.push_back(pixel);
    }
    j["pixels"] = pixels;

    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }

    file << j.dump(4);
    return file.good();
}

SnapshotComparisonResult VisualRegressionHarness::compareAgainstGolden(const std::string& testName,
                                                                       const std::string& snapshotId,
                                                                       const SceneSnapshot& current,
                                                                       float thresholdPercentage) {
    auto goldenOpt = loadGolden(testName, snapshotId);
    if (!goldenOpt.has_value()) {
        return {.matches = false, .errorPercentage = 100.0f};
    }

    const GoldenSnapshot& golden = goldenOpt.value();
    SceneSnapshot goldenScene;
    goldenScene.width = golden.width;
    goldenScene.height = golden.height;
    goldenScene.pixels = golden.pixels;

    return SnapshotValidator::compare(goldenScene, current, thresholdPercentage);
}

SceneSnapshot VisualRegressionHarness::generateDiffHeatmap(const SceneSnapshot& golden,
                                                           const SceneSnapshot& current) {
    SceneSnapshot heatmap;
    heatmap.width = golden.width;
    heatmap.height = golden.height;

    if (golden.width != current.width || golden.height != current.height ||
        golden.pixels.size() != current.pixels.size()) {
        heatmap.pixels.assign(static_cast<size_t>(golden.width) * golden.height, {255, 0, 0, 255});
        return heatmap;
    }

    heatmap.pixels.reserve(golden.pixels.size());
    for (size_t i = 0; i < golden.pixels.size(); ++i) {
        if (golden.pixels[i] == current.pixels[i]) {
            heatmap.pixels.push_back({0, 0, 0, 255});
        } else {
            heatmap.pixels.push_back({255, 0, 0, 255});
        }
    }

    return heatmap;
}

nlohmann::json VisualRegressionHarness::buildReportJson(const std::string& testName,
                                                        const SnapshotComparisonResult& result) const {
    nlohmann::json j;
    j["testName"] = testName;
    j["matches"] = result.matches;
    j["errorPercentage"] = result.errorPercentage;
    return j;
}

} // namespace urpg::testing
