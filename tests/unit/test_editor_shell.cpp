#include "engine/core/editor/editor_shell.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("EditorShell registers panels and selects the first reachable panel", "[editor][shell]") {
    urpg::editor::EditorShell shell;
    int renderCount = 0;

    REQUIRE(shell.addPanel(urpg::editor::EditorPanelDescriptor{"diagnostics", "Diagnostics", "System"},
                           [&](const urpg::editor::EditorFrameContext& context) {
                               REQUIRE(context.headless);
                               ++renderCount;
                           }));
    REQUIRE_FALSE(shell.addPanel(urpg::editor::EditorPanelDescriptor{"diagnostics", "Duplicate", "System"},
                                 [](const urpg::editor::EditorFrameContext&) {}));

    REQUIRE(shell.start(true));
    REQUIRE(shell.isRunning());
    REQUIRE(shell.activePanelId() == "diagnostics");
    REQUIRE(shell.renderFrame(1.0 / 60.0));

    const auto snapshot = shell.snapshot();
    REQUIRE(snapshot.running);
    REQUIRE(snapshot.headless);
    REQUIRE(snapshot.frame_index == 1);
    REQUIRE(snapshot.panels.size() == 1);
    REQUIRE(snapshot.panels[0].rendered_last_frame);
    REQUIRE(snapshot.panels[0].render_count == 1);
    REQUIRE(renderCount == 1);
}

TEST_CASE("EditorShell routes visibility and active panel navigation", "[editor][shell]") {
    urpg::editor::EditorShell shell;
    int diagnosticsRenderCount = 0;
    int assetsRenderCount = 0;

    REQUIRE(shell.addPanel(urpg::editor::EditorPanelDescriptor{"diagnostics", "Diagnostics", "System"},
                           [&](const urpg::editor::EditorFrameContext&) { ++diagnosticsRenderCount; }));
    REQUIRE(shell.addPanel(urpg::editor::EditorPanelDescriptor{"assets", "Assets", "Content"},
                           [&](const urpg::editor::EditorFrameContext&) { ++assetsRenderCount; }));

    REQUIRE(shell.start(false));
    REQUIRE(shell.openPanel("assets"));
    REQUIRE(shell.activePanelId() == "assets");
    REQUIRE(shell.renderFrame(1.0 / 30.0));
    REQUIRE(diagnosticsRenderCount == 0);
    REQUIRE(assetsRenderCount == 1);

    REQUIRE(shell.setPanelVisible("assets", false));
    REQUIRE_FALSE(shell.renderFrame(1.0 / 30.0));
    REQUIRE(assetsRenderCount == 1);

    REQUIRE(shell.openPanel("diagnostics"));
    REQUIRE(shell.beginFrame(1.0 / 30.0));
    REQUIRE(shell.renderVisiblePanels() == 1);
    REQUIRE(shell.endFrame());
    REQUIRE(diagnosticsRenderCount == 1);
    REQUIRE(assetsRenderCount == 1);
}

TEST_CASE("EditorShell carries project and runtime preview context", "[editor][shell]") {
    urpg::editor::EditorShell shell;

    shell.setProjectRoot("fixtures/project");
    shell.setRuntimePreviewId("EditorPreview");
    REQUIRE(shell.addPanel(urpg::editor::EditorPanelDescriptor{"mod", "Mod Manager", "Runtime"},
                           [](const urpg::editor::EditorFrameContext&) {}));
    REQUIRE(shell.start(true));

    const auto snapshot = shell.snapshot();
    REQUIRE(snapshot.project_root.generic_string() == "fixtures/project");
    REQUIRE(snapshot.runtime_preview_id == "EditorPreview");
    REQUIRE(snapshot.panels[0].category == "Runtime");
}
