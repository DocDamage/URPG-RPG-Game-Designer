#pragma once

namespace urpg {

/**
 * @brief Context passed to systems during a frame update/render.
 * Contains delta time and other per-frame metadata.
 */
struct FrameContext {
    float dt;
    uint32_t frame_index = 0;
};

} // namespace urpg
