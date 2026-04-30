#include <catch2/catch_test_macros.hpp>

#include "editor/action/controller_binding_panel.h"

using urpg::action::ControllerBindingRuntime;
using urpg::action::ControllerButton;
using urpg::editor::ControllerBindingPanel;

TEST_CASE("ControllerBindingPanel returns empty snapshot without bound runtime", "[input][controller][editor][panel]") {
    ControllerBindingPanel panel;
    panel.render();

    const auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["binding_count"] == 0);
    REQUIRE(snapshot["unsaved_changes"] == false);
    REQUIRE(snapshot["bindings"].empty());
    REQUIRE(snapshot["missing_required_actions"].empty());
    REQUIRE(snapshot["issue_count"] == 0);
    REQUIRE(snapshot["issues"].empty());
}

TEST_CASE("ControllerBindingPanel surfaces binding rows and validation issues", "[input][controller][editor][panel]") {
    ControllerBindingRuntime runtime;
    runtime.clearBinding(ControllerButton::FaceRight);

    ControllerBindingPanel panel;
    panel.bindRuntime(&runtime);
    panel.render();

    const auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["binding_count"] == 13);
    REQUIRE(snapshot["unsaved_changes"] == true);
    REQUIRE(snapshot["bindings"].size() == 13);
    REQUIRE(snapshot["missing_required_actions"].size() == 1);
    REQUIRE(snapshot["missing_required_actions"][0] == "Cancel");
    REQUIRE(snapshot["issue_count"] == 1);
    REQUIRE(snapshot["issues"].size() == 1);
    REQUIRE(snapshot["issues"][0]["category"] == "missing_required_action");
    REQUIRE(snapshot["issues"][0]["action"] == "Cancel");
}
