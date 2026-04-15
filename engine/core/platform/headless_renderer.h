#pragma once

#include "renderer_backend.h"
#include <iostream>

namespace urpg {

/**
 * @brief Soft renderer mock for CI/Headless environments.
 * Just logs or ignores render calls, but maintains the lifecycle states.
 */
class HeadlessRenderer : public RendererBackend {
public:
    RenderTier getTier() const override {
        return RenderTier::Basic;
    }

    bool initialize(IPlatformSurface* /*surface*/) override {
        // No-op for headless environments.
        return true;
    }

    void beginFrame() override {
        // No buffers to clear in headless mode.
    }

    void endFrame() override {
        // No buffers to swap in headless mode.
    }

    void renderBatches(const std::vector<SpriteDrawData>& /*batches*/) override {
        // No-op for headless mode.
    }

    void shutdown() override {
        // No resources to clean up.
    }

    void onResize(int /*width*/, int /*height*/) override {
        // Viewport resize is irrelevant here.
    }
};

} // namespace urpg
