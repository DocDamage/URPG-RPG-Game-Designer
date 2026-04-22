#include "engine/core/render/render_layer.h"
#include "engine/core/testing/visual_regression_harness.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>

using namespace urpg;
using namespace urpg::testing;

#ifndef URPG_HEADLESS
namespace {

std::filesystem::path getGoldenRoot() {
    return std::filesystem::path(URPG_SOURCE_DIR) / "tests" / "snapshot" / "goldens";
}

std::vector<FrameRenderCommand> makeRendererBackedCommands(bool includeText) {
    std::vector<FrameRenderCommand> commands;

    RectCommand rect;
    rect.x = 10.0f;
    rect.y = 12.0f;
    rect.w = 20.0f;
    rect.h = 10.0f;
    rect.r = 1.0f;
    rect.g = 0.0f;
    rect.b = 0.0f;
    rect.a = 1.0f;
    commands.push_back(toFrameRenderCommand(rect));

    if (includeText) {
        TextCommand text;
        text.x = 4.0f;
        text.y = 4.0f;
        text.text = "HP";
        text.fontSize = 12;
        text.r = 255;
        text.g = 255;
        text.b = 255;
        text.a = 255;
        commands.push_back(toFrameRenderCommand(text));
    }

    return commands;
}

std::vector<FrameRenderCommand> makeFullFrameRectCommands() {
    std::vector<FrameRenderCommand> commands;

    RectCommand rect;
    rect.x = 0.0f;
    rect.y = 0.0f;
    rect.w = 4.0f;
    rect.h = 4.0f;
    rect.r = 1.0f;
    rect.g = 0.0f;
    rect.b = 0.0f;
    rect.a = 1.0f;
    commands.push_back(toFrameRenderCommand(rect));

    return commands;
}

std::vector<FrameRenderCommand> makeInsetRectCommands() {
    std::vector<FrameRenderCommand> commands;

    RectCommand rect;
    rect.x = 1.0f;
    rect.y = 1.0f;
    rect.w = 5.0f;
    rect.h = 4.0f;
    rect.r = 1.0f;
    rect.g = 0.0f;
    rect.b = 0.0f;
    rect.a = 1.0f;
    commands.push_back(toFrameRenderCommand(rect));

    return commands;
}

} // namespace

TEST_CASE("Snapshot: renderer-backed visual capture matches committed full-frame rect golden",
          "[snapshot][renderer][visual_capture]") {
    VisualRegressionHarness harness;
    harness.setGoldenRoot(getGoldenRoot().string());

    std::string errorMessage;
    const auto snapshot = harness.captureOpenGLFrame(makeFullFrameRectCommands(), 4, 4, &errorMessage);
    INFO(errorMessage);
    REQUIRE(snapshot.has_value());

    const auto result = harness.compareAgainstGolden("RendererBackedCapture", "rect_full_frame", *snapshot);
    REQUIRE(result.matches);
    REQUIRE(result.errorPercentage == 0.0f);
}

TEST_CASE("Snapshot: renderer-backed visual capture matches committed clear-frame golden",
          "[snapshot][renderer][visual_capture]") {
    VisualRegressionHarness harness;
    harness.setGoldenRoot(getGoldenRoot().string());

    std::string errorMessage;
    const auto snapshot = harness.captureOpenGLFrame({}, 4, 4, &errorMessage);
    INFO(errorMessage);
    REQUIRE(snapshot.has_value());

    const auto result = harness.compareAgainstGolden("RendererBackedCapture", "clear_frame", *snapshot);
    REQUIRE(result.matches);
    REQUIRE(result.errorPercentage == 0.0f);
}

TEST_CASE("Snapshot: renderer-backed visual capture matches committed inset-rect golden",
          "[snapshot][renderer][visual_capture]") {
    VisualRegressionHarness harness;
    harness.setGoldenRoot(getGoldenRoot().string());

    std::string errorMessage;
    const auto snapshot = harness.captureOpenGLFrame(makeInsetRectCommands(), 8, 8, &errorMessage);
    INFO(errorMessage);
    REQUIRE(snapshot.has_value());

    const auto result = harness.compareAgainstGolden("RendererBackedCapture", "rect_inset_clear_border", *snapshot);
    REQUIRE(result.matches);
    REQUIRE(result.errorPercentage == 0.0f);
}

TEST_CASE("Snapshot: renderer-backed visual capture round-trips through golden comparison",
          "[snapshot][renderer][visual_capture]") {
    VisualRegressionHarness harness;

    std::string captureError;
    const auto withText = harness.captureOpenGLFrame(makeRendererBackedCommands(true), 40, 32, &captureError);
    INFO(captureError);
    REQUIRE(withText.has_value());

    const auto withoutText = harness.captureOpenGLFrame(makeRendererBackedCommands(false), 40, 32, &captureError);
    INFO(captureError);
    REQUIRE(withoutText.has_value());

    const std::filesystem::path tempRoot =
        std::filesystem::temp_directory_path() / "urpg_renderer_backed_visual_capture";
    std::filesystem::remove_all(tempRoot);
    std::filesystem::create_directories(tempRoot);

    harness.setGoldenRoot(tempRoot.string());
    REQUIRE(harness.saveGolden("RendererBackedCapture", "with_text", *withText));

    const auto identical = harness.compareAgainstGolden("RendererBackedCapture", "with_text", *withText);
    REQUIRE(identical.matches);
    REQUIRE(identical.errorPercentage == 0.0f);

    const auto different = harness.compareAgainstGolden("RendererBackedCapture", "with_text", *withoutText);
    REQUIRE_FALSE(different.matches);
    REQUIRE(different.errorPercentage > 0.0f);

    std::filesystem::remove_all(tempRoot);
}
#endif
