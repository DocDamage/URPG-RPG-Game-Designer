#pragma once

#include "editor/ui/editor_panel.h"
#include "editor/ui/menu_inspector_model.h"
#include "engine/core/engine_context.h"

#include <memory>

namespace urpg::editor {

class MenuInspectorPanel : public EditorPanel {
public:
  explicit MenuInspectorPanel(std::shared_ptr<MenuInspectorModel> model);

  void Render(const urpg::FrameContext &context) override;

private:
  void RenderCommandRegistry();
  void RenderSceneGraphState();
  void RenderSelectedCommandDetails();

  std::shared_ptr<MenuInspectorModel> model_;
};

} // namespace urpg::editor