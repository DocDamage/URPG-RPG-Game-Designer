#pragma once

#include "editor/ui/editor_panel.h"
#include "engine/core/map/grid_part_catalog.h"
#include "engine/core/map/grid_part_commands.h"
#include "engine/core/map/grid_part_document.h"

#include <string>
#include <unordered_map>

namespace urpg::editor {

class GridPartInspectorPanel : public EditorPanel {
  public:
    struct RenderSnapshot {
        bool visible = true;
        bool has_document = false;
        bool has_catalog = false;
        bool has_selection = false;
        bool can_edit = false;
        std::string selected_instance_id;
        std::string selected_part_id;
        int32_t grid_x = 0;
        int32_t grid_y = 0;
        int32_t width = 1;
        int32_t height = 1;
        bool locked = false;
        bool hidden = false;
        std::unordered_map<std::string, std::string> properties;
        bool can_undo = false;
        bool can_redo = false;
    };

    GridPartInspectorPanel() : EditorPanel("Grid Part Inspector") {}

    void Render(const urpg::FrameContext& context) override;

    void SetTargets(urpg::map::GridPartDocument* document, const urpg::map::GridPartCatalog* catalog);
    bool SelectInstance(const std::string& instance_id);
    bool SetProperty(const std::string& key, const std::string& value);
    bool ClearProperty(const std::string& key);
    bool Undo();
    bool Redo();

    const RenderSnapshot& lastRenderSnapshot() const { return last_render_snapshot_; }

  private:
    void captureRenderSnapshot();

    urpg::map::GridPartDocument* document_ = nullptr;
    const urpg::map::GridPartCatalog* catalog_ = nullptr;
    urpg::map::GridPartCommandHistory history_;
    std::string selected_instance_id_;
    RenderSnapshot last_render_snapshot_;
};

} // namespace urpg::editor
