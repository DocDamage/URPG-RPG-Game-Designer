#pragma once

#include "editor/ui/editor_panel.h"
#include "engine/core/engine_context.h"
#include "engine/core/ui/menu_scene_graph.h"
#include "engine/core/ui/menu_authoring_document.h"
#include "engine/core/accessibility/inclusive_experience.h"

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace urpg::editor {

class MenuPreviewPanel : public EditorPanel {
public:
  struct PaneSnapshot {
    size_t pane_index = 0;
    std::string pane_id;
    std::string pane_label;
    bool pane_active = false;
    urpg::ui::MenuPaneLayout layout;
    std::optional<std::string> selected_command_id;
    std::vector<std::string> command_ids;
    std::vector<std::string> command_labels;
    std::vector<bool> command_enabled;
    std::vector<urpg::ui::MenuVisualState> command_visual_states;
    std::vector<std::map<std::string, std::string>> command_state_properties;
    std::vector<uint32_t> command_transition_duration_ms;
    std::vector<std::string> command_audio_hooks;
  };

  struct RenderSnapshot {
    std::string active_scene_id;
    urpg::ui::MenuDesignCanvas design_canvas;
    urpg::ui::MenuDesignCanvas preview_canvas;
    bool previewing_target_canvas = false;
    std::vector<PaneSnapshot> visible_panes;
    std::vector<std::string> responsive_diagnostics;
    std::string last_blocked_command_id;
    std::string last_blocked_reason;
    bool has_data = false;
  };

  MenuPreviewPanel();

  using LayoutChangeHandler = std::function<bool(size_t, urpg::ui::MenuPaneLayout)>;
  using LayoutBatchChangeHandler =
      std::function<bool(const std::vector<std::pair<size_t, urpg::ui::MenuPaneLayout>>&)>;

  void bindRuntime(urpg::ui::MenuSceneGraph& scene_graph);
  bool bindAuthoringDocument(const urpg::ui::MenuAuthoringDocument& document, std::string scene_id,
                             std::vector<std::string>* diagnostics = nullptr);
  bool bindAuthoringDocument(const urpg::ui::MenuAuthoringDocument& document, std::string scene_id,
                             const urpg::ui::MenuBindingContext& binding_context,
                             std::vector<std::string>* diagnostics = nullptr);
  void setPreviewAccessibilityPolicy(bool reduced_motion, bool audio_enabled);
  void clearRuntime();
  void setLayoutChangeHandler(LayoutChangeHandler handler);
  void setLayoutBatchChangeHandler(LayoutBatchChangeHandler handler);
  bool setPreviewTargetCanvas(urpg::ui::MenuDesignCanvas canvas);
  void clearPreviewTargetCanvas();

  void Render(const urpg::FrameContext &context) override;
  void refresh();
  void update();
  bool hasRenderedFrame() const { return has_rendered_frame_; }
  const RenderSnapshot& lastRenderSnapshot() const { return last_render_snapshot_; }
  std::vector<urpg::accessibility::InclusiveAuditIssue> auditInclusiveSnapshot(
      bool touch_declared, bool reduced_motion) const;

private:
  enum class DragMode {
    Move,
    Resize,
  };

  struct DragState {
    size_t pane_index = 0;
    DragMode mode = DragMode::Move;
    urpg::ui::MenuPaneLayout initial_layout;
    urpg::ui::MenuPaneLayout preview_layout;
    float start_mouse_x = 0.0f;
    float start_mouse_y = 0.0f;
  };

  void captureRenderSnapshot();
  bool bindAuthoringDocumentInternal(const urpg::ui::MenuAuthoringDocument& document,
                                     std::string scene_id,
                                     const urpg::ui::MenuBindingContext* binding_context,
                                     std::vector<std::string>* diagnostics);

  urpg::ui::MenuSceneGraph* scene_graph_ = nullptr;
  std::unique_ptr<urpg::ui::MenuSceneGraph> authored_preview_graph_;
  std::map<std::string, urpg::ui::MenuCanvasNode> authored_nodes_;
  std::set<std::string> authored_disabled_command_ids_;
  LayoutChangeHandler layout_change_handler_;
  LayoutBatchChangeHandler layout_batch_change_handler_;
  std::optional<DragState> drag_state_;
  std::vector<size_t> selected_pane_indices_;
  std::optional<urpg::ui::MenuDesignCanvas> preview_target_canvas_;
  int snap_grid_size_ = 16;
  bool reduced_motion_preview_ = false;
  bool audio_enabled_preview_ = true;
  bool has_rendered_frame_ = false;
  RenderSnapshot last_render_snapshot_;
};

} // namespace urpg::editor
