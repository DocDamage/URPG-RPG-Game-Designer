#define STB_IMAGE_IMPLEMENTATION
#include "asset_loader.h"
#include <stb_image.h>
#include <iostream>
#include <vector>

namespace urpg {

std::unordered_map<std::string, std::shared_ptr<Texture>> AssetLoader::s_textureCache;
std::unordered_set<std::string> AssetLoader::s_missingTextureCache;

std::shared_ptr<Texture> AssetLoader::loadTexture(const std::string& path) {
    if (s_textureCache.count(path)) {
        return s_textureCache[path];
    }
    if (s_missingTextureCache.count(path)) {
        return nullptr;
    }

    int width, height, channels;
    // MZ/MV assets should be loaded with upside-down flip for OpenGL
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 4); // Force 4 channels (RGBA)

    if (!data) {
        std::cerr << "[URPG][AssetLoader] Failed to load texture: " << path << " (" << stbi_failure_reason() << ")\n";
        s_missingTextureCache.insert(path);
        return nullptr;
    }

    std::vector<uint8_t> pixelData(data, data + (width * height * 4));
    stbi_image_free(data);

    auto texture = std::make_shared<Texture>();
    if (texture->loadFromMemory(pixelData, width, height)) {
        s_missingTextureCache.erase(path);
        s_textureCache[path] = texture;
        return texture;
    }

    s_missingTextureCache.insert(path);
    return nullptr;
}

} // namespace urpg
