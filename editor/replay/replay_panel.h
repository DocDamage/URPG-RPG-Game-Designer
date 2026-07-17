#pragma once

#include "engine/core/replay/replay_gallery.h"
#include "engine/core/playtest/playtest_scenario_replay_bridge.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace urpg::editor {

struct ReplayPanelSnapshot {
    std::size_t artifact_count = 0;
    std::vector<std::string> artifact_ids;
    bool live_connected = false;
    bool capturing = false;
    uint64_t live_revision = 0;
    std::size_t semantic_input_count = 0;
    std::size_t checkpoint_count = 0;
    std::string project_revision;
    std::string runtime_version;
    uint64_t last_control_id = 0;
    replay::ReplayExecutionResult last_result;
};

class ReplayPanel {
public:
    replay::ReplayGallery& gallery() { return gallery_; }
    const replay::ReplayGallery& gallery() const { return gallery_; }

    ReplayPanelSnapshot snapshot() const;
    void render();
    void bindLiveSession(const std::filesystem::path& session_directory);
    bool refreshLive(std::string* diagnostic = nullptr);
    bool requestLiveReplay(replay::ReplayExecutionMode mode, std::string* diagnostic = nullptr);
    const ReplayPanelSnapshot& lastRenderSnapshot() const { return last_render_snapshot_; }
    bool hasRenderedFrame() const { return has_rendered_frame_; }

private:
    replay::ReplayGallery gallery_;
    std::unique_ptr<playtest::PlaytestScenarioReplayBridge> live_bridge_;
    std::optional<playtest::LiveScenarioReplaySnapshot> live_snapshot_;
    uint64_t next_control_id_ = 1;
    ReplayPanelSnapshot last_render_snapshot_{};
    bool has_rendered_frame_ = false;
};

} // namespace urpg::editor
