#include "editor/localization/localization_workspace_model.h"
#include "engine/core/accessibility/accessibility_fix_advisor.h"
#include "engine/core/input/input_remap_profile.h"
#include "engine/core/platform/device_profile.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Localization workspace flags missing key and respects fallback", "[localization][ffs14]") {
    urpg::editor::localization::LocalizationWorkspaceModel workspace;
    workspace.loadSource({{"locale", "en"}, {"keys", {{"menu.start", "Start"}, {"menu.quit", "Quit"}}}});
    workspace.loadTarget({{"locale", "ja"}, {"keys", {{"menu.start", "Start JA"}}}});

    const auto snapshot = workspace.snapshot();

    REQUIRE(snapshot.missingKeys == std::vector<std::string>{"menu.quit"});
    REQUIRE(workspace.resolve("menu.quit") == "Quit");
    REQUIRE(snapshot.layoutIssues.empty());
}

TEST_CASE("Localization workspace reports glossary inconsistency and text overflow", "[localization][ffs14]") {
    urpg::editor::localization::LocalizationWorkspaceModel workspace;
    workspace.loadSource({{"locale", "en"}, {"keys", {{"term.save", "Checkpoint"}, {"menu.save", "Save"}}}});
    workspace.loadTarget({{"locale", "de"}, {"keys", {{"term.save", "Speicherpunkt"}, {"menu.save", "Sichern mit sehr langem Text"}}}});
    workspace.addGlossaryTerm({"Save", "Speichern"});
    workspace.setLayoutLimit("menu.save", 10);

    const auto snapshot = workspace.snapshot();

    REQUIRE(snapshot.glossaryIssues == std::vector<std::string>{"menu.save"});
    REQUIRE(snapshot.layoutIssues == std::vector<std::string>{"menu.save"});
}

TEST_CASE("Accessibility advisor emits focus-order and contrast fixes", "[accessibility][ffs14]") {
    urpg::accessibility::AccessibilityFixAdvisor advisor;
    const auto fixes = advisor.suggestFixes({
        {urpg::accessibility::IssueSeverity::Error, urpg::accessibility::IssueCategory::FocusOrder, "start", "focus order skips"},
        {urpg::accessibility::IssueSeverity::Warning, urpg::accessibility::IssueCategory::Contrast, "quit", "contrast low"},
    });

    REQUIRE(fixes.size() == 2);
    REQUIRE(fixes[0].elementId == "start");
    REQUIRE(fixes[0].action == "normalize_focus_order");
    REQUIRE(fixes[1].action == "raise_contrast_ratio");
}

TEST_CASE("Input remap profile rejects conflicts unless accessibility duplicate is allowed", "[input][ffs14]") {
    urpg::input::InputRemapProfile profile;
    profile.bind({"keyboard", "Space"}, urpg::input::InputAction::Confirm);

    const auto conflict = profile.validateBinding({"keyboard", "Space"}, urpg::input::InputAction::Cancel, false);
    REQUIRE_FALSE(conflict.accepted);
    REQUIRE(conflict.code == "binding_conflict");

    const auto allowed = profile.validateBinding({"keyboard", "Space"}, urpg::input::InputAction::Cancel, true);
    REQUIRE(allowed.accepted);
    profile.bind({"keyboard", "Space"}, urpg::input::InputAction::Cancel, true);
    REQUIRE(profile.bindingsFor("Space").size() == 2);
}

TEST_CASE("Device profile flags over-budget frame time and memory", "[platform][ffs14]") {
    urpg::platform::DeviceProfile profile{
        "low-end",
        16.6,
        512,
        {1280, 720},
        100,
        {"keyboard"},
    };

    const auto report = profile.evaluate({22.0, 768, {1920, 1080}, 180, {"keyboard", "controller"}});

    REQUIRE_FALSE(report.ready);
    REQUIRE(report.issues == std::vector<std::string>{"frame_time_over_budget", "memory_over_budget", "resolution_over_budget", "storage_over_budget", "missing_input_device"});
}
