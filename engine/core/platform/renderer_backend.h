#pragma once

#include "platform_surface.h"
#include "engine/core/render/render_tier.h"
#include "engine/core/sprite_batcher.h"
#include "engine/core/render/render_layer.h"
#include <vector>
#include <string>
#include <memory>

namespace urpg {

/**
 * @brief Abstract base class for the low-level rendering backend.
 * This decouples the EngineShell from specific APIs like OpenGL or DirectX.
 */
class RendererBackend {
public:
    virtual ~RendererBackend() = default;

    /**
     * @brief Get the rendering tier supported by this backend/hardware.
     */
    virtual RenderTier getTier() const = 0;

    /**
     * @brief Initialize the renderer with a given surface.
     * @param surface The platform-specific surface (window) to render to.
     */
    virtual bool initialize(IPlatformSurface* surface) = 0;

    /**
     * @brief Begin a new frame. Clears buffers and prepares states.
     */
    virtual void beginFrame() = 0;

    /**
     * @brief Renders the provided list of sprite batches.
     * @param batches The batches to draw (grouped by texture).
     */
    virtual void renderBatches(const std::vector<SpriteDrawData>& batches) = 0;

    /**
     * @brief End the current frame. Swaps buffers.
     */
    virtual void endFrame() = 0;

    /**
     * @brief Processes frame-owned render commands while preserving legacy backend overrides.
     */
    virtual void processFrameCommands(const std::vector<FrameRenderCommand>& commands) {
        processCommands(toLegacyRenderCommands(commands));
    }

    /**
     * @brief Processes a list of generic render commands (sprites, text, etc.).
     */
    virtual void processCommands(const std::vector<std::shared_ptr<RenderCommand>>& commands) = 0;

    /**
     * @brief Shuts down the renderer and releases resources.
     */
    virtual void shutdown() = 0;

    /**
     * @brief Optional: Resize the viewport if the surface changes.
     */
    virtual void onResize(int width, int height) = 0;
};

} // namespace urpg
