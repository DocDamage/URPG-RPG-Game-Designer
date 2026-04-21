#include <catch2/catch_test_macros.hpp>
#include "editor/accessibility/accessibility_panel.h"
#include "engine/core/accessibility/accessibility_auditor.h"

using namespace urpg::editor;
using namespace urpg::accessibility;

TEST_CASE("AccessibilityPanel: Empty snapshot when no auditor bound", "[accessibility][editor][panel]") {
    AccessibilityPanel panel;
    panel.render();
    auto snapshot = panel.lastRenderSnapshot();

    REQUIRE(snapshot.is_object());
    REQUIRE(snapshot.empty());
}

TEST_CASE("AccessibilityPanel: Snapshot reflects issues after audit", "[accessibility][editor][panel]") {
    AccessibilityAuditor auditor;
    AccessibilityPanel panel;
    panel.bindAuditor(&auditor);

    auditor.ingestElements({
        UiElementSnapshot{"btn_ok", "", true, 1, 4.5f},
        UiElementSnapshot{"text_1", "Low Contrast", false, 0, 2.5f}
    });

    panel.render();
    auto snapshot = panel.lastRenderSnapshot();

    REQUIRE(snapshot["issueCount"] == 2);
    REQUIRE(snapshot["issues"].is_array());
    REQUIRE(snapshot["issues"].size() == 2);
}

TEST_CASE("AccessibilityPanel: Counts are accurate", "[accessibility][editor][panel]") {
    AccessibilityAuditor auditor;
    AccessibilityPanel panel;
    panel.bindAuditor(&auditor);

    auditor.ingestElements({
        UiElementSnapshot{"btn_a", "", true, 1, 4.5f},
        UiElementSnapshot{"btn_b", "", true, 1, 4.5f},
        UiElementSnapshot{"text_1", "Low Contrast", false, 0, 2.5f}
    });

    panel.render();
    auto snapshot = panel.lastRenderSnapshot();

    REQUIRE(snapshot["issueCount"] == 5);
    REQUIRE(snapshot["errorCount"] == 3);
    REQUIRE(snapshot["warningCount"] == 2);
}
