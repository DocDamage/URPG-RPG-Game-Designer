#pragma once

#include "audio_inspector_model.h"
#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace urpg::editor {

/**
 * @brief GUI controller for the Audio Inspector tab.
 */
class AudioInspectorPanel {
  public:
    struct ProjectAssetOption {
        std::string asset_id;
        std::string label;
        std::string project_path;
        std::string picker_kind;
        std::string category;
        std::vector<std::string> picker_targets;
    };

    struct RenderSnapshot {
        size_t active_count = 0;
        size_t issue_count = 0;
        float master_volume = 1.0f;
        std::vector<AudioHandleRow> live_rows;
        std::optional<urpg::audio::AudioHandle> selected_handle;
        std::optional<AudioHandleRow> selected_row;
        bool can_select_next_row = false;
        bool can_select_previous_row = false;
        bool has_data = false;
        bool model_bound = true;
        std::string status_message = "Audio inspector has not rendered yet.";
        std::vector<ProjectAssetOption> project_asset_options;
        std::string selected_project_asset_id;
    };

    AudioInspectorPanel() : m_model(std::make_shared<AudioInspectorModel>()) {}

    void onRefreshRequested(const urpg::audio::AudioCore& core) { m_model->refresh(core); }

    void clear() {
        m_model->clear();
        m_has_rendered_frame = false;
        m_last_render_snapshot = {};
    }

    std::shared_ptr<AudioInspectorModel> getModel() const { return m_model; }

    void setProjectAssetOptions(std::vector<ProjectAssetOption> options) {
        m_project_asset_options.clear();
        for (auto& option : options) {
            const bool targets_audio =
                std::find(option.picker_targets.begin(), option.picker_targets.end(), "audio_selector") !=
                option.picker_targets.end();
            if (targets_audio || option.picker_kind == "audio") {
                m_project_asset_options.push_back(std::move(option));
            }
        }
        if (!m_selected_project_asset_id.empty()) {
            const auto selected = std::find_if(m_project_asset_options.begin(), m_project_asset_options.end(),
                                               [&](const auto& option) {
                                                   return option.asset_id == m_selected_project_asset_id;
                                               });
            if (selected == m_project_asset_options.end()) {
                m_selected_project_asset_id.clear();
            }
        }
    }

    bool selectProjectAsset(std::string asset_id) {
        const auto selected = std::find_if(m_project_asset_options.begin(), m_project_asset_options.end(),
                                           [&](const auto& option) {
                                               return option.asset_id == asset_id;
                                           });
        if (selected == m_project_asset_options.end()) {
            return false;
        }
        m_selected_project_asset_id = std::move(asset_id);
        return true;
    }

    bool isVisible() const { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }

    void render() {
        if (!m_visible) {
            return;
        }

        const auto summary = m_model->getSummary();
        const auto rows = m_model->getRows();
        m_last_render_snapshot.active_count = summary.activeCount;
        m_last_render_snapshot.issue_count = summary.issueCount;
        m_last_render_snapshot.master_volume = summary.masterVolume;
        m_last_render_snapshot.live_rows = rows;
        m_last_render_snapshot.selected_handle = m_model->selectedHandle();
        m_last_render_snapshot.selected_row = m_model->selectedRow();
        m_last_render_snapshot.can_select_next_row = m_model->canSelectNextRow();
        m_last_render_snapshot.can_select_previous_row = m_model->canSelectPreviousRow();
        m_last_render_snapshot.has_data = summary.activeCount > 0 || summary.issueCount > 0;
        m_last_render_snapshot.model_bound = m_model != nullptr;
        m_last_render_snapshot.project_asset_options = m_project_asset_options;
        m_last_render_snapshot.selected_project_asset_id = m_selected_project_asset_id;
        m_last_render_snapshot.status_message =
            m_last_render_snapshot.has_data
                ? ""
                : "No live audio sources or diagnostics are available; refresh from AudioCore.";
        m_has_rendered_frame = true;
    }

    void update() { render(); }

    bool hasRenderedFrame() const { return m_has_rendered_frame; }
    const RenderSnapshot& lastRenderSnapshot() const { return m_last_render_snapshot; }

  private:
    std::shared_ptr<AudioInspectorModel> m_model;
    std::vector<ProjectAssetOption> m_project_asset_options;
    std::string m_selected_project_asset_id;
    bool m_visible = false;
    bool m_has_rendered_frame = false;
    RenderSnapshot m_last_render_snapshot;
};

} // namespace urpg::editor
