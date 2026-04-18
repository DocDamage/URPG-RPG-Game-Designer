#pragma once

#include "editor/ui/editor_panel.h"
#include "editor/ui/menu_inspector_model.h"
#include "engine/core/engine_context.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace urpg::editor {

class MenuInspectorPanel : public EditorPanel {
public:
  struct RenderSnapshot {
    MenuInspectorSummary summary;
    std::vector<MenuInspectorRow> visible_rows;
    std::vector<MenuInspectorIssue> issues;
    std::optional<std::string> selected_command_id;
    std::optional<MenuInspectorRow> selected_row;
    std::string command_id_filter;
    bool show_issues_only = false;
    bool has_data = false;
  };

  explicit MenuInspectorPanel(std::shared_ptr<MenuInspectorModel> model);

  void Render(const urpg::FrameContext &context) override;
  void refresh();
  void update();

  MenuInspectorModel& getModel() { return *model_; }
  const MenuInspectorModel& getModel() const { return *model_; }
  bool hasRenderedFrame() const { return has_rendered_frame_; }
  const RenderSnapshot& lastRenderSnapshot() const { return last_render_snapshot_; }

private:
  void CaptureRenderSnapshot();
  void RenderCommandRegistry();
  void RenderSceneGraphState();
  void RenderSelectedCommandDetails();

  std::shared_ptr<MenuInspectorModel> model_;
  bool has_rendered_frame_ = false;
  RenderSnapshot last_render_snapshot_;
};

} // namespace urpg::editor
