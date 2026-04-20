#include "sprite_animator.h"

namespace urpg {

SpriteAnimator::SpriteAnimator(const std::shared_ptr<Texture>& texture)
    : SpriteAnimator(texture, AnimationConfig{}) {
}

SpriteAnimator::SpriteAnimator(const std::shared_ptr<Texture>& texture, const AnimationConfig& config)
    : m_texture(texture), m_config(config) {
}

SpriteAnimator::SpriteAnimator(const std::shared_ptr<Texture>& texture,
                               const AtlasDefinition& atlas,
                               const std::string& animationId)
    : m_texture(texture), m_atlas(atlas), m_usesAtlas(true), m_currentAnimationId(animationId) {
    if (!setAnimation(animationId)) {
        m_usesAtlas = false;
    }
}

void SpriteAnimator::update(float deltaTime) {
    if (m_usesAtlas) {
        const AtlasAnimation* animation = currentAnimation();
        if (animation == nullptr || animation->frameIds.empty()) {
            return;
        }

        if (!m_isWalking) {
            m_currentAtlasFrameIndex = 0;
            m_elapsedTime = 0.0f;
            return;
        }

        m_elapsedTime += deltaTime;
        if (m_elapsedTime >= animation->frameDuration) {
            m_elapsedTime = 0.0f;
            if (animation->loop) {
                m_currentAtlasFrameIndex = (m_currentAtlasFrameIndex + 1) % static_cast<int>(animation->frameIds.size());
            } else if (m_currentAtlasFrameIndex + 1 < static_cast<int>(animation->frameIds.size())) {
                ++m_currentAtlasFrameIndex;
            }
        }
        return;
    }

    if (!m_texture) return;

    if (!m_isWalking) {
        m_currentFrame = 1; // Middle frame (standard RM idle)
        m_elapsedTime = 0.0f;
        return;
    }

    m_elapsedTime += deltaTime;
    if (m_elapsedTime >= m_config.frameDuration) {
        m_elapsedTime = 0.0f;
        m_currentFrame = (m_currentFrame + 1) % m_config.framesX;
    }
}

void SpriteAnimator::setRow(int row) {
    if (row < m_config.framesY) {
        m_currentRow = row;
    }
}

void SpriteAnimator::reset() {
    m_currentFrame = 0;
    m_currentAtlasFrameIndex = 0;
    m_elapsedTime = 0.0f;
}

bool SpriteAnimator::setAnimation(const std::string& animationId) {
    const AtlasAnimation* animation = findAnimation(animationId);
    if (animation == nullptr) {
        return false;
    }

    m_currentAnimationId = animationId;
    m_currentAtlasFrameIndex = 0;
    m_elapsedTime = 0.0f;
    return true;
}

SpriteAnimator::FrameView SpriteAnimator::getCurrentFrameView() const {
    FrameView view;

    if (m_usesAtlas) {
        const AtlasAnimation* animation = currentAnimation();
        if (animation == nullptr || animation->frameIds.empty()) {
            return view;
        }

        const int safeIndex = m_currentAtlasFrameIndex < static_cast<int>(animation->frameIds.size())
            ? m_currentAtlasFrameIndex
            : 0;
        const AtlasFrame* frame = findFrame(animation->frameIds[static_cast<size_t>(safeIndex)]);
        if (frame == nullptr || m_atlas.width <= 0 || m_atlas.height <= 0) {
            return view;
        }

        view.usingAtlas = true;
        view.frameId = frame->id;
        view.pixelWidth = frame->width;
        view.pixelHeight = frame->height;
        view.u1 = static_cast<float>(frame->x) / static_cast<float>(m_atlas.width);
        view.v1 = static_cast<float>(frame->y) / static_cast<float>(m_atlas.height);
        view.u2 = static_cast<float>(frame->x + frame->width) / static_cast<float>(m_atlas.width);
        view.v2 = static_cast<float>(frame->y + frame->height) / static_cast<float>(m_atlas.height);
        return view;
    }

    if (!m_texture || m_config.framesX <= 0 || m_config.framesY <= 0) {
        return view;
    }

    const float frameWidth = static_cast<float>(m_texture->getWidth()) / static_cast<float>(m_config.framesX);
    const float frameHeight = static_cast<float>(m_texture->getHeight()) / static_cast<float>(m_config.framesY);
    view.pixelWidth = static_cast<int>(frameWidth);
    view.pixelHeight = static_cast<int>(frameHeight);
    view.u1 = (m_currentFrame * frameWidth) / static_cast<float>(m_texture->getWidth());
    view.v1 = (m_currentRow * frameHeight) / static_cast<float>(m_texture->getHeight());
    view.u2 = ((m_currentFrame + 1) * frameWidth) / static_cast<float>(m_texture->getWidth());
    view.v2 = ((m_currentRow + 1) * frameHeight) / static_cast<float>(m_texture->getHeight());
    return view;
}

void SpriteAnimator::draw(SpriteBatcher& batcher, float x, float y, float w, float h, float z) {
    if (!m_texture) return;

    uint32_t texID = m_texture->getId();
    const FrameView view = getCurrentFrameView();

    // Submit the quad to the batcher
    batcher.submit(texID, x, y, w, h, view.u1, view.v1, view.u2, view.v2, z);
}

const SpriteAnimator::AtlasAnimation* SpriteAnimator::findAnimation(const std::string& animationId) const {
    for (const auto& animation : m_atlas.animations) {
        if (animation.id == animationId) {
            return &animation;
        }
    }
    return nullptr;
}

const SpriteAnimator::AtlasAnimation* SpriteAnimator::currentAnimation() const {
    return findAnimation(m_currentAnimationId);
}

const SpriteAnimator::AtlasFrame* SpriteAnimator::findFrame(const std::string& frameId) const {
    for (const auto& frame : m_atlas.frames) {
        if (frame.id == frameId) {
            return &frame;
        }
    }
    return nullptr;
}

} // namespace urpg
