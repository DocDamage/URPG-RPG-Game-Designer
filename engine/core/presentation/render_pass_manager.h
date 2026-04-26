#pragma once

#include "presentation_types.h"
#include <string>
#include <vector>

namespace urpg::presentation {

/**
 * @brief Identifies the functional purpose of a render pass.
 * ADR-007: Strict separation of visual layers.
 */
enum class RenderPassType {
    Background,    // Static environment/Skybox
    WorldSpatial,  // Map, Actors, Props (Depth Sorted)
    WorldOverlay,  // Combat damage numbers, local world-space icons
    UserInterface, // Global HUD, Menus, Dialogue
    Debug          // Collision bounds, wireframes, stats
};

/**
 * @brief A discrete execution unit of the presentation layer.
 * ADR-007: Defines the sort and depth policies for a frame segment.
 */
struct RenderPass {
    std::string passName;
    RenderPassType type;

    // Policy identifiers
    bool useDepthTesting = true;
    bool useAlphaBlending = false;
    int32_t priority = 0; // Execution order (lower = earlier)

    // Range of commands in the PresentationFrameIntent buffer (Section 9.5)
    size_t commandStartIndex = 0;
    size_t commandCount = 0;
};

/**
 * @brief Orchestrates the execution logic for visual intent.
 * Section 11.2: Render Execution Pipeline
 */
class RenderPassManager {
  public:
    RenderPassManager() = default;

    /**
     * @brief Resets the pass sequence for a new frame.
     */
    void Reset() { m_passes.clear(); }

    /**
     * @brief Adds a new pass to the current frame.
     */
    void AddPass(const RenderPass& pass) { m_passes.push_back(pass); }

    /**
     * @brief Retrieves the current pass sequence.
     */
    const std::vector<RenderPass>& GetPasses() const { return m_passes; }

  private:
    std::vector<RenderPass> m_passes;
};

} // namespace urpg::presentation
