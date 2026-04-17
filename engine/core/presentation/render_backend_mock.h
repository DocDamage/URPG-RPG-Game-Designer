#pragma once

#include "presentation_runtime.h"
#include <vector>
#include <string>
#include <iostream>

namespace urpg::render {

/**
 * @brief Mock Render Backend following ADR-007.
 * Consumes PresentationFrameIntent and tracks draw calls and pipeline states.
 */
class RenderBackendMock {
public:
    struct DrawCall {
        uint32_t id;
        presentation::Vec3 position;
        std::string type;
    };

    struct PipelineState {
        bool fogEnabled = false;
        bool postFxEnabled = false;
        float bloomIntensity = 0.0f;
    };

    /**
     * @brief Consumes the intent buffer and simulates a render frame.
     */
    void ConsumeFrame(const presentation::PresentationFrameIntent& intent) {
        m_drawCalls.clear();
        m_state = PipelineState{};

        for (const auto& cmd : intent.commands) {
            switch (cmd.type) {
                case presentation::PresentationCommand::Type::DrawActor:
                    m_drawCalls.push_back({cmd.id, cmd.position, "Actor"});
                    break;
                case presentation::PresentationCommand::Type::DrawProp:
                    m_drawCalls.push_back({cmd.id, cmd.position, "Prop"});
                    break;
                case presentation::PresentationCommand::Type::SetFog:
                    m_state.fogEnabled = true;
                    if (cmd.fogProfile) {
                        std::cout << "[BACKEND] Applied Fog: Density=" << cmd.fogProfile->density << "\n";
                    }
                    break;
                case presentation::PresentationCommand::Type::SetPostFX:
                    m_state.postFxEnabled = true;
                    if (cmd.postFXProfile) {
                        m_state.bloomIntensity = cmd.postFXProfile->bloomIntensity;
                        std::cout << "[BACKEND] Applied PostFX: Bloom=" << m_state.bloomIntensity << "\n";
                    }
                    break;
                case presentation::PresentationCommand::Type::DrawShadowProxy:
                    m_drawCalls.push_back({cmd.id, cmd.position, "ShadowProxy"});
                    break;
                default:
                    // Other commands (Lights, Camera) would be handled here in a full backend
                    break;
            }
        }
    }

    const std::vector<DrawCall>& GetDrawCalls() const { return m_drawCalls; }
    const PipelineState& GetCurrentState() const { return m_state; }

private:
    std::vector<DrawCall> m_drawCalls;
    PipelineState m_state;
};

} // namespace urpg::render
