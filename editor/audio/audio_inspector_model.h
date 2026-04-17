#pragma once

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include "../../engine/core/audio/audio_core.h"

namespace urpg::editor {

/**
 * @brief Diagnostic row representing an active or recent audio handle.
 */
struct AudioHandleRow {
    urpg::audio::AudioHandle handle;
    std::string assetId;
    urpg::audio::AudioCategory category;
    float volume;
    float pitch;
    bool isLooping;
    bool isActive;
};

/**
 * @brief Model for projecting active AudioCore state into the editor UI.
 */
class AudioInspectorModel {
public:
    void refresh(const urpg::audio::AudioCore& audio) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_master_volume = audio.getCategoryVolume(urpg::audio::AudioCategory::System);

        const auto active_sources = audio.activeSources();
        std::vector<AudioHandleRow> projected_rows;
        projected_rows.reserve(active_sources.size());
        for (const auto& source : active_sources) {
            projected_rows.push_back({
                source.handle,
                source.asset_id,
                source.category,
                source.volume,
                source.pitch,
                source.isLooping,
                true,
            });
        }
        m_rows = std::move(projected_rows);
        m_projected_active_count = m_rows.size();
        
        // Issue tracking
        m_issues.clear();
        
        // Example validation: volume > 1.0 (unlikely but possible if gain staging is weird)
        // Or "asset missing" if we had asset registry access
    }

    void clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_rows.clear();
        m_issues.clear();
        m_projected_active_count = 0;
        m_master_volume = 1.0f;
    }

    std::vector<AudioHandleRow> getRows() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_rows;
    }

    std::vector<std::string> getIssues() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_issues;
    }

    struct Summary {
        size_t activeCount;
        size_t issueCount;
        float masterVolume;
    };

    Summary getSummary() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return { m_projected_active_count, m_issues.size(), m_master_volume };
    }

private:
    std::vector<AudioHandleRow> m_rows;
    std::vector<std::string> m_issues;
    size_t m_projected_active_count = 0;
    float m_master_volume = 1.0f;
    mutable std::mutex m_mutex;
};

} // namespace urpg::editor
