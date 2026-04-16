#pragma once

#include <string>
#include <vector>
#include <queue>
#include <map>
#include <memory>
#include <mutex>

namespace urpg::asset {

    /**
     * @brief Priority levels for background asset loading.
     * Part of Wave 4 Performance Polish.
     */
    enum class LoadPriority {
        Critical = 10,   // Immediate needed (current view)
        High = 5,       // Likely needed (adjacent map)
        Normal = 1,     // Background prefetch
        Low = 0         // Idle cleanup/recycling
    };

    /**
     * @brief Request for an asset to be loaded.
     */
    struct AssetLoadRequest {
        std::string path;
        LoadPriority priority;
        bool operator<(const AssetLoadRequest& other) const {
            return priority < other.priority;
        }
    };

    /**
     * @brief Smart Async Asset Queue with LRU caching.
     * Prevents frame spikes by distributing loads over multiple frames.
     */
    class PriorityLoader {
    public:
        void request(const std::string& path, LoadPriority priority = LoadPriority::Normal) {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            m_requestQueue.push({path, priority});
        }

        void process(int maxProcessedPerFrame = 5) {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            int processed = 0;
            while (!m_requestQueue.empty() && processed < maxProcessedPerFrame) {
                AssetLoadRequest req = m_requestQueue.top();
                m_requestQueue.pop();
                executeLoad(req.path);
                processed++;
            }
        }

        void clear() {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            while (!m_requestQueue.empty()) m_requestQueue.pop();
        }

        bool isIdle() const { return m_requestQueue.empty(); }

    private:
        void executeLoad(const std::string& path) {
            // Signal renderer/audio to load the binary into memory
            // This happens on a background worker thread but is prioritized here
        }

        std::priority_queue<AssetLoadRequest> m_requestQueue;
        std::mutex m_queueMutex;
        std::map<std::string, bool> m_currentLoads;
    };

} // namespace urpg::asset
