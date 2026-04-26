#pragma once

#include "editor/save/save_inspector_model.h"

namespace urpg::editor {

class SaveInspectorPanel {
public:
    struct RenderSnapshot {
        std::string status = "disabled";
        std::string message = "No save runtime is bound.";
        std::string remediation = "Bind SaveCatalog and SaveSessionCoordinator before rendering save state.";
        bool runtime_bound = false;
        bool visible = true;
        bool can_refresh = false;
        bool can_apply_policy = false;
        size_t visible_row_count = 0;
        size_t issue_count = 0;
    };

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
    const RenderSnapshot& lastRenderSnapshot() const;

private:
    void captureRenderSnapshot();

    const urpg::SaveCatalog* catalog_ = nullptr;
    urpg::SaveSessionCoordinator* coordinator_ = nullptr;
    SaveInspectorModel model_;
    RenderSnapshot last_render_snapshot_;
    bool visible_ = true;
    bool show_problem_slots_only_ = false;
    bool include_autosave_ = true;
};

} // namespace urpg::editor
