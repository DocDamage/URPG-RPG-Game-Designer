#pragma once

#include "engine/core/accessibility/accessibility_auditor.h"
#include <nlohmann/json.hpp>

namespace urpg::editor {

/**
 * @brief Editor panel that renders accessibility audit results
 *        and exposes a JSON snapshot of the last render.
 */
class AccessibilityPanel {
public:
    void bindAuditor(urpg::accessibility::AccessibilityAuditor* auditor);
    void render();
    nlohmann::json lastRenderSnapshot() const;

private:
    urpg::accessibility::AccessibilityAuditor* m_auditor = nullptr;
    nlohmann::json m_snapshot;
};

} // namespace urpg::editor
