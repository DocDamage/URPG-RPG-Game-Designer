#include "editor/accessibility/accessibility_battle_adapter.h"

namespace urpg::editor {

using namespace urpg::accessibility;

std::vector<UiElementSnapshot> AccessibilityBattleAdapter::ingest(
    const BattleInspectorModel& model) {
    std::vector<UiElementSnapshot> elements;

    const auto& rows = model.VisibleRows();
    elements.reserve(rows.size());

    for (size_t i = 0; i < rows.size(); ++i) {
        const auto& row = rows[i];
        UiElementSnapshot el;
        el.id = "battle.action." + std::to_string(row.action_order);
        // Use the command string as the accessible label for the action row.
        // An empty command string surfaces a MissingLabel issue.
        el.label = row.command;
        // Rows with a valid subject are navigable/focusable in the battle inspector.
        el.hasFocus = !row.subject_id.empty();
        // Use priority for focus order; fall back to sequential index.
        el.focusOrder = row.priority > 0 ? row.priority : static_cast<int32_t>(i + 1);
        el.contrastRatio = 0.0f;
        el.sourceContext = "editor/battle/battle_inspector_model.h";
        elements.push_back(std::move(el));
    }

    return elements;
}

} // namespace urpg::editor
