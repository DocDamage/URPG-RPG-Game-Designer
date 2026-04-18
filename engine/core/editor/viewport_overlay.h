#pragma once

#include <vector>
#include <string>
#include "math/vector2.h"

namespace urpg::editor {

    /**
     * @brief Visual primitives for editor viewport gizmos and overlays.
     * Part of Wave 5.3: Viewport Overlay Rendering.
     */
    enum class OverlayPrimitiveType {
        Line,
        Rect,
        Circle,
        Text
    };

    struct OverlayPrimitive {
        OverlayPrimitiveType type;
        float x1, y1, x2, y2; // Coordinates or dimensions
        uint32_t color = 0xFFFFFFFF; // RGBA
        std::string label;
        bool depthTested = false;
    };

    /**
     * @brief Manager for rendering 'non-diegetic' editor visuals (Gizmos, Grid, Selection).
     * Bridges workspace state with the final frame buffer.
     */
    class ViewportOverlayRenderer {
    public:
        void addLine(float x1, float y1, float x2, float y2, uint32_t color) {
            m_primitives.push_back({OverlayPrimitiveType::Line, x1, y1, x2, y2, color});
        }

        void addRect(float x, float y, float w, float h, uint32_t color) {
            m_primitives.push_back({OverlayPrimitiveType::Rect, x, y, w, h, color});
        }

        void addLabel(float x, float y, const std::string& text, uint32_t color = 0xFFFFFFFF) {
            m_primitives.push_back({OverlayPrimitiveType::Text, x, y, 0, 0, color, text});
        }

        void clear() { m_primitives.clear(); }

        /**
         * @brief Emit draw commands to the primary renderer.
         */
        void renderAll() {
            for (const auto& p : m_primitives) {
                // Logic to push to native Renderer vertex buffer
            }
        }

    private:
        std::vector<OverlayPrimitive> m_primitives;
    };

} // namespace urpg::editor
