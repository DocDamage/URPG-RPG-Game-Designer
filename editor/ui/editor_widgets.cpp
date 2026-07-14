#include "editor/ui/editor_widgets.h"

#ifdef URPG_IMGUI_ENABLED
#include <imgui.h>
#endif

#include <string>

namespace urpg::editor::ui {
namespace {

const char* severityLabel(const EditorSeverity severity) {
    switch (severity) {
    case EditorSeverity::Info: return "Info";
    case EditorSeverity::Success: return "Success";
    case EditorSeverity::Warning: return "Warning";
    case EditorSeverity::Error: return "Error";
    }
    return "Info";
}

#ifdef URPG_IMGUI_ENABLED
ImVec4 severityColor(const EditorSeverity severity) {
    const auto tokens = defaultEditorTheme();
    switch (severity) {
    case EditorSeverity::Success: return {tokens.success.r, tokens.success.g, tokens.success.b, tokens.success.a};
    case EditorSeverity::Warning: return {tokens.warning.r, tokens.warning.g, tokens.warning.b, tokens.warning.a};
    case EditorSeverity::Error: return {tokens.error.r, tokens.error.g, tokens.error.b, tokens.error.a};
    case EditorSeverity::Info: return {tokens.selection.r, tokens.selection.g, tokens.selection.b, tokens.selection.a};
    }
    return {1, 1, 1, 1};
}
#endif

} // namespace

void renderStatusBanner(const EditorStatusBanner& banner) {
#ifdef URPG_IMGUI_ENABLED
    ImGui::PushStyleColor(ImGuiCol_Text, severityColor(banner.severity));
    ImGui::TextWrapped("%s: %.*s", severityLabel(banner.severity), static_cast<int>(banner.message.size()), banner.message.data());
    ImGui::PopStyleColor();
    if (!banner.remediation.empty()) ImGui::TextDisabled("Next: %.*s", static_cast<int>(banner.remediation.size()), banner.remediation.data());
#else
    (void)banner;
#endif
}

bool renderCommandButton(const std::string_view label, const std::string_view icon, const bool enabled,
                         const std::string_view disabledReason) {
#ifdef URPG_IMGUI_ENABLED
    const std::string text = icon.empty() ? std::string(label) : std::string(icon) + " " + std::string(label);
    ImGui::BeginDisabled(!enabled);
    const bool pressed = ImGui::Button(text.c_str());
    ImGui::EndDisabled();
    if (!enabled && !disabledReason.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
        ImGui::SetTooltip("%.*s", static_cast<int>(disabledReason.size()), disabledReason.data());
    }
    return pressed;
#else
    (void)label; (void)icon; (void)enabled; (void)disabledReason;
    return false;
#endif
}

void renderEmptyState(const std::string_view title, const std::string_view detail, const std::string_view nextAction) {
#ifdef URPG_IMGUI_ENABLED
    ImGui::TextUnformatted(title.data(), title.data() + title.size());
    ImGui::TextDisabled("%.*s", static_cast<int>(detail.size()), detail.data());
    if (!nextAction.empty()) ImGui::TextDisabled("Next: %.*s", static_cast<int>(nextAction.size()), nextAction.data());
#else
    (void)title; (void)detail; (void)nextAction;
#endif
}

void renderDiagnosticRow(const EditorSeverity severity, const std::string_view code, const std::string_view message) {
#ifdef URPG_IMGUI_ENABLED
    ImGui::TextColored(severityColor(severity), "%s", severityLabel(severity));
    ImGui::SameLine();
    ImGui::TextDisabled("%.*s", static_cast<int>(code.size()), code.data());
    ImGui::SameLine();
    ImGui::TextWrapped("%.*s", static_cast<int>(message.size()), message.data());
#else
    (void)severity; (void)code; (void)message;
#endif
}

} // namespace urpg::editor::ui
