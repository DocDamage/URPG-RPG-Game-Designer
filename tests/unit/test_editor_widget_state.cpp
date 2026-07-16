#include "editor/ui/editor_widget_state.h"

#include <catch2/catch_test_macros.hpp>

#include <set>

using namespace urpg::editor;

TEST_CASE("Editor shared widget matrix covers every family and required state", "[editor][widget_state]") {
    const auto matrix = buildEditorWidgetStateMatrix();
    REQUIRE(matrix.size() == 15 * 7);
    std::set<std::string> ids;
    std::set<std::string> kinds;
    for (const auto& descriptor : matrix) {
        REQUIRE(ids.insert(descriptor.id).second);
        kinds.insert(editorWidgetKindName(descriptor.kind));
        const auto snapshot = resolveEditorWidgetState(descriptor);
        REQUIRE(snapshot.valid);
        REQUIRE_FALSE(snapshot.color_role.empty());
        REQUIRE_FALSE(snapshot.border_role.empty());
        if (descriptor.state == EditorWidgetVisualState::Focused) REQUIRE(snapshot.focus_visible);
        if (descriptor.state == EditorWidgetVisualState::Loading) {
            REQUIRE(snapshot.busy);
            REQUIRE_FALSE(snapshot.action_enabled);
            REQUIRE_FALSE(snapshot.announcement.empty());
        }
        if (descriptor.state == EditorWidgetVisualState::Disabled || descriptor.state == EditorWidgetVisualState::Error)
            REQUIRE_FALSE(snapshot.action_enabled);
        if (snapshot.action_enabled || descriptor.focus_order >= 0) {
            REQUIRE_FALSE(snapshot.accessible_name.empty());
            REQUIRE(snapshot.focus_order >= 0);
            REQUIRE(snapshot.keyboard_reachable);
            REQUIRE(snapshot.controller_reachable);
            REQUIRE(snapshot.has_canvas_alternative);
        }
    }
    REQUIRE(kinds.size() == 15);
    REQUIRE(auditEditorWidgetAccessibility(matrix).empty());
}

TEST_CASE("Editor destructive and invalid widget states resolve safely", "[editor][widget_state]") {
    const auto destructive = resolveEditorWidgetState(
        {"delete", "Delete", EditorWidgetKind::Button, EditorWidgetIntent::Destructive,
         EditorWidgetVisualState::Focused, "", ""});
    REQUIRE(destructive.valid);
    REQUIRE(destructive.action_enabled);
    REQUIRE(destructive.color_role == "danger");
    REQUIRE(destructive.border_role == "focus");
    REQUIRE_FALSE(resolveEditorWidgetState({}).valid);
    REQUIRE_FALSE(resolveEditorWidgetState(
        {"field", "Name", EditorWidgetKind::Field, EditorWidgetIntent::Neutral,
         EditorWidgetVisualState::Error, "", ""}).valid);
}

TEST_CASE("Editor widget accessibility audit rejects missing and duplicate navigation contracts",
          "[editor][widget_state][accessibility]") {
    EditorWidgetDescriptor first{"first", "First", EditorWidgetKind::Button, EditorWidgetIntent::Primary,
                                 EditorWidgetVisualState::Normal, "", ""};
    first.accessible_name = "First action";
    first.keyboard_action = "Enter activates.";
    first.controller_action = "Confirm activates.";
    first.canvas_alternative = "Use the ordered control list.";
    first.focus_order = 1;
    auto second = first;
    second.id = "second";
    second.accessible_name.clear();
    second.keyboard_action.clear();
    second.controller_action.clear();
    second.canvas_alternative.clear();

    const auto issues = auditEditorWidgetAccessibility({first, second});
    std::set<std::string> codes;
    for (const auto& issue : issues) codes.insert(issue.code);
    REQUIRE(codes.contains("accessible_name_missing"));
    REQUIRE(codes.contains("focus_order_duplicate"));
    REQUIRE(codes.contains("keyboard_semantics_missing"));
    REQUIRE(codes.contains("controller_semantics_missing"));
    REQUIRE(codes.contains("pointer_alternative_missing"));
}
