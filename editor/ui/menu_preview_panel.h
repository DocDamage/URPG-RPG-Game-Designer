#pragma once

#include "editor/ui/editor_panel.h"
#include "engine/core/engine_context.h"
#include "engine/core/ui/menu_scene_graph.h"

#include <functional>
#include <memory>
#include <optional>
#include <string>
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
  };

  struct RenderSnapshot {
    std::string active_scene_id;
    urpg::ui::MenuDesignCanvas design_canvas;
    std::vector<PaneSnapshot> visible_panes;
    std::string last_blocked_command_id;
    std::string last_blocked_reason;
    bool has_data = false;
  };

  MenuPreviewPanel();

  using LayoutChangeHandler = std::function<bool(size_t, urpg::ui::MenuPaneLayout)>;

  void bindRuntime(urpg::ui::MenuSceneGraph& scene_graph);
  void clearRuntime();
  void setLayoutChangeHandler(LayoutChangeHandler handler);

  void Render(const urpg::FrameContext &context) override;
  void refresh();
  void update();
  bool hasRenderedFrame() const { return has_rendered_frame_; }
  const RenderSnapshot& lastRenderSnapshot() const { return last_render_snapshot_; }

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

  urpg::ui::MenuSceneGraph* scene_graph_ = nullptr;
  LayoutChangeHandler layout_change_handler_;
  std::optional<DragState> drag_state_;
  int snap_grid_size_ = 16;
  bool has_rendered_frame_ = false;
  RenderSnapshot last_render_snapshot_;
};

} // namespace urpg::editor
