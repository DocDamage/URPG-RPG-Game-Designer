#pragma once

#include "asset_cache.h"
#include <string>
#include <vector>

namespace urpg {

/**
 * @brief Represents metadata for a texture or sprite sheet.
 */
struct TextureMeta : public Asset {
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t tileWidth = 0;
    uint32_t tileHeight = 0;
    std::string filePath;
    bool isSmooth = true;
};

/**
 * @brief Registry for texture metadata, allowing the MapScene to reference native textures.
 */
class TextureRegistry {
public:
    static TextureRegistry& getInstance() {
        static TextureRegistry instance;
        return instance;
    }

    /**
     * @brief Registers texture metadata into the global cache.
     */
    void registerTexture(const std::string& id, const TextureMeta& meta) {
        auto texture = std::make_shared<TextureMeta>(meta);
        texture->setId(id);
        m_cache.store(texture);
    }

    /**
     * @brief Retrieves texture metadata by ID.
     */
    std::shared_ptr<TextureMeta> getTexture(const std::string& id) {
        return m_cache.get(id);
    }

    /**
     * @brief Clears the registry.
     */
    void clear() {
        m_cache.clear();
    }

    size_t getCount() const { return m_cache.size(); }

private:
    TextureRegistry() = default;
    AssetCache<TextureMeta> m_cache;
};

} // namespace urpg
