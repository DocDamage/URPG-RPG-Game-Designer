#pragma once

#include "tools/sprite_pipeline/sprite_pipeline_defs.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace urpg::editor {

class SpriteAnimationPreviewPanel {
public:
    struct AnimationRow {
        std::string id;
        size_t frame_count = 0;
        float frame_duration = 0.0f;
        bool loop = false;
        bool is_selected = false;
    };

    struct RenderSnapshot {
        bool visible = true;
        bool has_data = false;
        bool preview_playing = false;
        std::string atlas_name;
        std::string texture_path;
        std::optional<std::string> selected_animation_id;
        std::vector<AnimationRow> animation_rows;
        std::vector<std::string> active_frame_ids;
        std::string active_frame_id;
        size_t active_frame_index = 0;
        int32_t active_frame_width = 0;
        int32_t active_frame_height = 0;
        float selected_frame_duration = 0.0f;
        bool selected_loop = false;
        float preview_elapsed = 0.0f;
    };

    SpriteAnimationPreviewPanel() = default;

    void update(const urpg::tools::SpriteAtlas& atlas);
    void clear();
    void render();

    bool selectAnimation(std::string_view animation_id);
    bool setSelectedAnimationFrameDuration(float frame_duration);
    bool setSelectedAnimationLoop(bool loop);
    void setPreviewPlaying(bool preview_playing);
    void advancePreview(float delta_time);

    const RenderSnapshot& getRenderSnapshot() const { return snapshot_; }

    bool isVisible() const { return visible_; }
    void setVisible(bool visible) { visible_ = visible; }

private:
    using SpriteAtlas = urpg::tools::SpriteAtlas;
    using SpriteAnimationClip = urpg::tools::SpriteAnimationClip;
    using SpriteRect = urpg::tools::SpriteRect;

    const SpriteAnimationClip* findSelectedAnimation() const;
    SpriteAnimationClip* findSelectedAnimation();
    const SpriteRect* findSprite(std::string_view sprite_id) const;
    void ensureSelectedAnimation();
    void clampFrameIndex();
    void rebuildSnapshot();

    SpriteAtlas atlas_;
    bool has_atlas_ = false;
    bool visible_ = true;
    bool preview_playing_ = false;
    std::string selected_animation_id_;
    size_t active_frame_index_ = 0;
    float preview_elapsed_ = 0.0f;
    RenderSnapshot snapshot_;
};

} // namespace urpg::editor
