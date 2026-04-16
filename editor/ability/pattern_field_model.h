#pragma once

#include "engine/core/ability/pattern_field.h"
#include <vector>
#include <memory>

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

    PatternFieldModel();

    void setCurrentPattern(std::shared_ptr<PatternField> pattern);
    std::shared_ptr<PatternField> getCurrentPattern() const { return m_currentPattern; }

    void togglePoint(int32_t x, int32_t y);
    void clearPattern();

    GridBounds getViewportBounds() const;
    void setViewportSize(int32_t size); // e.g. 5x5, 7x7

    bool isPointSelected(int32_t x, int32_t y) const;

private:
    std::shared_ptr<PatternField> m_currentPattern;
    int32_t m_viewportSize = 5;
};

} // namespace urpg::editor
