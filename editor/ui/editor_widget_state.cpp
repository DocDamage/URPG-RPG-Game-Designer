#include "editor/ui/editor_widget_state.h"

#include <array>
#include <algorithm>
#include <cmath>
#include <set>
#include <utility>

#ifdef URPG_IMGUI_ENABLED
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#endif

namespace urpg::editor {
namespace {

bool interactive(const EditorWidgetKind kind) {
    return kind == EditorWidgetKind::Button || kind == EditorWidgetKind::SegmentedControl ||
           kind == EditorWidgetKind::Field || kind == EditorWidgetKind::Picker || kind == EditorWidgetKind::Tree ||
           kind == EditorWidgetKind::Tabs || kind == EditorWidgetKind::Table ||
           kind == EditorWidgetKind::Confirmation || kind == EditorWidgetKind::CommandPreview;
}

const char* intentRole(const EditorWidgetIntent intent) {
    switch (intent) {
    case EditorWidgetIntent::Primary: return "accent";
    case EditorWidgetIntent::Secondary: return "surface_raised";
    case EditorWidgetIntent::Destructive: return "danger";
    case EditorWidgetIntent::Success: return "success";
    case EditorWidgetIntent::Warning: return "warning";
    case EditorWidgetIntent::Neutral: return "surface_raised";
    }
    return "surface_raised";
}

} // namespace

const char* editorWidgetKindName(const EditorWidgetKind kind) {
    switch (kind) {
    case EditorWidgetKind::Button: return "button";
    case EditorWidgetKind::SegmentedControl: return "segmented_control";
    case EditorWidgetKind::Card: return "card";
    case EditorWidgetKind::Field: return "field";
    case EditorWidgetKind::Picker: return "picker";
    case EditorWidgetKind::Tree: return "tree";
    case EditorWidgetKind::Tabs: return "tabs";
    case EditorWidgetKind::Table: return "table";
    case EditorWidgetKind::Toast: return "toast";
    case EditorWidgetKind::Banner: return "banner";
    case EditorWidgetKind::ProgressJob: return "progress_job";
    case EditorWidgetKind::EmptyState: return "empty_state";
    case EditorWidgetKind::Diagnostic: return "diagnostic";
    case EditorWidgetKind::Confirmation: return "confirmation";
    case EditorWidgetKind::CommandPreview: return "command_preview";
    }
    return "unknown";
}

const char* editorWidgetStateName(const EditorWidgetVisualState state) {
    switch (state) {
    case EditorWidgetVisualState::Normal: return "normal";
    case EditorWidgetVisualState::Hover: return "hover";
    case EditorWidgetVisualState::Focused: return "focused";
    case EditorWidgetVisualState::Pressed: return "pressed";
    case EditorWidgetVisualState::Disabled: return "disabled";
    case EditorWidgetVisualState::Loading: return "loading";
    case EditorWidgetVisualState::Error: return "error";
    }
    return "unknown";
}

EditorWidgetSnapshot resolveEditorWidgetState(const EditorWidgetDescriptor& descriptor, const float requestedScale) {
    EditorWidgetSnapshot result;
    if (descriptor.id.empty() || descriptor.label.empty()) {
        result.code = "editor_widget_identity_missing";
        return result;
    }
    if (descriptor.state == EditorWidgetVisualState::Error && descriptor.diagnostic.empty()) {
        result.code = "editor_widget_error_diagnostic_missing";
        return result;
    }
    result.valid = true;
    result.scale = std::isfinite(requestedScale) ? std::clamp(requestedScale, 0.5F, 3.0F) : 1.0F;
    result.minimum_hit_target = std::max(30.0F, 40.0F * result.scale);
    result.icon_size = 20.0F * result.scale;
    result.content_padding = 8.0F * result.scale;
    result.code = std::string("editor_widget_") + editorWidgetStateName(descriptor.state);
    result.color_role = descriptor.state == EditorWidgetVisualState::Error ? "danger" : intentRole(descriptor.intent);
    result.border_role = descriptor.state == EditorWidgetVisualState::Focused ? "focus" :
                         (descriptor.state == EditorWidgetVisualState::Error ? "danger" : "hairline");
    result.focus_visible = descriptor.state == EditorWidgetVisualState::Focused;
    result.busy = descriptor.state == EditorWidgetVisualState::Loading;
    result.action_enabled = interactive(descriptor.kind) && descriptor.state != EditorWidgetVisualState::Disabled &&
                            descriptor.state != EditorWidgetVisualState::Loading &&
                            descriptor.state != EditorWidgetVisualState::Error;
    result.accessible_name = descriptor.accessible_name.empty() ? descriptor.label : descriptor.accessible_name;
    result.keyboard_action = descriptor.keyboard_action;
    result.controller_action = descriptor.controller_action;
    result.canvas_alternative = descriptor.canvas_alternative;
    result.focus_order = descriptor.focus_order;
    result.keyboard_reachable = !descriptor.keyboard_action.empty() && descriptor.focus_order >= 0;
    result.controller_reachable = !descriptor.controller_action.empty() && descriptor.focus_order >= 0;
    result.has_canvas_alternative = !descriptor.canvas_alternative.empty();
    if (result.busy) result.announcement = descriptor.label + " is working.";
    else if (descriptor.state == EditorWidgetVisualState::Error) result.announcement = descriptor.diagnostic;
    else if (descriptor.state == EditorWidgetVisualState::Disabled && !descriptor.help.empty())
        result.announcement = descriptor.help;
    return result;
}

EditorWidgetRenderResult renderEditorWidget(const EditorWidgetDescriptor& descriptor, const float scale,
                                            const float progress) {
    EditorWidgetRenderResult result;
    result.snapshot = resolveEditorWidgetState(descriptor, scale);
    if (!result.snapshot.valid) return result;
#ifdef URPG_IMGUI_ENABLED
    if (ImGui::GetCurrentContext() == nullptr) return result;
    result.rendered = true;
    ImGui::PushID(descriptor.id.c_str());
    if (!result.snapshot.action_enabled) ImGui::BeginDisabled();
    switch (descriptor.kind) {
    case EditorWidgetKind::Button:
    case EditorWidgetKind::Confirmation:
        result.activated = ImGui::Button(descriptor.label.c_str(),
                                        ImVec2(0.0F, result.snapshot.minimum_hit_target));
        break;
    case EditorWidgetKind::SegmentedControl:
        result.activated = ImGui::SmallButton(descriptor.label.c_str());
        break;
    case EditorWidgetKind::Card:
        if (ImGui::BeginChild("card", ImVec2(0.0F, result.snapshot.minimum_hit_target * 2.0F),
                              ImGuiChildFlags_Borders)) {
            ImGui::TextUnformatted(descriptor.label.c_str());
            if (!descriptor.help.empty()) ImGui::TextWrapped("%s", descriptor.help.c_str());
        }
        ImGui::EndChild();
        break;
    case EditorWidgetKind::Field: {
        auto value = descriptor.help;
        result.activated = ImGui::InputText(descriptor.label.c_str(), &value);
        break;
    }
    case EditorWidgetKind::Picker:
        if (ImGui::BeginCombo(descriptor.label.c_str(), descriptor.help.empty() ? "Choose" : descriptor.help.c_str())) {
            result.activated = ImGui::Selectable(descriptor.label.c_str());
            ImGui::EndCombo();
        }
        break;
    case EditorWidgetKind::Tree:
        if (ImGui::TreeNodeEx(descriptor.label.c_str())) {
            ImGui::TextWrapped("%s", descriptor.help.c_str());
            ImGui::TreePop();
        }
        break;
    case EditorWidgetKind::Tabs:
        if (ImGui::BeginTabBar("tabs")) {
            if (ImGui::BeginTabItem(descriptor.label.c_str())) ImGui::EndTabItem();
            ImGui::EndTabBar();
        }
        break;
    case EditorWidgetKind::Table:
        if (ImGui::BeginTable("table", 1, ImGuiTableFlags_Borders)) {
            ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted(descriptor.label.c_str());
            ImGui::EndTable();
        }
        break;
    case EditorWidgetKind::ProgressJob:
        ImGui::ProgressBar(std::clamp(progress, 0.0F, 1.0F), ImVec2(-1.0F, 0.0F), descriptor.label.c_str());
        break;
    case EditorWidgetKind::Toast:
    case EditorWidgetKind::Banner:
    case EditorWidgetKind::Diagnostic:
        ImGui::SeparatorText(descriptor.label.c_str());
        if (!descriptor.help.empty()) ImGui::TextWrapped("%s", descriptor.help.c_str());
        break;
    case EditorWidgetKind::EmptyState:
        ImGui::TextDisabled("%s", descriptor.label.c_str());
        if (!descriptor.help.empty()) ImGui::TextWrapped("%s", descriptor.help.c_str());
        break;
    case EditorWidgetKind::CommandPreview:
        result.activated = ImGui::Selectable(descriptor.label.c_str());
        if (!descriptor.help.empty() && ImGui::IsItemHovered()) ImGui::SetTooltip("%s", descriptor.help.c_str());
        break;
    }
    if (!result.snapshot.action_enabled) ImGui::EndDisabled();
    if (!result.snapshot.announcement.empty() && ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", result.snapshot.announcement.c_str());
    }
    ImGui::PopID();
#else
    (void)progress;
#endif
    return result;
}

std::vector<EditorWidgetDescriptor> buildEditorWidgetStateMatrix() {
    constexpr std::array kinds{EditorWidgetKind::Button, EditorWidgetKind::SegmentedControl, EditorWidgetKind::Card,
                               EditorWidgetKind::Field, EditorWidgetKind::Picker, EditorWidgetKind::Tree,
                               EditorWidgetKind::Tabs, EditorWidgetKind::Table, EditorWidgetKind::Toast,
                               EditorWidgetKind::Banner, EditorWidgetKind::ProgressJob, EditorWidgetKind::EmptyState,
                               EditorWidgetKind::Diagnostic, EditorWidgetKind::Confirmation,
                               EditorWidgetKind::CommandPreview};
    constexpr std::array states{EditorWidgetVisualState::Normal, EditorWidgetVisualState::Hover,
                                EditorWidgetVisualState::Focused, EditorWidgetVisualState::Pressed,
                                EditorWidgetVisualState::Disabled, EditorWidgetVisualState::Loading,
                                EditorWidgetVisualState::Error};
    std::vector<EditorWidgetDescriptor> result;
    int focusOrder = 0;
    for (const auto kind : kinds) {
        for (const auto state : states) {
            const auto id = std::string(editorWidgetKindName(kind)) + "." + editorWidgetStateName(state);
            EditorWidgetDescriptor descriptor{id, editorWidgetKindName(kind), kind, EditorWidgetIntent::Primary, state,
                                               "This action is currently unavailable.",
                                               state == EditorWidgetVisualState::Error
                                                   ? "The preview contains an error."
                                                   : ""};
            descriptor.accessible_name = std::string(editorWidgetKindName(kind)) + " " + editorWidgetStateName(state);
            if (interactive(kind)) {
                descriptor.keyboard_action = kind == EditorWidgetKind::Field
                    ? "Tab to focus; type to edit; Escape to cancel."
                    : "Tab to focus; Enter or Space to activate.";
                descriptor.controller_action = kind == EditorWidgetKind::Field
                    ? "D-pad to focus; Confirm to edit; Cancel to stop editing."
                    : "D-pad to focus; Confirm to activate.";
                descriptor.canvas_alternative = kind == EditorWidgetKind::Tree || kind == EditorWidgetKind::Table
                    ? "Use the structured row list with arrow-key and D-pad navigation."
                    : "Use the ordered control list without pointer input.";
                descriptor.focus_order = focusOrder++;
            }
            result.push_back(std::move(descriptor));
        }
    }
    return result;
}

std::vector<EditorWidgetAccessibilityIssue> auditEditorWidgetAccessibility(
    const std::vector<EditorWidgetDescriptor>& descriptors) {
    std::vector<EditorWidgetAccessibilityIssue> issues;
    std::set<int> focusOrders;
    for (const auto& descriptor : descriptors) {
        if (!interactive(descriptor.kind)) continue;
        const auto append = [&](const char* code, const char* message) {
            issues.push_back({descriptor.id, code, message});
        };
        if (descriptor.accessible_name.empty()) append("accessible_name_missing", "Interactive widget needs an accessible name.");
        if (descriptor.focus_order < 0) append("focus_order_missing", "Interactive widget needs a non-negative focus order.");
        else if (!focusOrders.insert(descriptor.focus_order).second)
            append("focus_order_duplicate", "Interactive widget focus order must be unique in the audited surface.");
        if (descriptor.keyboard_action.empty()) append("keyboard_semantics_missing", "Interactive widget needs keyboard semantics.");
        if (descriptor.controller_action.empty()) append("controller_semantics_missing", "Interactive widget needs controller semantics.");
        if (descriptor.canvas_alternative.empty()) append("pointer_alternative_missing", "Interactive widget needs a non-pointer alternative.");
    }
    return issues;
}

} // namespace urpg::editor
