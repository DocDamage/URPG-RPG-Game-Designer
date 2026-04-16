#include "editor/ability/pattern_field_model.h"

namespace urpg::editor {

PatternFieldModel::PatternFieldModel() {
    m_currentPattern = std::make_shared<PatternField>("New Pattern");
}

void PatternFieldModel::setCurrentPattern(std::shared_ptr<PatternField> pattern) {
    m_currentPattern = pattern;
}

void PatternFieldModel::togglePoint(int32_t x, int32_t y) {
    if (!m_currentPattern) return;
    if (m_currentPattern->hasPoint(x, y)) {
        m_currentPattern->removePoint(x, y);
    } else {
        m_currentPattern->addPoint(x, y);
    }
}

void PatternFieldModel::clearPattern() {
    if (!m_currentPattern) return;
    auto points = m_currentPattern->getPoints();
    // Copy points to avoid modifying collection while iterating
    auto pointsCopy = points;
    for (const auto& p : pointsCopy) {
        m_currentPattern->removePoint(p.x, p.y);
    }
}

void PatternFieldModel::resizeViewport(int32_t newSize) {
    if (newSize < 1) newSize = 1;
    if (newSize % 2 == 0) newSize++; // Force odd size for centered origin
    m_viewportSize = newSize;
}

void PatternFieldModel::setName(const std::string& name) {
    if (m_currentPattern) {
        m_currentPattern->setName(name);
    }
}

PatternFieldModel::GridBounds PatternFieldModel::getViewportBounds() const {
    int32_t half = m_viewportSize / 2;
    return { -half, -half, half, half };
}

bool PatternFieldModel::isPointSelected(int32_t x, int32_t y) const {
    if (!m_currentPattern) return false;
    return m_currentPattern->hasPoint(x, y);
}

} // namespace urpg::editor
