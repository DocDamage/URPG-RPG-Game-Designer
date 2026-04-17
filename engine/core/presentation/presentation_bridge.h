#pragma once

#include "engine/core/presentation/presentation_runtime.h"
#include "engine/core/presentation/presentation_schema.h"
#include "engine/core/presentation/presentation_context.h"
#include "engine/core/presentation/scene_adapters.h"
#include "engine/core/scene/scene_manager.h"
#include <memory>

namespace urpg::presentation {

/**
 * @brief Integration bridge between the high-level SceneManager and the Presentation Core.
 * This class coordinates the transformation of GameScene state into Render Intent.
 */
class PresentationBridge {
public:
    PresentationBridge(std::shared_ptr<PresentationRuntime> runtime, 
                       std::shared_ptr<PresentationAuthoringData> authoringData)
        : m_runtime(runtime), m_authoringData(authoringData) {}

    /**
     * @brief Generates the visual intent for the currently active scene.
     */
    PresentationFrameIntent BuildFrameForActiveScene(const scene::SceneManager& sceneManager, 
                                                     const PresentationContext& context) {
        auto activeScene = sceneManager.getActiveScene();
        if (!activeScene) {
            return PresentationFrameIntent{PresentationMode::Classic2D, CapabilityTier::Tier0_Baseline};
        }

        // The PresentationRuntime owns the logic to build the frame.
        // In a full implementation, we would route through the appropriate translator
        // based on activeScene->getType().
        return m_runtime->BuildPresentationFrame(context, *m_authoringData);
    }

    /**
     * @brief Access the underlying runtime.
     */
    std::shared_ptr<PresentationRuntime> GetRuntime() const { return m_runtime; }

private:
    std::shared_ptr<PresentationRuntime> m_runtime;
    std::shared_ptr<PresentationAuthoringData> m_authoringData;
};

} // namespace urpg::presentation
