#pragma once

#include <string>
#include <list>
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

    /**
     * @brief Clear both positive and negative texture caches.
     * Intended for deterministic tests, editor/runtime resets, and any future
     * asset-domain teardown path that must discard process-global cache state.
     */
    static void clearCaches();

private:
    struct TextureCacheEntry {
        std::shared_ptr<Texture> texture;
        std::list<std::string>::iterator usageIt;
    };

    struct MissingTextureCacheEntry {
        std::list<std::string>::iterator usageIt;
    };

    static constexpr std::size_t kTextureCacheLimit = 256;
    static constexpr std::size_t kMissingTextureCacheLimit = 256;

    static void touchTextureCacheEntry(const std::string& path);
    static void touchMissingTextureCacheEntry(const std::string& path);
    static void cacheTexture(const std::string& path, const std::shared_ptr<Texture>& texture);
    static void cacheMissingTexture(const std::string& path);

    // Bounded LRU-style caches avoid unbounded growth while preserving reuse/suppression semantics.
    // Entries live until evicted by the LRU policy or explicitly discarded via clearCaches().
    static std::unordered_map<std::string, TextureCacheEntry> s_textureCache;
    static std::list<std::string> s_textureCacheUsage;
    static std::unordered_map<std::string, MissingTextureCacheEntry> s_missingTextureCache;
    static std::list<std::string> s_missingTextureCacheUsage;
};

} // namespace urpg
