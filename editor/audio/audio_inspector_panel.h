#pragma once

#include "audio_inspector_model.h"
#include <memory>

namespace urpg::editor {

/**
 * @brief GUI controller for the Audio Inspector tab.
 */
class AudioInspectorPanel {
public:
    struct RenderSnapshot {
        size_t active_count = 0;
        size_t issue_count = 0;
        float master_volume = 1.0f;
        bool has_data = false;
    };

    AudioInspectorPanel() : m_model(std::make_shared<AudioInspectorModel>()) {}

    void onRefreshRequested(const urpg::audio::AudioCore& core) {
        m_model->refresh(core);
    }

    void clear() {
        m_model->clear();
        m_has_rendered_frame = false;
        m_last_render_snapshot = {};
    }

    std::shared_ptr<AudioInspectorModel> getModel() const { return m_model; }

    bool isVisible() const { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }

    void render() {
        if (!m_visible) {
            return;
        }

        const auto summary = m_model->getSummary();
        m_last_render_snapshot = {
            summary.activeCount,
            summary.issueCount,
            summary.masterVolume,
            summary.activeCount > 0 || summary.issueCount > 0
        };
        m_has_rendered_frame = true;
    }

    void update() {
        render();
    }

    bool hasRenderedFrame() const { return m_has_rendered_frame; }
    const RenderSnapshot& lastRenderSnapshot() const { return m_last_render_snapshot; }

private:
    std::shared_ptr<AudioInspectorModel> m_model;
    bool m_visible = false;
    bool m_has_rendered_frame = false;
    RenderSnapshot m_last_render_snapshot;
};

} // namespace urpg::editor
