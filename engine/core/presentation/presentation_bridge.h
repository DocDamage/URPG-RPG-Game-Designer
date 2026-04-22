#pragma once

#include "engine/core/presentation/presentation_runtime.h"
#include "engine/core/presentation/presentation_schema.h"
#include "engine/core/presentation/presentation_context.h"
#include "engine/core/presentation/scene_adapters.h"
#include "engine/core/scene/battle_scene.h"
#include "engine/core/scene/scene_manager.h"
#include <charconv>
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
                                                     const PresentationContext& context);

    /**
     * @brief Access the underlying runtime.
     */
    std::shared_ptr<PresentationRuntime> GetRuntime() const { return m_runtime; }

private:
    struct BattleCueCursor {
        uint32_t nextSequenceIndex = 0;
        size_t observedCueCount = 0;
    };

    void resetBattleCueCursor();

    static uint32_t parseParticipantId(const std::string& id, uint32_t fallback) {
        uint32_t parsed = fallback;
        const auto* begin = id.data();
        const auto* end = begin + id.size();
        const auto result = std::from_chars(begin, end, parsed);
        if (result.ec != std::errc{} || result.ptr != end) {
            return fallback;
        }
        return parsed;
    }

    static std::uint64_t parseParticipantCueId(const std::string& id, bool isEnemy) {
        std::uint64_t parsed = 0;
        const auto* begin = id.data();
        const auto* end = begin + id.size();
        const auto result = std::from_chars(begin, end, parsed);
        if (result.ec != std::errc{} || result.ptr != end || parsed == 0) {
            return 0;
        }

        constexpr std::uint64_t kEnemyCueDomainBit = 1ull << 63;
        return isEnemy ? (parsed | kEnemyCueDomainBit) : parsed;
    }

    BattleSceneState BuildBattleSceneState(const scene::BattleScene& battleScene);

    std::shared_ptr<PresentationRuntime> m_runtime;
    std::shared_ptr<PresentationAuthoringData> m_authoringData;
    const scene::BattleScene* m_activeBattleScene = nullptr;
    BattleCueCursor m_battleCueCursor;
};

} // namespace urpg::presentation
