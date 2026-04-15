#include "editor/save/save_inspector_panel.h"

namespace urpg::editor {

void SaveInspectorPanel::bindRuntime(const urpg::SaveCatalog& catalog,
                                     const urpg::SaveSessionCoordinator& coordinator) {
    catalog_ = &catalog;
    coordinator_ = &coordinator;
}

void SaveInspectorPanel::clearRuntime() {
    catalog_ = nullptr;
    coordinator_ = nullptr;
    model_ = SaveInspectorModel{};
}

SaveInspectorModel& SaveInspectorPanel::getModel() {
    return model_;
}

const SaveInspectorModel& SaveInspectorPanel::getModel() const {
    return model_;
}

void SaveInspectorPanel::setVisible(bool visible) {
    visible_ = visible;
}

bool SaveInspectorPanel::isVisible() const {
    return visible_;
}

void SaveInspectorPanel::setShowProblemSlotsOnly(bool show_problem_slots_only) {
    show_problem_slots_only_ = show_problem_slots_only;
    model_.SetShowProblemSlotsOnly(show_problem_slots_only_);
}

void SaveInspectorPanel::setIncludeAutosave(bool include_autosave) {
    include_autosave_ = include_autosave;
    model_.SetIncludeAutosave(include_autosave_);
}

void SaveInspectorPanel::render() {
    if (!visible_) {
        return;
    }
}

void SaveInspectorPanel::refresh() {
    if (!catalog_ || !coordinator_) {
        return;
    }

    model_.LoadFromCatalog(*catalog_, *coordinator_);
    model_.SetShowProblemSlotsOnly(show_problem_slots_only_);
    model_.SetIncludeAutosave(include_autosave_);
}

void SaveInspectorPanel::update() {
    refresh();
}

} // namespace urpg::editor