#include "editor/accessibility/accessibility_menu_adapter.h"

namespace urpg::editor {

using namespace urpg::accessibility;

std::vector<UiElementSnapshot> AccessibilityMenuAdapter::ingest(const MenuInspectorModel& model) {
    std::vector<UiElementSnapshot> elements;
    const auto& rows = model.VisibleRows();
    elements.reserve(rows.size());

    for (size_t i = 0; i < rows.size(); ++i) {
        const auto& row = rows[i];
        UiElementSnapshot snapshot;
        snapshot.id = row.command_id;
        snapshot.label = row.command_label;
        snapshot.hasFocus = row.row_navigable && row.command_visible && row.command_enabled;
        snapshot.focusOrder = row.priority > 0 ? row.priority : static_cast<int32_t>(i + 1);
        snapshot.contrastRatio = 0.0f; // Menu model does not carry contrast data
        elements.push_back(std::move(snapshot));
    }

    return elements;
}

} // namespace urpg::editor
