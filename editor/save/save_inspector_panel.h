#pragma once

#include "editor/save/save_inspector_model.h"

namespace urpg::editor {

class SaveInspectorPanel {
public:
    SaveInspectorPanel() = default;

    void bindRuntime(const urpg::SaveCatalog& catalog,
                     urpg::SaveSessionCoordinator& coordinator);
    void clearRuntime();

    SaveInspectorModel& getModel();
    const SaveInspectorModel& getModel() const;

    void setVisible(bool visible);
    bool isVisible() const;

    void setShowProblemSlotsOnly(bool show_problem_slots_only);
    void setIncludeAutosave(bool include_autosave);
    bool showProblemSlotsOnly() const;
    bool includeAutosave() const;
    bool setPolicyAutosaveEnabled(bool autosave_enabled);
    bool setPolicyAutosaveSlotId(int32_t autosave_slot_id);
    bool setPolicyRetentionLimits(size_t max_autosave_slots,
                                  size_t max_quicksave_slots,
                                  size_t max_manual_slots,
                                  bool prune_excess_on_save);
    bool applyPolicyToRuntime();

    void render();
    void refresh();
    void update();

private:
    const urpg::SaveCatalog* catalog_ = nullptr;
    urpg::SaveSessionCoordinator* coordinator_ = nullptr;
    SaveInspectorModel model_;
    bool visible_ = true;
    bool show_problem_slots_only_ = false;
    bool include_autosave_ = true;
};

} // namespace urpg::editor
