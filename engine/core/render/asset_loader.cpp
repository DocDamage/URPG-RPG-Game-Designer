#define STB_IMAGE_IMPLEMENTATION
#include "asset_loader.h"
#include <stb_image.h>
#include <iostream>
#include <vector>
#include <utility>

namespace urpg {

std::mutex AssetLoader::s_cacheMutex;
std::thread::id AssetLoader::s_cacheOwnerThread;
bool AssetLoader::s_hasCacheOwnerThread = false;
std::unordered_map<std::string, AssetLoader::TextureCacheEntry> AssetLoader::s_textureCache;
std::list<std::string> AssetLoader::s_textureCacheUsage;
std::unordered_map<std::string, AssetLoader::MissingTextureCacheEntry> AssetLoader::s_missingTextureCache;
std::list<std::string> AssetLoader::s_missingTextureCacheUsage;

bool AssetLoader::ensureCacheOwnerThreadLocked(const char* operation) {
    const std::thread::id currentThread = std::this_thread::get_id();
    if (!s_hasCacheOwnerThread) {
        s_cacheOwnerThread = currentThread;
        s_hasCacheOwnerThread = true;
        return true;
    }

    if (s_cacheOwnerThread == currentThread) {
        return true;
    }

    std::cerr << "[URPG][AssetLoader] Rejected " << operation
              << " from a non-owner thread. AssetLoader caches are single-thread-affine; "
                 "call clearCaches() on the new owning thread before reusing them.\n";
    return false;
}

void AssetLoader::clearCaches() {
    std::lock_guard<std::mutex> lock(s_cacheMutex);
    s_textureCache.clear();
    s_textureCacheUsage.clear();
    s_missingTextureCache.clear();
    s_missingTextureCacheUsage.clear();
    s_cacheOwnerThread = std::this_thread::get_id();
    s_hasCacheOwnerThread = true;
}

void AssetLoader::touchTextureCacheEntry(const std::string& path) {
    auto entryIt = s_textureCache.find(path);
    if (entryIt == s_textureCache.end()) {
        return;
    }

    s_textureCacheUsage.splice(s_textureCacheUsage.begin(), s_textureCacheUsage, entryIt->second.usageIt);
    entryIt->second.usageIt = s_textureCacheUsage.begin();
}

void AssetLoader::touchMissingTextureCacheEntry(const std::string& path) {
    auto entryIt = s_missingTextureCache.find(path);
    if (entryIt == s_missingTextureCache.end()) {
        return;
    }

    s_missingTextureCacheUsage.splice(s_missingTextureCacheUsage.begin(), s_missingTextureCacheUsage, entryIt->second.usageIt);
    entryIt->second.usageIt = s_missingTextureCacheUsage.begin();
}

void AssetLoader::cacheTexture(const std::string& path, const std::shared_ptr<Texture>& texture) {
    auto existingIt = s_textureCache.find(path);
    if (existingIt != s_textureCache.end()) {
        existingIt->second.texture = texture;
        touchTextureCacheEntry(path);
        return;
    }

    s_textureCacheUsage.push_front(path);
    s_textureCache.emplace(path, TextureCacheEntry{texture, s_textureCacheUsage.begin()});

    if (s_textureCache.size() <= kTextureCacheLimit) {
        return;
    }

    const std::string& evictedPath = s_textureCacheUsage.back();
    s_textureCache.erase(evictedPath);
    s_textureCacheUsage.pop_back();
}

void AssetLoader::cacheMissingTexture(const std::string& path) {
    auto existingIt = s_missingTextureCache.find(path);
    if (existingIt != s_missingTextureCache.end()) {
        touchMissingTextureCacheEntry(path);
        return;
    }

    s_missingTextureCacheUsage.push_front(path);
    s_missingTextureCache.emplace(path, MissingTextureCacheEntry{s_missingTextureCacheUsage.begin()});

    if (s_missingTextureCache.size() <= kMissingTextureCacheLimit) {
        return;
    }

    const std::string& evictedPath = s_missingTextureCacheUsage.back();
    s_missingTextureCache.erase(evictedPath);
    s_missingTextureCacheUsage.pop_back();
}

std::shared_ptr<Texture> AssetLoader::loadTexture(const std::string& path) {
    std::lock_guard<std::mutex> lock(s_cacheMutex);
    if (!ensureCacheOwnerThreadLocked("loadTexture")) {
        return nullptr;
    }

    if (auto cachedIt = s_textureCache.find(path); cachedIt != s_textureCache.end()) {
        touchTextureCacheEntry(path);
        return cachedIt->second.texture;
    }
    if (s_missingTextureCache.find(path) != s_missingTextureCache.end()) {
        touchMissingTextureCacheEntry(path);
        return nullptr;
    }

    int width, height, channels;
    // MZ/MV assets should be loaded with upside-down flip for OpenGL
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 4); // Force 4 channels (RGBA)

    if (!data) {
        std::cerr << "[URPG][AssetLoader] Failed to load texture: " << path << " (" << stbi_failure_reason() << ")\n";
        cacheMissingTexture(path);
        return nullptr;
    }

    std::vector<uint8_t> pixelData(data, data + (width * height * 4));
    stbi_image_free(data);

    auto texture = std::make_shared<Texture>();
    if (texture->loadFromMemory(pixelData, width, height)) {
        if (auto missingIt = s_missingTextureCache.find(path); missingIt != s_missingTextureCache.end()) {
            s_missingTextureCacheUsage.erase(missingIt->second.usageIt);
            s_missingTextureCache.erase(missingIt);
        }
        cacheTexture(path, texture);
        return texture;
    }

    cacheMissingTexture(path);
    return nullptr;
}

} // namespace urpg
