#pragma once

#include "editor/ability/pattern_field_model.h"
#include <cstdint>
#include <string>
#include <vector>

namespace urpg::editor {

/**
 * @brief ImGui-backed UI surface for editing PatternField assets.
 */
class PatternFieldPanel {
  public:
    struct RenderSnapshot {
        struct ControlState {
            std::string id;
            std::string label;
            bool enabled = false;
            std::string disabled_reason;
        };

        bool visible = true;
        bool has_rendered_frame = false;
        std::string name;
        bool is_valid = true;
        std::vector<std::string> issues;
        std::vector<std::string> grid_rows;
        PatternFieldModel::GridBounds viewport_bounds{};
        int32_t viewport_size = 0;
        size_t active_point_count = 0;
        std::vector<ControlState> controls;
    };

    PatternFieldPanel() = default;

    void bindModel(PatternFieldModel& model);
    void clearBinding();
    void update(const PatternFieldModel& model);
    void render();
    const RenderSnapshot& getRenderSnapshot() const { return m_snapshot; }

    bool setPatternName(const std::string& name);
    bool togglePoint(int32_t x, int32_t y);
    bool clearPattern();
    bool resizeViewport(int32_t viewport_size);
    bool applyPreset(const std::string& preset_id);

    bool isVisible() const { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }

  private:
    PatternFieldModel& model();
    const PatternFieldModel& model() const;
    void rebuildSnapshot();
    void rebuildControlState();

    PatternFieldModel m_model;
    PatternFieldModel* m_bound_model = nullptr;
    RenderSnapshot m_snapshot;
    bool m_visible = true;
};

} // namespace urpg::editor
