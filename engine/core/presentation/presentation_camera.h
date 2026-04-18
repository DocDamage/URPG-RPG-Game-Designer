#pragma once

#include "presentation_runtime.h"
#include "presentation_schema.h"
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

namespace urpg::presentation {

/**
 * @brief Dynamic camera state including current target and computed position.
 */
struct CameraState {
    Vec3 targetPos = {0.0f, 0.0f, 0.0f};
    float currentPitch = 45.0f;
    float currentYaw = 0.0f;
    float currentDistance = 10.0f;
    bool isOrtho = false;
};

/**
 * @brief Runtime state and logic for the presentation camera.
 * Section 14.1: Camera Constraints and Framing
 */
class PresentationCamera {
public:
    /**
     * @brief Resolves camera intent based on profile constraints.
     */
    void Update(CameraState& state, const CameraProfile& profile, PresentationFrameIntent& intent) {
        // 1. Enforce constraints
        state.currentPitch = std::clamp(state.currentPitch, profile.minPitch, profile.maxPitch);
        state.currentDistance = std::clamp(state.currentDistance, profile.minDistance, profile.maxDistance);

        // 2. Compute final looked-at point using offset
        Vec3 finalLookAt = {
            state.targetPos.x + profile.lookAtOffset.x,
            state.targetPos.y + profile.lookAtOffset.y,
            state.targetPos.z + profile.lookAtOffset.z
        };

        // 3. Emit intent command
        PresentationCommand camCmd;
        camCmd.type = PresentationCommand::Type::SetCamera;
        camCmd.id = 0xCAF; // Main Camera ID
        camCmd.position = { 
            finalLookAt.x, 
            finalLookAt.y + state.currentDistance, // Mock vertical offset for distance
            finalLookAt.z 
        };
        camCmd.rotation = { state.currentPitch, state.currentYaw, 0.0f };
        
        intent.commands.push_back(camCmd);
        
        std::cout << "[CAMERA] Applied profile '" << profile.profileId << "'. "
                  << "Target Offset: [" << profile.lookAtOffset.x << "," << profile.lookAtOffset.y << "," << profile.lookAtOffset.z << "] "
                  << "Clamped Distance: " << state.currentDistance << "\n";
    }

    const CameraProfile& GetActiveProfile() const { return m_activeProfile; }

private:
    CameraProfile m_activeProfile;
};

} // namespace urpg::presentation
