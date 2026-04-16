#pragma once

#include "editor/ui/editor_panel.h"
#include "engine/core/engine_context.h"
#include "engine/core/ui/menu_scene_graph.h"

#include <memory>

namespace urpg::editor {

class MenuPreviewPanel : public EditorPanel {
public:
  explicit MenuPreviewPanel(std::shared_ptr<urpg::ui::MenuSceneGraph> scene_graph);

  void Render(const urpg::FrameContext &context) override;

private:
  std::shared_ptr<urpg::ui::MenuSceneGraph> scene_graph_;
};

} // namespace urpg::editor