#include "editor/sprite/sprite_animation_preview_panel.h"

#include <algorithm>

namespace urpg::editor {

void SpriteAnimationPreviewPanel::update(const urpg::tools::SpriteAtlas& atlas) {
    atlas_ = atlas;
    has_atlas_ = true;
    ensureSelectedAnimation();
    clampFrameIndex();
    rebuildSnapshot();
}

void SpriteAnimationPreviewPanel::clear() {
    atlas_ = {};
    has_atlas_ = false;
    preview_playing_ = false;
    selected_animation_id_.clear();
    active_frame_index_ = 0;
    preview_elapsed_ = 0.0f;
    snapshot_ = {};
    snapshot_.visible = visible_;
}

void SpriteAnimationPreviewPanel::render() {
    rebuildSnapshot();
}

bool SpriteAnimationPreviewPanel::selectAnimation(std::string_view animation_id) {
    if (!has_atlas_) {
        return false;
    }

    const auto it = std::find_if(
        atlas_.animations.begin(),
        atlas_.animations.end(),
        [&](const SpriteAnimationClip& clip) {
            return clip.id == animation_id;
        }
    );
    if (it == atlas_.animations.end()) {
        return false;
    }

    selected_animation_id_ = it->id;
    active_frame_index_ = 0;
    preview_elapsed_ = 0.0f;
    rebuildSnapshot();
    return true;
}

bool SpriteAnimationPreviewPanel::setSelectedAnimationFrameDuration(float frame_duration) {
    if (frame_duration <= 0.0f) {
        return false;
    }

    auto* clip = findSelectedAnimation();
    if (clip == nullptr) {
        return false;
    }

    clip->frameDuration = frame_duration;
    preview_elapsed_ = 0.0f;
    rebuildSnapshot();
    return true;
}

bool SpriteAnimationPreviewPanel::setSelectedAnimationLoop(bool loop) {
    auto* clip = findSelectedAnimation();
    if (clip == nullptr) {
        return false;
    }

    clip->loop = loop;
    clampFrameIndex();
    rebuildSnapshot();
    return true;
}

void SpriteAnimationPreviewPanel::setPreviewPlaying(bool preview_playing) {
    preview_playing_ = preview_playing;
    rebuildSnapshot();
}

void SpriteAnimationPreviewPanel::advancePreview(float delta_time) {
    if (!preview_playing_ || delta_time <= 0.0f) {
        rebuildSnapshot();
        return;
    }

    const auto* clip = findSelectedAnimation();
    if (clip == nullptr || clip->frames.empty() || clip->frameDuration <= 0.0f) {
        rebuildSnapshot();
        return;
    }

    preview_elapsed_ += delta_time;
    while (preview_elapsed_ >= clip->frameDuration) {
        preview_elapsed_ -= clip->frameDuration;
        if (active_frame_index_ + 1 < clip->frames.size()) {
            ++active_frame_index_;
            continue;
        }

        if (clip->loop) {
            active_frame_index_ = 0;
        } else {
            active_frame_index_ = clip->frames.size() - 1;
            preview_elapsed_ = 0.0f;
            preview_playing_ = false;
            break;
        }
    }

    rebuildSnapshot();
}

const SpriteAnimationPreviewPanel::SpriteAnimationClip*
SpriteAnimationPreviewPanel::findSelectedAnimation() const {
    if (!has_atlas_ || selected_animation_id_.empty()) {
        return nullptr;
    }

    const auto it = std::find_if(
        atlas_.animations.begin(),
        atlas_.animations.end(),
        [&](const SpriteAnimationClip& clip) {
            return clip.id == selected_animation_id_;
        }
    );
    return it == atlas_.animations.end() ? nullptr : &(*it);
}

SpriteAnimationPreviewPanel::SpriteAnimationClip*
SpriteAnimationPreviewPanel::findSelectedAnimation() {
    if (!has_atlas_ || selected_animation_id_.empty()) {
        return nullptr;
    }

    const auto it = std::find_if(
        atlas_.animations.begin(),
        atlas_.animations.end(),
        [&](const SpriteAnimationClip& clip) {
            return clip.id == selected_animation_id_;
        }
    );
    return it == atlas_.animations.end() ? nullptr : &(*it);
}

const SpriteAnimationPreviewPanel::SpriteRect*
SpriteAnimationPreviewPanel::findSprite(std::string_view sprite_id) const {
    const auto it = std::find_if(
        atlas_.sprites.begin(),
        atlas_.sprites.end(),
        [&](const SpriteRect& sprite) {
            return sprite.id == sprite_id;
        }
    );
    return it == atlas_.sprites.end() ? nullptr : &(*it);
}

void SpriteAnimationPreviewPanel::ensureSelectedAnimation() {
    if (!has_atlas_) {
        selected_animation_id_.clear();
        return;
    }

    const auto has_selected = std::any_of(
        atlas_.animations.begin(),
        atlas_.animations.end(),
        [&](const SpriteAnimationClip& clip) {
            return clip.id == selected_animation_id_;
        }
    );
    if (has_selected) {
        return;
    }

    if (!atlas_.preview.defaultAnimation.empty()) {
        const auto it = std::find_if(
            atlas_.animations.begin(),
            atlas_.animations.end(),
            [&](const SpriteAnimationClip& clip) {
                return clip.id == atlas_.preview.defaultAnimation;
            }
        );
        if (it != atlas_.animations.end()) {
            selected_animation_id_ = it->id;
            active_frame_index_ = 0;
            preview_elapsed_ = 0.0f;
            return;
        }
    }

    if (!atlas_.animations.empty()) {
        selected_animation_id_ = atlas_.animations.front().id;
        active_frame_index_ = 0;
        preview_elapsed_ = 0.0f;
        return;
    }

    selected_animation_id_.clear();
}

void SpriteAnimationPreviewPanel::clampFrameIndex() {
    const auto* clip = findSelectedAnimation();
    if (clip == nullptr || clip->frames.empty()) {
        active_frame_index_ = 0;
        return;
    }

    if (active_frame_index_ >= clip->frames.size()) {
        active_frame_index_ = clip->frames.size() - 1;
    }
}

void SpriteAnimationPreviewPanel::rebuildSnapshot() {
    snapshot_ = {};
    snapshot_.visible = visible_;
    snapshot_.preview_playing = preview_playing_;
    if (!has_atlas_) {
        return;
    }

    ensureSelectedAnimation();
    clampFrameIndex();

    snapshot_.has_data = true;
    snapshot_.atlas_name = atlas_.atlasName;
    snapshot_.texture_path = atlas_.texturePath;
    if (!selected_animation_id_.empty()) {
        snapshot_.selected_animation_id = selected_animation_id_;
    }

    snapshot_.animation_rows.reserve(atlas_.animations.size());
    for (const auto& clip : atlas_.animations) {
        snapshot_.animation_rows.push_back(AnimationRow{
            clip.id,
            clip.frames.size(),
            clip.frameDuration,
            clip.loop,
            clip.id == selected_animation_id_
        });
    }

    const auto* clip = findSelectedAnimation();
    if (clip == nullptr) {
        return;
    }

    snapshot_.active_frame_ids = clip->frames;
    snapshot_.selected_frame_duration = clip->frameDuration;
    snapshot_.selected_loop = clip->loop;
    snapshot_.preview_elapsed = preview_elapsed_;
    if (clip->frames.empty()) {
        return;
    }

    snapshot_.active_frame_index = active_frame_index_;
    snapshot_.active_frame_id = clip->frames[active_frame_index_];

    const auto* sprite = findSprite(snapshot_.active_frame_id);
    if (sprite != nullptr) {
        snapshot_.active_frame_width = sprite->width;
        snapshot_.active_frame_height = sprite->height;
    }
}

} // namespace urpg::editor
