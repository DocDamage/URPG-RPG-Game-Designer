#pragma once

#include "level_assembly.h"
#include <vector>
#include <random>
#include <algorithm>
#include <optional>
#include <string>

namespace urpg::level {

/**
 * @brief Parameters for dungeon generation.
 */
struct GenParams {
    uint32_t seed = 0;
    int32_t maxBlocks = 10;
};

struct GeneratedBlock {
    std::string blockId;
    int32_t x = 0;
    int32_t y = 0;
    int32_t z = 0;
};

struct GeneratedEncounter {
    std::string encounterId;
    std::string anchorBlockId;
    int32_t x = 0;
    int32_t y = 0;
    int32_t z = 0;
    int32_t difficultyTier = 1;
    std::string role;
};

struct ScenarioBundle {
    uint32_t seed = 0;
    std::string scenarioId;
    std::vector<GeneratedBlock> layout;
    std::vector<GeneratedEncounter> encounters;
};

/**
 * @brief Base toolkit for procedural content generation.
 */
class ProceduralToolkit {
public:
    static std::vector<GeneratedBlock> generateDungeon(const std::vector<LevelBlock>& library, const GenParams& params) {
        std::vector<GeneratedBlock> layout;
        if (library.empty() || params.maxBlocks <= 0) {
            return layout;
        }

        LevelAssemblyWorkspace workspace;
        for (const auto& block : library) {
            workspace.registerBlockDefinition(block);
        }

        std::vector<const LevelBlock*> eligibleSeedBlocks;
        eligibleSeedBlocks.reserve(library.size());
        for (const auto& block : library) {
            if (!block.getConnectors().empty()) {
                eligibleSeedBlocks.push_back(&block);
            }
        }

        if (eligibleSeedBlocks.empty()) {
            return layout;
        }

        std::mt19937 rng(params.seed);
        const LevelBlock* seedBlock = eligibleSeedBlocks[static_cast<size_t>(params.seed) % eligibleSeedBlocks.size()];

        if (!workspace.placeBlock(seedBlock->getId(), 0, 0, 0)) {
            return layout;
        }

        layout.push_back({seedBlock->getId(), 0, 0, 0});

        while (static_cast<int32_t>(layout.size()) < params.maxBlocks) {
            const GeneratedBlock& anchor = layout.back();
            const LevelBlock* anchorDefinition = workspace.findBlockDefinition(anchor.blockId);
            if (anchorDefinition == nullptr) {
                break;
            }

            bool placedNext = false;
            for (const auto& anchorConnector : anchorDefinition->getConnectors()) {
                int32_t dx = 0;
                int32_t dy = 0;
                int32_t dz = 0;
                offsetForSide(anchorConnector.side, dx, dy, dz);

                const int32_t nextX = anchor.x + dx;
                const int32_t nextY = anchor.y + dy;
                const int32_t nextZ = anchor.z + dz;
                if (workspace.hasBlockAt(nextX, nextY, nextZ)) {
                    continue;
                }

                std::vector<const LevelBlock*> candidates;
                for (const auto& candidate : library) {
                    for (const auto& candidateConnector : candidate.getConnectors()) {
                        if (SnapLogic::canSnap(anchorConnector, candidateConnector)) {
                            candidates.push_back(&candidate);
                            break;
                        }
                    }
                }

                if (candidates.empty()) {
                    continue;
                }

                const bool needsFurtherExpansion = static_cast<int32_t>(layout.size()) + 1 < params.maxBlocks;
                size_t bestConnectorCount = 0;
                for (const auto* candidate : candidates) {
                    const size_t connectorCount = candidate->getConnectors().size();
                    if (connectorCount > bestConnectorCount) {
                        bestConnectorCount = connectorCount;
                    }
                }

                std::vector<const LevelBlock*> preferredCandidates;
                preferredCandidates.reserve(candidates.size());
                for (const auto* candidate : candidates) {
                    if (!needsFurtherExpansion || candidate->getConnectors().size() == bestConnectorCount) {
                        preferredCandidates.push_back(candidate);
                    }
                }

                const auto& candidatePool = preferredCandidates.empty() ? candidates : preferredCandidates;
                std::uniform_int_distribution<size_t> candidateDist(0, candidatePool.size() - 1);
                const LevelBlock* nextBlock = candidatePool[candidateDist(rng)];
                if (!workspace.placeBlock(nextBlock->getId(), nextX, nextY, nextZ)) {
                    continue;
                }

                layout.push_back({nextBlock->getId(), nextX, nextY, nextZ});
                placedNext = true;
                break;
            }

            if (!placedNext) {
                break;
            }
        }

        return layout;
    }

    static ScenarioBundle generateScenario(const std::vector<LevelBlock>& library, const GenParams& params) {
        ScenarioBundle bundle;
        bundle.seed = params.seed;
        bundle.scenarioId = "scenario_" + std::to_string(params.seed);
        bundle.layout = generateDungeon(library, params);

        if (bundle.layout.empty()) {
            return bundle;
        }

        bundle.encounters.push_back({
            "encounter_" + std::to_string(params.seed) + "_entry",
            bundle.layout.front().blockId,
            bundle.layout.front().x,
            bundle.layout.front().y,
            bundle.layout.front().z,
            1,
            "entry_guard"
        });

        if (bundle.layout.size() > 1) {
            const GeneratedBlock& goal = bundle.layout.back();
            bundle.encounters.push_back({
                "encounter_" + std::to_string(params.seed) + "_goal",
                goal.blockId,
                goal.x,
                goal.y,
                goal.z,
                2,
                "goal_guard"
            });
        }

        return bundle;
    }

private:
    static void offsetForSide(ConnectorSide side, int32_t& dx, int32_t& dy, int32_t& dz) {
        dx = 0;
        dy = 0;
        dz = 0;

        switch (side) {
        case ConnectorSide::North:
            dy = -1;
            break;
        case ConnectorSide::South:
            dy = 1;
            break;
        case ConnectorSide::East:
            dx = 1;
            break;
        case ConnectorSide::West:
            dx = -1;
            break;
        case ConnectorSide::Up:
            dz = 1;
            break;
        case ConnectorSide::Down:
            dz = -1;
            break;
        }
    }
};

} // namespace urpg::level
