#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include "engine/core/platform/gl_texture.h"

namespace urpg {

/**
 * @brief Handles loading resources (Textures, Shaders, etc.) from the file system.
 */
class AssetLoader {
public:
    AssetLoader() = default;
    ~AssetLoader() = default;

    /**
     * @brief Load a texture from a file (PNG, JPG, BMP, etc.).
     * @param path Path to the image file.
     * @return Shared pointer to the Texture, or nullptr if loading fails.
     */
    static std::shared_ptr<Texture> loadTexture(const std::string& path);

private:
    // Simple cache to avoid redundant loads
    static std::unordered_map<std::string, std::shared_ptr<Texture>> s_textureCache;
};

} // namespace urpg
