#pragma once

#include "engine/core/math/fixed32.h"
#include <string>

namespace urpg {

/**
 * @brief Logic for quest progression.
 */
enum class QuestState {
    NotStarted,
    Active,
    Completed,
    Failed
};

struct QuestComponent {
    std::string questId;
    QuestState state = QuestState::NotStarted;
    int currentObjectiveProgress = 0;
    int requiredObjectiveCount = 1;
};

class QuestSystem {
public:
    void update(World& world) {
        // Quest updates are usually event driven, but periodically we might 
        // want to check objective status globally.
    }

    void advanceQuest(World& world, EntityID entity, const std::string& questId, int progress = 1) {
        world.ForEachWith<QuestComponent>([&](EntityID id, QuestComponent& quest) {
            if (id == entity && quest.questId == questId) {
                if (quest.state == QuestState::Active) {
                    quest.currentObjectiveProgress += progress;
                    if (quest.currentObjectiveProgress >= quest.requiredObjectiveCount) {
                        quest.state = QuestState::Completed;
                        // Trigger rewards? Particles?
                    }
                }
            }
        });
    }
};

} // namespace urpg
