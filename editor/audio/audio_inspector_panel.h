#pragma once

#include "audio_inspector_model.h"
#include <memory>

namespace urpg::editor {

/**
 * @brief GUI controller for the Audio Inspector tab.
 */
class AudioInspectorPanel {
public:
    AudioInspectorPanel() : m_model(std::make_shared<AudioInspectorModel>()) {}

    void onRefreshRequested(const urpg::audio::AudioCore& core) {
        m_model->refresh(core);
    }

    std::shared_ptr<AudioInspectorModel> getModel() const { return m_model; }

    bool isVisible() const { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }

private:
    std::shared_ptr<AudioInspectorModel> m_model;
    bool m_visible = false;
};

} // namespace urpg::editor
