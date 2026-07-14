#include "editor/ui/editor_widgets.h"

#ifdef URPG_IMGUI_ENABLED
#include <imgui.h>
#endif

#include <algorithm>
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
                         const std::string_view disabledReason, const float width) {
#ifdef URPG_IMGUI_ENABLED
    const std::string text = icon.empty() ? std::string(label) : std::string(icon) + " " + std::string(label);
    ImGui::BeginDisabled(!enabled);
    const bool pressed = ImGui::Button(text.c_str(), {width, 0.0f});
    ImGui::EndDisabled();
    if (!enabled && !disabledReason.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
        ImGui::SetTooltip("%.*s", static_cast<int>(disabledReason.size()), disabledReason.data());
    }
    return pressed;
#else
    (void)label; (void)icon; (void)enabled; (void)disabledReason; (void)width;
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

bool renderAssetCard(const EditorAssetCard& card) {
#ifdef URPG_IMGUI_ENABLED
    const auto tokens = defaultEditorTheme();
    if (card.selected) {
        ImGui::PushStyleColor(ImGuiCol_ChildBg,
                              {tokens.selection.r, tokens.selection.g, tokens.selection.b, tokens.selection.a});
    }
    const std::string childId = "asset_card##" + std::string(card.id);
    ImGui::BeginChild(childId.c_str(), {0.0f, 0.0f}, ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);
    const bool selected = ImGui::Selectable(std::string(card.title).c_str(), card.selected,
                                            ImGuiSelectableFlags_AllowDoubleClick);
    if (!card.state.empty()) {
        ImGui::SameLine();
        ImGui::TextDisabled("%.*s", static_cast<int>(card.state.size()), card.state.data());
    }
    if (!card.detail.empty()) ImGui::TextWrapped("%.*s", static_cast<int>(card.detail.size()), card.detail.data());
    ImGui::EndChild();
    if (card.selected) ImGui::PopStyleColor();
    return selected;
#else
    (void)card;
    return false;
#endif
}

bool beginInspectorSection(const std::string_view label, const bool defaultOpen) {
#ifdef URPG_IMGUI_ENABLED
    if (defaultOpen) ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    return ImGui::CollapsingHeader(std::string(label).c_str(), ImGuiTreeNodeFlags_DefaultOpen);
#else
    (void)label; (void)defaultOpen;
    return false;
#endif
}

void endInspectorSection() {
    // CollapsingHeader owns its scope; this symmetric helper keeps callers readable.
}

void renderProgress(const EditorProgress& progress) {
#ifdef URPG_IMGUI_ENABLED
    const float fraction = std::clamp(progress.fraction, 0.0f, 1.0f);
    const std::string overlay = progress.label.empty() ? std::to_string(static_cast<int>(fraction * 100.0f)) + "%"
                                                        : std::string(progress.label);
    ImGui::ProgressBar(fraction, {-1.0f, 0.0f}, overlay.c_str());
    if (!progress.detail.empty()) ImGui::TextDisabled("%.*s", static_cast<int>(progress.detail.size()), progress.detail.data());
#else
    (void)progress;
#endif
}

bool renderDestructiveConfirmation(const EditorDestructiveConfirmation& confirmation) {
#ifdef URPG_IMGUI_ENABLED
    const std::string popupId = "confirm_destructive##" + std::string(confirmation.id);
    bool confirmed = false;
    if (renderCommandButton(confirmation.actionLabel, "!", confirmation.enabled, confirmation.disabledReason)) {
        ImGui::OpenPopup(popupId.c_str());
    }
    if (ImGui::BeginPopupModal(popupId.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextWrapped("%.*s", static_cast<int>(confirmation.detail.size()), confirmation.detail.data());
        ImGui::Separator();
        if (ImGui::Button(std::string(confirmation.confirmationLabel).c_str())) {
            confirmed = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    return confirmed;
#else
    (void)confirmation;
    return false;
#endif
}

} // namespace urpg::editor::ui
