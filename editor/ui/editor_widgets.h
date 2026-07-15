#pragma once

#include "editor/ui/editor_theme.h"

#include <string_view>

namespace urpg::editor::ui {

enum class EditorSeverity { Info, Success, Warning, Error };

struct EditorStatusBanner {
    EditorSeverity severity = EditorSeverity::Info;
    std::string_view message;
    std::string_view remediation;
};

struct EditorAssetCard {
    std::string_view id;
    std::string_view title;
    std::string_view detail;
    std::string_view state;
    bool selected = false;
};

struct EditorProgress {
    float fraction = 0.0f;
    std::string_view label;
    std::string_view detail;
};

struct EditorDestructiveConfirmation {
    std::string_view id;
    std::string_view actionLabel;
    std::string_view detail;
    std::string_view confirmationLabel = "Confirm";
    bool enabled = true;
    std::string_view disabledReason;
};

// Small shared primitives used by release surfaces. The label is never
// optional: icons are supplementary and do not create inaccessible commands.
void renderStatusBanner(const EditorStatusBanner& banner);
bool renderCommandButton(std::string_view label, std::string_view icon = {}, bool enabled = true,
                         std::string_view disabledReason = {}, float width = 0.0f);
void renderEmptyState(std::string_view title, std::string_view detail, std::string_view nextAction = {});
void renderDiagnosticRow(EditorSeverity severity, std::string_view code, std::string_view message);
bool renderAssetCard(const EditorAssetCard& card);
bool beginInspectorSection(std::string_view label, bool defaultOpen = true);
void endInspectorSection();
void renderProgress(const EditorProgress& progress);
bool renderDestructiveConfirmation(const EditorDestructiveConfirmation& confirmation);

} // namespace urpg::editor::ui
