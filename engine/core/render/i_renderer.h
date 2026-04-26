#pragma once

#include <string>
#include <vector>

namespace urpg {

/**
 * @brief Abstract interface for the platform-specific renderer.
 * Allows the ECS systems to stay decoupled from SDL/OpenGL/Vulkan/etc.
 */
class IRenderer {
  public:
    virtual ~IRenderer() = default;

    virtual void clear() = 0;
    virtual void present() = 0;

    virtual void drawSprite(const std::string& textureId, int srcX, int srcY, int srcW, int srcH, float destX,
                            float destY, float destW, float destH, bool flipX = false) = 0;

    virtual void drawRect(float x, float y, float w, float h, unsigned char r, unsigned char g, unsigned char b,
                          unsigned char a) = 0;

    virtual void setCamera(float x, float y, float zoom) = 0;
};

} // namespace urpg
