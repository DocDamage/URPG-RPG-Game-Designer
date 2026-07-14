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

// Small shared primitives used by release surfaces. The label is never
// optional: icons are supplementary and do not create inaccessible commands.
void renderStatusBanner(const EditorStatusBanner& banner);
bool renderCommandButton(std::string_view label, std::string_view icon = {}, bool enabled = true,
                         std::string_view disabledReason = {});
void renderEmptyState(std::string_view title, std::string_view detail, std::string_view nextAction = {});
void renderDiagnosticRow(EditorSeverity severity, std::string_view code, std::string_view message);

} // namespace urpg::editor::ui
