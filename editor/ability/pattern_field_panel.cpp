#include "editor/ability/pattern_field_panel.h"
#include <iomanip>

namespace urpg::editor {

void PatternFieldPanel::update(const PatternFieldModel& model) {
    m_model = model;
    const auto preview = m_model.buildPreviewSnapshot();
    m_snapshot = {};
    m_snapshot.visible = m_visible;
    m_snapshot.name = preview.name;
    m_snapshot.is_valid = preview.is_valid;
    m_snapshot.issues = preview.issues;
    m_snapshot.grid_rows = preview.grid_rows;
}

void PatternFieldPanel::render() {
    if (!m_visible) return;

    std::cout << "\n--- Pattern Field Editor ---\n";
    if (auto p = m_model.getCurrentPattern()) {
        std::cout << "Name: " << p->getName() << "\n";
    }

    auto bounds = m_model.getViewportBounds();
    std::cout << "Viewport: (" << bounds.minX << "," << bounds.minY << ") to (" << bounds.maxX << "," << bounds.maxY << ")\n";
    if (!m_snapshot.is_valid) {
        for (const auto& issue : m_snapshot.issues) {
            std::cout << "Validation: " << issue << "\n";
        }
    }

    // Legend
    std::cout << "  - [X]: Active point\n";
    std::cout << "  - [ ]: Empty space\n";
    std::cout << "  - (0,0): Origin (Origin is [O] if active, otherwise [.])\n\n";

    // Render Grid
    std::cout << "      ";
    for (int x = bounds.minX; x <= bounds.maxX; ++x) {
        std::cout << std::setw(3) << x;
    }
    std::cout << "\n";

    for (int y = bounds.minY; y <= bounds.maxY; ++y) {
        std::cout << std::setw(4) << y << " | ";
        for (int x = bounds.minX; x <= bounds.maxX; ++x) {
            bool isActive = m_model.isPointSelected(x, y);
            if (x == 0 && y == 0) {
                std::cout << (isActive ? " [O]" : " [.]");
            } else {
                std::cout << (isActive ? " [X]" : " [ ]");
            }
        }
        std::cout << "\n";
    }
    std::cout << "----------------------------\n";
}

} // namespace urpg::editor
