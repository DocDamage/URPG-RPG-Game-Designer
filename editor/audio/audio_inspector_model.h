#pragma once

#include "../../engine/core/audio/audio_core.h"
#include <algorithm>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

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
        if (selected_handle_.has_value()) {
            const auto selected_it = std::find_if(m_rows.begin(), m_rows.end(), [&](const AudioHandleRow& row) {
                return row.handle == *selected_handle_;
            });
            if (selected_it == m_rows.end()) {
                selected_handle_.reset();
            }
        }

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
        selected_handle_.reset();
    }

    std::vector<AudioHandleRow> getRows() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_rows;
    }

    std::vector<std::string> getIssues() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_issues;
    }

    bool selectNextRow() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_rows.empty()) {
            return false;
        }

        if (!selected_handle_.has_value()) {
            selected_handle_ = m_rows.front().handle;
            return true;
        }

        const auto current_index = selectedRowIndexLocked();
        if (!current_index.has_value() || *current_index + 1 >= m_rows.size()) {
            return false;
        }

        selected_handle_ = m_rows[*current_index + 1].handle;
        return true;
    }

    bool selectPreviousRow() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_rows.empty()) {
            return false;
        }

        if (!selected_handle_.has_value()) {
            return false;
        }

        const auto current_index = selectedRowIndexLocked();
        if (!current_index.has_value() || *current_index == 0) {
            return false;
        }

        selected_handle_ = m_rows[*current_index - 1].handle;
        return true;
    }

    bool canSelectNextRow() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_rows.empty()) {
            return false;
        }

        if (!selected_handle_.has_value()) {
            return true;
        }

        const auto current_index = selectedRowIndexLocked();
        return current_index.has_value() && *current_index + 1 < m_rows.size();
    }

    bool canSelectPreviousRow() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_rows.empty() || !selected_handle_.has_value()) {
            return false;
        }

        const auto current_index = selectedRowIndexLocked();
        return current_index.has_value() && *current_index > 0;
    }

    std::optional<urpg::audio::AudioHandle> selectedHandle() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return selected_handle_;
    }

    std::optional<AudioHandleRow> selectedRow() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        const auto current_index = selectedRowIndexLocked();
        if (!current_index.has_value()) {
            return std::nullopt;
        }
        return m_rows[*current_index];
    }

    struct Summary {
        size_t activeCount;
        size_t issueCount;
        float masterVolume;
    };

    Summary getSummary() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return {m_projected_active_count, m_issues.size(), m_master_volume};
    }

  private:
    std::optional<size_t> selectedRowIndexLocked() const {
        if (!selected_handle_.has_value()) {
            return std::nullopt;
        }

        for (size_t index = 0; index < m_rows.size(); ++index) {
            if (m_rows[index].handle == *selected_handle_) {
                return index;
            }
        }

        return std::nullopt;
    }

    std::vector<AudioHandleRow> m_rows;
    std::vector<std::string> m_issues;
    size_t m_projected_active_count = 0;
    float m_master_volume = 1.0f;
    std::optional<urpg::audio::AudioHandle> selected_handle_;
    mutable std::mutex m_mutex;
};

} // namespace urpg::editor
