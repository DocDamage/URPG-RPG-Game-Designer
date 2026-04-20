#include "editor/ability/pattern_field_model.h"

#include <sstream>

namespace urpg::editor {

PatternFieldModel::PatternFieldModel() {
    m_currentPattern = std::make_shared<PatternField>("New Pattern");
}

void PatternFieldModel::setCurrentPattern(std::shared_ptr<PatternField> pattern) {
    m_currentPattern = pattern ? pattern : std::make_shared<PatternField>("New Pattern");
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

void PatternFieldModel::applyPreset(const std::string& presetId) {
    if (!m_currentPattern) {
        m_currentPattern = std::make_shared<PatternField>("New Pattern");
    }

    if (presetId == "cross_small") {
        *m_currentPattern = PatternField::MakeCross("Cross Small", 1);
    } else if (presetId == "line_long_horizontal") {
        *m_currentPattern = PatternField::MakeLine("Line Long Horizontal", 3, true);
    } else if (presetId == "line_long_vertical") {
        *m_currentPattern = PatternField::MakeLine("Line Long Vertical", 3, false);
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

PatternFieldModel::PreviewSnapshot PatternFieldModel::buildPreviewSnapshot() const {
    PreviewSnapshot snapshot;
    if (!m_currentPattern) {
        snapshot.name = "No Pattern";
        snapshot.is_valid = false;
        snapshot.issues.push_back("No pattern is currently loaded.");
        return snapshot;
    }

    snapshot.name = m_currentPattern->getName();
    const auto validation = PatternValidator::Validate(*m_currentPattern);
    snapshot.is_valid = validation.isValid;
    snapshot.issues = validation.issues;

    const auto bounds = getViewportBounds();
    for (int32_t y = bounds.minY; y <= bounds.maxY; ++y) {
        std::ostringstream row;
        for (int32_t x = bounds.minX; x <= bounds.maxX; ++x) {
            const bool isActive = isPointSelected(x, y);
            if (x == 0 && y == 0) {
                row << (isActive ? "[O]" : "[.]");
            } else {
                row << (isActive ? "[X]" : "[ ]");
            }
        }
        snapshot.grid_rows.push_back(row.str());
    }

    return snapshot;
}

} // namespace urpg::editor
