#pragma once

#include "editor/ui/editor_panel.h"
#include "engine/core/engine_context.h"
#include "engine/core/ui/menu_scene_graph.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace urpg::editor {

class MenuPreviewPanel : public EditorPanel {
public:
  struct PaneSnapshot {
    std::string pane_id;
    std::string pane_label;
    bool pane_active = false;
    std::optional<std::string> selected_command_id;
    std::vector<std::string> command_ids;
    std::vector<std::string> command_labels;
    std::vector<bool> command_enabled;
  };

  struct RenderSnapshot {
    std::string active_scene_id;
    std::vector<PaneSnapshot> visible_panes;
    std::string last_blocked_command_id;
    std::string last_blocked_reason;
    bool has_data = false;
  };

  MenuPreviewPanel();

  void bindRuntime(urpg::ui::MenuSceneGraph& scene_graph);
  void clearRuntime();

  void Render(const urpg::FrameContext &context) override;
  void refresh();
  void update();
  bool hasRenderedFrame() const { return has_rendered_frame_; }
  const RenderSnapshot& lastRenderSnapshot() const { return last_render_snapshot_; }

private:
  void captureRenderSnapshot();

  urpg::ui::MenuSceneGraph* scene_graph_ = nullptr;
  bool has_rendered_frame_ = false;
  RenderSnapshot last_render_snapshot_;
};

} // namespace urpg::editor
