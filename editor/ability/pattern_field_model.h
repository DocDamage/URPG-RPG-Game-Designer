#pragma once

#include "engine/core/ability/pattern_field.h"
#include "engine/core/ability/pattern_field_presets.h"
#include <vector>
#include <memory>
#include <string>
#include <optional>

namespace urpg {
    class PatternField;
}

namespace urpg::editor {

/**
 * @brief View model for the Pattern Field editor.
 * Manages the current pattern being edited and selection state.
 */
class PatternFieldModel {
public:
    struct GridBounds {
        int32_t minX, minY, maxX, maxY;
    };

    struct PreviewSnapshot {
        std::string name;
        bool is_valid = true;
        std::vector<std::string> issues;
        std::vector<std::string> grid_rows;
    };

    struct AvailablePreset {
        std::string id;
        std::string display_name;
        urpg::PatternFieldPresets::Usage usage = urpg::PatternFieldPresets::Usage::Skill;
    };

    PatternFieldModel();

    void setCurrentPattern(std::shared_ptr<PatternField> pattern);
    std::shared_ptr<PatternField> getCurrentPattern() const { return m_currentPattern; }

    void togglePoint(int32_t x, int32_t y);
    void clearPattern();
    void resizeViewport(int32_t newSize);
    void setName(const std::string& name);
    void applyPreset(const std::string& presetId);
    std::vector<AvailablePreset> availablePresets(
        std::optional<urpg::PatternFieldPresets::Usage> usage = std::nullopt) const;

    GridBounds getViewportBounds() const;
    int32_t getViewportSize() const { return m_viewportSize; }

    bool isPointSelected(int32_t x, int32_t y) const;
    PreviewSnapshot buildPreviewSnapshot() const;

private:
    std::shared_ptr<PatternField> m_currentPattern;
    int32_t m_viewportSize = 5;
};

} // namespace urpg::editor
