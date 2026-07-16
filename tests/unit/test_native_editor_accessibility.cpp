#include <catch2/catch_test_macros.hpp>

#include "editor/accessibility/native_editor_accessibility.h"
#include "engine/core/editor/editor_shell.h"

TEST_CASE("Native editor accessibility tree mirrors panel registry and routes default actions",
          "[accessibility][native_bridge][pcq656]") {
    urpg::editor::EditorShell shell;
    shell.setProjectRoot("projects/example");
    REQUIRE(shell.addPanel({"map", "Map", "Authoring"}, [](const auto&) {}));
    REQUIRE(shell.addPanel({"diagnostics", "Diagnostics", "Quality"}, [](const auto&) {}));
    REQUIRE(shell.start());

    auto snapshot = urpg::editor::nativeAccessibilitySnapshotForEditorShell(shell);
    REQUIRE(snapshot.name == "URPG Editor");
    REQUIRE(snapshot.nodes.size() == 3);
    CHECK(snapshot.nodes[0].id == "project.current");
    CHECK(snapshot.nodes[0].value == "projects/example");
    CHECK_FALSE(snapshot.nodes[0].focusable);
    CHECK(snapshot.nodes[1].id == "panel.map");
    CHECK(snapshot.nodes[1].role == urpg::editor::NativeAccessibilityRole::Button);
    CHECK(snapshot.nodes[1].default_action == "Open");
    CHECK(snapshot.nodes[1].selected);
    CHECK_FALSE(snapshot.nodes[2].selected);

    REQUIRE(urpg::editor::activateNativeEditorAccessibilityNode(shell, "panel.diagnostics"));
    REQUIRE(shell.activePanelId() == "diagnostics");
    snapshot = urpg::editor::nativeAccessibilitySnapshotForEditorShell(shell);
    CHECK_FALSE(snapshot.nodes[1].selected);
    CHECK(snapshot.nodes[2].selected);
    CHECK_FALSE(urpg::editor::activateNativeEditorAccessibilityNode(shell, "project.current"));
    CHECK_FALSE(urpg::editor::activateNativeEditorAccessibilityNode(shell, "panel.missing"));
}

TEST_CASE("Native editor accessibility tree retains disabled panels without making them actionable",
          "[accessibility][native_bridge][pcq656]") {
    urpg::editor::EditorShell shell;
    REQUIRE(shell.addPanel({"map", "Map", "Authoring"}, [](const auto&) {}));
    REQUIRE(shell.addPanel({"package", "Package", "Release", true, false}, [](const auto&) {}));
    REQUIRE(shell.start());

    const auto snapshot = urpg::editor::nativeAccessibilitySnapshotForEditorShell(shell);
    REQUIRE(snapshot.nodes.size() == 3);
    CHECK(snapshot.nodes[2].id == "panel.package");
    CHECK_FALSE(snapshot.nodes[2].enabled);
    CHECK_FALSE(snapshot.nodes[2].focusable);
    CHECK_FALSE(urpg::editor::activateNativeEditorAccessibilityNode(shell, "panel.package"));
    CHECK(shell.activePanelId() == "map");
}
