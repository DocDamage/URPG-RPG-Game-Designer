#pragma once

#include "engine/core/assets/texture_registry.h"
#include "engine/core/platform/gl_texture.h"
#include "engine/core/platform/renderer_backend.h"
#include "engine/core/render/render_layer.h"
#include "engine/core/runtime_asset_mode.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace urpg {

/**
 * @brief Represents a GPU-side texture handle.
 */
struct GLTexture {
    uint32_t handle = 0;
    int32_t width = 0;
    int32_t height = 0;
    std::shared_ptr<Texture> owner;
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
    bool registerTextureHandle(const std::string& id, const std::shared_ptr<Texture>& texture);
    void setRuntimeAssetMode(RuntimeAssetMode mode) { m_runtimeAssetMode = mode; }
    RuntimeAssetMode runtimeAssetMode() const { return m_runtimeAssetMode; }

  private:
    void setupDefaultShaders();
    void setupDrawBuffers();
    void processFrameCommand(const FrameRenderCommand& command);
    void processCommand(const RenderCommand& command);
    void submitImmediateBatch(const std::vector<float>& vertices) const;
    void submitTexturedBatch(const SpriteDrawData& batch) const;
    void drawSpriteCommand(const SpriteCommand& command);
    void drawTileCommand(const TileCommand& command);
    void drawTextCommand(const TextCommand& command);
    void drawRectCommand(const RectCommand& command);
    std::shared_ptr<GLTexture> resolveTextureHandle(const std::string& id);
    void emitUnresolvedTextureDiagnostic(const char* code, const std::string& role, const std::string& id) const;

    // Internal state
    IPlatformSurface* m_surface = nullptr;
    uint32_t m_shaderProgram = 0;
    uint32_t m_vao = 0;
    uint32_t m_vbo = 0;
    uint32_t m_texturedShaderProgram = 0;
    uint32_t m_texturedVao = 0;
    uint32_t m_texturedVbo = 0;
    int m_viewportWidth = 640;
    int m_viewportHeight = 480;
    bool m_immediatePipelineReady = false;
    bool m_texturedPipelineReady = false;
    RuntimeAssetMode m_runtimeAssetMode = RuntimeAssetMode::Development;

    std::map<std::string, std::shared_ptr<GLTexture>> m_textures;
};

} // namespace urpg
