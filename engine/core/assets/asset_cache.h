#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <list>

namespace urpg {

/**
 * @brief Base class for any asset managed by the AssetCache.
 */
class Asset {
public:
    virtual ~Asset() = default;
    
    const std::string& getId() const { return m_id; }
    void setId(const std::string& id) { m_id = id; }

protected:
    std::string m_id;
};

/**
 * @brief Thread-safe template-based asset cache with LRU eviction logic.
 */
template<typename T>
class AssetCache {
public:
    static_assert(std::is_base_of<Asset, T>::value, "T must inherit from urpg::Asset");

    explicit AssetCache(size_t capacity = 100) : m_capacity(capacity) {}

    /**
     * @brief Stores an asset in the cache. Evicts least recently used if capacity is reached.
     */
    void store(std::shared_ptr<T> asset) {
        if (!asset || asset->getId().empty()) return;
        std::lock_guard<std::mutex> lock(m_mutex);
        
        const std::string& id = asset->getId();
        auto it = m_cacheMap.find(id);

        if (it != m_cacheMap.end()) {
            // Update existing: move to front of LRU list
            m_lruList.erase(it->second);
        } else {
            // Add new: check capacity
            if (m_lruList.size() >= m_capacity) {
                // Evict tail (least recently used)
                std::string oldestId = m_lruList.back()->getId();
                m_cacheMap.erase(oldestId);
                m_lruList.pop_back();
            }
        }

        m_lruList.push_front(asset);
        m_cacheMap[id] = m_lruList.begin();
    }

    /**
     * @brief Retrieves an asset by ID and updates its position in the LRU list.
     */
    std::shared_ptr<T> get(const std::string& id) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_cacheMap.find(id);
        if (it != m_cacheMap.end()) {
            // Move accessed item to front
            auto asset = *it->second;
            m_lruList.erase(it->second);
            m_lruList.push_front(asset);
            m_cacheMap[id] = m_lruList.begin();
            return asset;
        }
        return nullptr;
    }

    /**
     * @brief Checks if an asset exists in the cache without updating LRU position.
     */
    bool has(const std::string& id) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_cacheMap.find(id) != m_cacheMap.end();
    }

    /**
     * @brief Removes an asset from the cache.
     */
    void remove(const std::string& id) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_cacheMap.find(id);
        if (it != m_cacheMap.end()) {
            m_lruList.erase(it->second);
            m_cacheMap.erase(it);
        }
    }

    /**
     * @brief Clears all assets from the cache.
     */
    void clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_cacheMap.clear();
        m_lruList.clear();
    }

    /**
     * @brief Returns the number of assets currently cached.
     */
    size_t size() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_cacheMap.size();
    }

    void setCapacity(size_t capacity) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_capacity = capacity;
        // Optional: Trigger immediate eviction if current size > new capacity
        while (m_lruList.size() > m_capacity) {
            std::string oldestId = m_lruList.back()->getId();
            m_cacheMap.erase(oldestId);
            m_lruList.pop_back();
        }
    }

private:
    size_t m_capacity;
    std::list<std::shared_ptr<T>> m_lruList;
    std::unordered_map<std::string, typename std::list<std::shared_ptr<T>>::iterator> m_cacheMap;
    mutable std::mutex m_mutex;
};

} // namespace urpg
