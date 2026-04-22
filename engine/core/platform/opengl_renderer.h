#pragma once

#include "engine/core/platform/renderer_backend.h"
#include "engine/core/render/render_layer.h"
#include "engine/core/assets/texture_registry.h"
#include <string>
#include <memory>
#include <vector>
#include <map>

namespace urpg {

/**
 * @brief Represents a GPU-side texture handle.
 */
struct GLTexture {
    uint32_t handle = 0;
    int32_t width = 0;
    int32_t height = 0;
};

/**
 * @brief OpenGL implementation of the RendererBackend.
 * Focuses on TIER_BASIC (OpenGL 3.3) for maximum compatibility.
 */
class OpenGLRenderer : public RendererBackend {
public:
    OpenGLRenderer() = default;
    virtual ~OpenGLRenderer() { shutdown(); }

    RenderTier getTier() const override { return RenderTier::Basic; }

    bool initialize(IPlatformSurface* surface) override;
    void beginFrame() override;
    void renderBatches(const std::vector<SpriteDrawData>& batches) override;
    void endFrame() override;
    void processFrameCommands(const std::vector<FrameRenderCommand>& commands) override;
    void processCommands(const std::vector<std::shared_ptr<RenderCommand>>& commands) override;
    void shutdown() override;
    void onResize(int width, int height) override;

    /**
     * @brief Loads a texture file into GPU memory and returns a registry reference.
     * In a real implementation, this would use an image loading library like stb_image.
     */
    bool loadTexture(const std::string& id, const std::string& filePath);

private:
    void setupDefaultShaders();
    void setupDrawBuffers();
    void processFrameCommand(const FrameRenderCommand& command);
    void processCommand(const RenderCommand& command);
    void submitImmediateBatch(const std::vector<float>& vertices) const;
    void drawSpriteCommand(const SpriteCommand& command);
    void drawTileCommand(const TileCommand& command);
    void drawTextCommand(const TextCommand& command);
    void drawRectCommand(const RectCommand& command);
    
    // Internal state
    IPlatformSurface* m_surface = nullptr;
    void* m_context = nullptr; // Platform-specific GL context (HGLRC, GLXContext, etc.)
    uint32_t m_shaderProgram = 0;
    uint32_t m_vao = 0;
    uint32_t m_vbo = 0;
    uint32_t m_ebo = 0;
    int m_viewportWidth = 640;
    int m_viewportHeight = 480;
    bool m_immediatePipelineReady = false;

    std::map<std::string, std::shared_ptr<GLTexture>> m_textures;
};

} // namespace urpg
