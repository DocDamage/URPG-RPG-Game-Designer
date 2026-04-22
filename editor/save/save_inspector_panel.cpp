#include "editor/save/save_inspector_panel.h"

namespace urpg::editor {

void SaveInspectorPanel::bindRuntime(const urpg::SaveCatalog& catalog,
                                     urpg::SaveSessionCoordinator& coordinator) {
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

bool SaveInspectorPanel::showProblemSlotsOnly() const {
    return show_problem_slots_only_;
}

bool SaveInspectorPanel::includeAutosave() const {
    return include_autosave_;
}

bool SaveInspectorPanel::setPolicyAutosaveEnabled(bool autosave_enabled) {
    return model_.SetPolicyAutosaveEnabled(autosave_enabled);
}

bool SaveInspectorPanel::setPolicyAutosaveSlotId(int32_t autosave_slot_id) {
    return model_.SetPolicyAutosaveSlotId(autosave_slot_id);
}

bool SaveInspectorPanel::setPolicyRetentionLimits(size_t max_autosave_slots,
                                                  size_t max_quicksave_slots,
                                                  size_t max_manual_slots,
                                                  bool prune_excess_on_save) {
    return model_.SetPolicyRetentionLimits(
        max_autosave_slots, max_quicksave_slots, max_manual_slots, prune_excess_on_save);
}

bool SaveInspectorPanel::applyPolicyToRuntime() {
    if (!coordinator_) {
        return false;
    }
    const bool applied = model_.ApplyPolicyToRuntime(*coordinator_);
    if (applied) {
        refresh();
    }
    return applied;
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
