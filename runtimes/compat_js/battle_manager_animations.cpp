#include "runtimes/compat_js/battle_manager.h"

#include "runtimes/compat_js/battle_manager_support.h"

#include <utility>
#include <vector>

namespace urpg {
namespace compat {
void BattleManager::playAnimation(int32_t animationId, BattleSubject* target) {
    if (!target) {
        return;
    }

    const int32_t durationFrames = resolveBattleAnimationDurationFrames(animationId);
    if (durationFrames <= 0) {
        return;
    }

    BattleAnimationPlayback playback;
    playback.animationId = animationId;
    playback.targetType = target->type;
    playback.targetIndex = target->index;
    playback.targetId = target->id;
    playback.framesRemaining = durationFrames;
    playback.subjectAnimation = false;
    activeAnimations_.push_back(std::move(playback));
}

void BattleManager::playAnimationOnSubject(int32_t animationId, BattleSubject* subject) {
    if (!subject) {
        return;
    }

    const int32_t durationFrames = resolveBattleAnimationDurationFrames(animationId);
    if (durationFrames <= 0) {
        return;
    }

    BattleAnimationPlayback playback;
    playback.animationId = animationId;
    playback.targetType = subject->type;
    playback.targetIndex = subject->index;
    playback.targetId = subject->id;
    playback.framesRemaining = durationFrames;
    playback.subjectAnimation = true;
    activeAnimations_.push_back(std::move(playback));
}

bool BattleManager::isAnimationPlaying() const {
    return !activeAnimations_.empty();
}

const std::vector<BattleAnimationPlayback>& BattleManager::getActiveAnimations() const {
    return activeAnimations_;
}

} // namespace compat
} // namespace urpg
