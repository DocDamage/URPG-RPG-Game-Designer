#pragma once

#include "achievement_registry.h"
#include <vector>

namespace urpg::achievement {

enum class AchievementIssueSeverity {
    Warning,
    Error
};

enum class AchievementIssueCategory {
    EmptyId,
    EmptyTitle,
    EmptyUnlockCondition,
    ZeroTarget,
    DuplicateId,
    EmptyIconId
};

struct AchievementIssue {
    AchievementIssueSeverity severity;
    AchievementIssueCategory category;
    std::string achievementId;
    std::string message;
};

/**
 * @brief Validates a collection of achievement definitions for data-quality issues.
 */
class AchievementValidator {
public:
    std::vector<AchievementIssue> validate(const std::vector<AchievementDef>& definitions) const;
};

} // namespace urpg::achievement
