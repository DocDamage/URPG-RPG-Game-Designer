#include "opengl_renderer.h"
#include <iostream>

namespace urpg {

/**
 * @brief Default Vertex Shader for TIER_BASIC (OpenGL 3.3).
 */
const char* DEFAULT_VERTEX_SHADER = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexHandle;

out vec2 TexCoord;

void main() {
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    TexCoord = aTexHandle;
}
)";

/**
 * @brief Default Fragment Shader for TIER_BASIC.
 */
const char* DEFAULT_FRAGMENT_SHADER = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture1;

void main() {
    FragColor = texture(texture1, TexCoord);
}
)";

bool OpenGLRenderer::initialize(IPlatformSurface* /*surface*/) {
    // In a real implementation, this would use GLEW/GLAD to load functions
    // and create a WGL/GLX/NSGL context based on the PlatformSurface.
    std::cout << "[URPG][OpenGL] Initializing OpenGL 3.3 Backend (TIER_BASIC)\n";
    
    // 1. Load GL Function Pointers
    // 2. Setup Shaders
    // 3. Setup Quad Buffers (Used for Sprite Rendering)
    
    return true; 
}

void OpenGLRenderer::beginFrame() {
    // glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderer::renderBatches(const std::vector<SpriteDrawData>& batches) {
    for (const auto& batch : batches) {
        // 1. Bind Texture
        // glBindTexture(GL_TEXTURE_2D, batch.textureId);
        
        // 2. Clear then update VBO with new vertices
        // glBufferData(GL_ARRAY_BUFFER, batch.vertices.size() * sizeof(SpriteVertex), batch.vertices.data(), GL_DYNAMIC_DRAW);
        
        // 3. Draw triangles
        // glDrawArrays(GL_TRIANGLES, 0, (GLsizei)batch.vertices.size());
    }
}

void OpenGLRenderer::endFrame() {
    // Swap buffers (SwapBuffers(hdc), glXSwapBuffers, etc.)
}

void OpenGLRenderer::shutdown() {
    std::cout << "[URPG][OpenGL] Shutting down renderer\n";
    // Release Shaders, Buffers, Textures
}

void OpenGLRenderer::onResize(int /*width*/, int /*height*/) {
    // glViewport(0, 0, width, height);
}

void OpenGLRenderer::processCommands(const std::vector<std::shared_ptr<RenderCommand>>& commands) {
    for (const auto& cmd : commands) {
        switch (cmd->type) {
            case RenderCmdType::Sprite: {
                auto spriteCmd = std::static_pointer_cast<SpriteCommand>(cmd);
                // 1. Bind Texture
                // 2. Set Uniforms (Model Matrix, Opacity)
                // 3. Draw Quad
                break;
            }
            case RenderCmdType::Tile: {
                // Similar to sprite but often uses a vertex buffer of quads
                break;
            }
            case RenderCmdType::Clear: {
                // glClear(...)
                break;
            }
            case RenderCmdType::Text: {
                auto textCmd = std::static_pointer_cast<TextCommand>(cmd);
                // In TIER_BASIC placeholder, we just log text draw intent
                // In real implementation:
                // 1. Get or Generate Glyph Atlas for fontFace/fontSize
                // 2. Map string into quad stream with UVs from atlas
                // 3. Draw quads with text-specific shader (alpha masking)
                std::cout << "[URPG][Renderer] DrawText: \"" << textCmd->text << "\" at (" << textCmd->x << ", " << textCmd->y << ") color: " << (int)textCmd->r << "," << (int)textCmd->g << "," << (int)textCmd->b << "\n";
                break;
            }
            case RenderCmdType::Rect: {
                auto rectCmd = std::static_pointer_cast<RectCommand>(cmd);
                // In TIER_BASIC placeholder, we just log rect draw intent
                // In real implementation:
                // 1. Build a quad from (x,y) to (x+w, y+h)
                // 2. Draw with a solid-color shader
                std::cout << "[URPG][Renderer] DrawRect: (" << rectCmd->x << ", " << rectCmd->y << ") "
                          << rectCmd->w << "x" << rectCmd->h << " color: "
                          << rectCmd->r << "," << rectCmd->g << "," << rectCmd->b << "," << rectCmd->a << "\n";
                break;
            }
            default:
                break;
        }
    }
}

bool OpenGLRenderer::loadTexture(const std::string& /*id*/, const std::string& /*filePath*/) {
    // 1. Load pixels using stb_image
    // 2. glGenTextures(1, &texHandle);
    // 3. glTexImage2D(...)
    return true;
}

} // namespace urpg
