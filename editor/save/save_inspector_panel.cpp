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
    captureRenderSnapshot();
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
    captureRenderSnapshot();
}

void SaveInspectorPanel::refresh() {
    if (!catalog_ || !coordinator_) {
        captureRenderSnapshot();
        return;
    }

    model_.LoadFromCatalog(*catalog_, *coordinator_);
    model_.SetShowProblemSlotsOnly(show_problem_slots_only_);
    model_.SetIncludeAutosave(include_autosave_);
    captureRenderSnapshot();
}

void SaveInspectorPanel::update() {
    refresh();
}

const SaveInspectorPanel::RenderSnapshot& SaveInspectorPanel::lastRenderSnapshot() const {
    return last_render_snapshot_;
}

void SaveInspectorPanel::captureRenderSnapshot() {
    last_render_snapshot_ = {};
    last_render_snapshot_.visible = visible_;
    last_render_snapshot_.runtime_bound = catalog_ != nullptr && coordinator_ != nullptr;
    last_render_snapshot_.can_refresh = last_render_snapshot_.runtime_bound;
    last_render_snapshot_.visible_row_count = model_.VisibleRows().size();
    last_render_snapshot_.issue_count = model_.PolicyValidation().issue_count;
    last_render_snapshot_.can_apply_policy =
        last_render_snapshot_.runtime_bound && model_.PolicyValidation().can_apply;

    if (!last_render_snapshot_.runtime_bound) {
        last_render_snapshot_.status = "disabled";
        last_render_snapshot_.message = "No save runtime is bound.";
        last_render_snapshot_.remediation =
            "Bind SaveCatalog and SaveSessionCoordinator before rendering save state.";
        last_render_snapshot_.can_apply_policy = false;
        return;
    }

    if (model_.PolicyValidation().error_count > 0) {
        last_render_snapshot_.status = "error";
        last_render_snapshot_.message = "Save policy draft has validation errors.";
        last_render_snapshot_.remediation = "Fix save policy validation errors before applying runtime changes.";
        return;
    }

    if (model_.VisibleRows().empty()) {
        last_render_snapshot_.status = "empty";
        last_render_snapshot_.message = "No save slots are visible.";
        last_render_snapshot_.remediation = show_problem_slots_only_
                                               ? "Disable problem-slot filtering or create a save slot."
                                               : "Create a save slot before using save inspection tools.";
        return;
    }

    last_render_snapshot_.status = "ready";
    last_render_snapshot_.message = "Save inspection data is available.";
    last_render_snapshot_.remediation = "";
}

} // namespace urpg::editor
