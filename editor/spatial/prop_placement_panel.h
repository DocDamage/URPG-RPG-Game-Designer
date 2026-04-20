#pragma once

#include "editor/ui/editor_panel.h"
#include "engine/core/presentation/presentation_schema.h"
#include "engine/core/presentation/spatial_projection.h"
#include "engine/core/engine_context.h"
#include <cmath>
#include <string>
#include <vector>

namespace urpg::editor {

/**
 * @brief Editor tool for 3D Prop Placement.
 * Exposes SpatialMapOverlay logic to non-technical authors.
 */
class PropPlacementPanel : public EditorPanel {
public:
    struct ScreenProjectionSettings {
        float viewportWidth = 0.0f;
        float viewportHeight = 0.0f;
        float cameraCenterX = 0.0f;
        float cameraCenterZ = 0.0f;
        float worldUnitsPerPixel = 1.0f;
        bool rejectOutOfBounds = true;
    };

    struct RenderSnapshot {
        bool visible = true;
        bool has_target = false;
        std::string selected_asset_id;
        size_t prop_count = 0;
        std::optional<std::string> last_added_asset_id;
        std::optional<float> last_added_x;
        std::optional<float> last_added_y;
        std::optional<float> last_added_z;
    };

    PropPlacementPanel() : EditorPanel("Prop Placement") {}

    void Render(const urpg::FrameContext& context) override;

    void AddProp(const std::string& assetId, float x, float y, float z);

    static bool TryProjectScreenToGround(
        const urpg::presentation::SpatialMapOverlay& overlay,
        float screenX,
        float screenY,
        const ScreenProjectionSettings& settings,
        float& outWorldX,
        float& outWorldY,
        float& outWorldZ) {
        if (settings.viewportWidth <= 0.0f || settings.viewportHeight <= 0.0f ||
            settings.worldUnitsPerPixel <= 0.0f) {
            return false;
        }

        const float halfWidth = settings.viewportWidth * 0.5f;
        const float halfHeight = settings.viewportHeight * 0.5f;
        const float projectedWorldX = settings.cameraCenterX + (screenX - halfWidth) * settings.worldUnitsPerPixel;
        const float projectedWorldZ = settings.cameraCenterZ + (screenY - halfHeight) * settings.worldUnitsPerPixel;

        if (overlay.elevation.width == 0 || overlay.elevation.height == 0) {
            return false;
        }

        const float maxWorldX = static_cast<float>(overlay.elevation.width);
        const float maxWorldZ = static_cast<float>(overlay.elevation.height);
        if (settings.rejectOutOfBounds &&
            (projectedWorldX < 0.0f || projectedWorldZ < 0.0f ||
             projectedWorldX >= maxWorldX || projectedWorldZ >= maxWorldZ)) {
            return false;
        }

        const float clampedWorldX = std::clamp(projectedWorldX, 0.0f, maxWorldX - 0.001f);
        const float clampedWorldZ = std::clamp(projectedWorldZ, 0.0f, maxWorldZ - 0.001f);
        const uint32_t tileX = static_cast<uint32_t>(std::floor(clampedWorldX));
        const uint32_t tileZ = static_cast<uint32_t>(std::floor(clampedWorldZ));

        outWorldX = clampedWorldX;
        outWorldY = overlay.elevation.GetWorldHeight(tileX, tileZ);
        outWorldZ = clampedWorldZ;
        return true;
    }

    static bool TryProjectScreenToGround(
        const urpg::presentation::SpatialMapOverlay& overlay,
        const urpg::Vector2f& screenPoint,
        const urpg::presentation::ViewportRect& viewport,
        const urpg::presentation::CameraState& cameraState,
        const urpg::presentation::CameraProfile& cameraProfile,
        float& outWorldX,
        float& outWorldY,
        float& outWorldZ,
        bool rejectOutOfBounds = true) {
        const urpg::presentation::WorldRay ray = urpg::presentation::SpatialProjection::ScreenPointToWorldRay(
            screenPoint, viewport, cameraState, cameraProfile);
        const auto hit = urpg::presentation::SpatialProjection::IntersectElevationGrid(ray, overlay.elevation);
        if (!hit.has_value()) {
            return false;
        }

        if (rejectOutOfBounds) {
            const float maxWorldX = static_cast<float>(overlay.elevation.width);
            const float maxWorldZ = static_cast<float>(overlay.elevation.height);
            if (hit->x < 0.0f || hit->z < 0.0f || hit->x >= maxWorldX || hit->z >= maxWorldZ) {
                return false;
            }
        }

        outWorldX = hit->x;
        outWorldY = hit->y;
        outWorldZ = hit->z;
        return true;
    }

    bool AddPropFromScreen(
        const std::string& assetId,
        float screenX,
        float screenY,
        const ScreenProjectionSettings& settings) {
        if (!m_targetOverlay) {
            return false;
        }

        float worldX = 0.0f;
        float worldY = 0.0f;
        float worldZ = 0.0f;
        if (!TryProjectScreenToGround(*m_targetOverlay, screenX, screenY, settings, worldX, worldY, worldZ)) {
            return false;
        }

        AddProp(assetId, worldX, worldY, worldZ);
        return true;
    }

    bool AddPropFromScreen(
        const std::string& assetId,
        const urpg::Vector2f& screenPoint,
        const urpg::presentation::ViewportRect& viewport,
        const urpg::presentation::CameraState& cameraState,
        const urpg::presentation::CameraProfile& cameraProfile,
        bool rejectOutOfBounds = true) {
        if (!m_targetOverlay) {
            return false;
        }

        float worldX = 0.0f;
        float worldY = 0.0f;
        float worldZ = 0.0f;
        if (!TryProjectScreenToGround(
                *m_targetOverlay,
                screenPoint,
                viewport,
                cameraState,
                cameraProfile,
                worldX,
                worldY,
                worldZ,
                rejectOutOfBounds)) {
            return false;
        }

        AddProp(assetId, worldX, worldY, worldZ);
        return true;
    }

    void SetTarget(urpg::presentation::SpatialMapOverlay* overlay);
    void SetSelectedAssetId(const std::string& asset_id);

    const RenderSnapshot& lastRenderSnapshot() const { return last_render_snapshot_; }

private:
    void captureRenderSnapshot();

    urpg::presentation::SpatialMapOverlay* m_targetOverlay = nullptr;
    std::string m_selectedAssetId = "tree_01";
    RenderSnapshot last_render_snapshot_;
};

} // namespace urpg::editor
