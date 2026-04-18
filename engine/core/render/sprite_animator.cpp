#include "sprite_animator.h"

namespace urpg {

SpriteAnimator::SpriteAnimator(const std::shared_ptr<Texture>& texture)
    : SpriteAnimator(texture, AnimationConfig{}) {
}

SpriteAnimator::SpriteAnimator(const std::shared_ptr<Texture>& texture, const AnimationConfig& config)
    : m_texture(texture), m_config(config) {
}

void SpriteAnimator::update(float deltaTime) {
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
    m_elapsedTime = 0.0f;
}

void SpriteAnimator::draw(SpriteBatcher& batcher, float x, float y, float w, float h, float z) {
    if (!m_texture) return;

    uint32_t texID = m_texture->getId();
    int texW = m_texture->getWidth();
    int texH = m_texture->getHeight();

    // Calculate normalized texture coordinates for the current frame
    float fw = (float)texW / m_config.framesX;
    float fh = (float)texH / m_config.framesY;

    float u1 = (m_currentFrame * fw) / texW;
    float v1 = (m_currentRow * fh) / texH;
    float u2 = ((m_currentFrame + 1) * fw) / texW;
    float v2 = ((m_currentRow + 1) * fh) / texH;

    // Submit the quad to the batcher
    batcher.submit(texID, x, y, w, h, u1, v1, u2, v2, z);
}

} // namespace urpg
