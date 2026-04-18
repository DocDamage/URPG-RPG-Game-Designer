#pragma once

#include "editor/ui/editor_panel.h"
#include "engine/core/engine_context.h"
#include "engine/core/ui/menu_scene_graph.h"

#include <memory>

namespace urpg::editor {

class MenuPreviewPanel : public EditorPanel {
public:
  MenuPreviewPanel();

  void bindRuntime(const urpg::ui::MenuSceneGraph& scene_graph);
  void clearRuntime();

  void Render(const urpg::FrameContext &context) override;
  void refresh();
  void update();

private:
  const urpg::ui::MenuSceneGraph* scene_graph_ = nullptr;
};

} // namespace urpg::editor
