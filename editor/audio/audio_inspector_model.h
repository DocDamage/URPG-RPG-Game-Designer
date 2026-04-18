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
        m_rows.clear();
        
        // Project active sources from AudioCore
        // In a real implementation we would iterate the internal source map
        // For now, we simulate projection based on the reported core state
        
        // Issue tracking
        m_issues.clear();
        
        // Example validation: volume > 1.0 (unlikely but possible if gain staging is weird)
        // Or "asset missing" if we had asset registry access
    }

    void clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_rows.clear();
        m_issues.clear();
    }

    const std::vector<AudioHandleRow>& getRows() const { return m_rows; }
    const std::vector<std::string>& getIssues() const { return m_issues; }

    struct Summary {
        size_t activeCount;
        size_t issueCount;
        float masterVolume;
    };

    Summary getSummary() const {
        return { m_rows.size(), m_issues.size(), 1.0f };
    }

private:
    std::vector<AudioHandleRow> m_rows;
    std::vector<std::string> m_issues;
    mutable std::mutex m_mutex;
};

} // namespace urpg::editor
