#pragma once

#include <string>
#include <utility>
#include <vector>

namespace urpg::editor {

enum class EditorWidgetKind {
    Button, SegmentedControl, Card, Field, Picker, Tree, Tabs, Table, Toast, Banner, ProgressJob,
    EmptyState, Diagnostic, Confirmation, CommandPreview
};
enum class EditorWidgetIntent { Neutral, Primary, Secondary, Destructive, Success, Warning };
enum class EditorWidgetVisualState { Normal, Hover, Focused, Pressed, Disabled, Loading, Error };

struct EditorWidgetDescriptor {
    EditorWidgetDescriptor() = default;
    EditorWidgetDescriptor(std::string widget_id, std::string widget_label, EditorWidgetKind widget_kind,
                           EditorWidgetIntent widget_intent, EditorWidgetVisualState widget_state,
                           std::string widget_help, std::string widget_diagnostic)
        : id(std::move(widget_id)), label(std::move(widget_label)), kind(widget_kind), intent(widget_intent),
          state(widget_state), help(std::move(widget_help)), diagnostic(std::move(widget_diagnostic)) {}

    std::string id;
    std::string label;
    EditorWidgetKind kind = EditorWidgetKind::Button;
    EditorWidgetIntent intent = EditorWidgetIntent::Neutral;
    EditorWidgetVisualState state = EditorWidgetVisualState::Normal;
    std::string help;
    std::string diagnostic;
    std::string accessible_name;
    std::string keyboard_action;
    std::string controller_action;
    std::string canvas_alternative;
    int focus_order = -1;
};

struct EditorWidgetSnapshot {
    bool valid = false;
    bool action_enabled = false;
    bool focus_visible = false;
    bool busy = false;
    std::string code;
    std::string color_role;
    std::string border_role;
    std::string announcement;
    std::string accessible_name;
    std::string keyboard_action;
    std::string controller_action;
    std::string canvas_alternative;
    int focus_order = -1;
    bool keyboard_reachable = false;
    bool controller_reachable = false;
    bool has_canvas_alternative = false;
    float scale = 1.0F;
    float minimum_hit_target = 40.0F;
    float icon_size = 20.0F;
    float content_padding = 8.0F;
};

struct EditorWidgetRenderResult {
    EditorWidgetSnapshot snapshot;
    bool rendered = false;
    bool activated = false;
};

struct EditorWidgetAccessibilityIssue {
    std::string widget_id;
    std::string code;
    std::string message;
};

EditorWidgetSnapshot resolveEditorWidgetState(const EditorWidgetDescriptor& descriptor, float scale = 1.0F);
EditorWidgetRenderResult renderEditorWidget(const EditorWidgetDescriptor& descriptor, float scale = 1.0F,
                                            float progress = 0.0F);
std::vector<EditorWidgetDescriptor> buildEditorWidgetStateMatrix();
std::vector<EditorWidgetAccessibilityIssue> auditEditorWidgetAccessibility(
    const std::vector<EditorWidgetDescriptor>& descriptors);
const char* editorWidgetKindName(EditorWidgetKind kind);
const char* editorWidgetStateName(EditorWidgetVisualState state);

} // namespace urpg::editor
