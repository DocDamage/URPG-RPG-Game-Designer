#include "achievement_validator.h"
#include "achievement_trigger.h"
#include <unordered_set>

namespace urpg::achievement {

std::vector<AchievementIssue> AchievementValidator::validate(const std::vector<AchievementDef>& definitions) const {
    std::vector<AchievementIssue> issues;
    std::unordered_set<std::string> seenIds;

    for (const auto& def : definitions) {
        // 1. Empty ID
        if (def.id.empty()) {
            issues.push_back(AchievementIssue{
                AchievementIssueSeverity::Error,
                AchievementIssueCategory::EmptyId,
                def.id,
                "Achievement has an empty id"
            });
        }

        // 2. Duplicate ID
        if (!def.id.empty()) {
            if (seenIds.find(def.id) != seenIds.end()) {
                issues.push_back(AchievementIssue{
                    AchievementIssueSeverity::Error,
                    AchievementIssueCategory::DuplicateId,
                    def.id,
                    "Duplicate achievement id: " + def.id
                });
            }
            seenIds.insert(def.id);
        }

        // 3. Empty title
        if (def.title.empty()) {
            issues.push_back(AchievementIssue{
                AchievementIssueSeverity::Error,
                AchievementIssueCategory::EmptyTitle,
                def.id,
                "Achievement has an empty title"
            });
        }

        // 4. Empty unlock condition
        if (def.unlockCondition.empty()) {
            issues.push_back(AchievementIssue{
                AchievementIssueSeverity::Error,
                AchievementIssueCategory::EmptyUnlockCondition,
                def.id,
                "Achievement has an empty unlock condition"
            });
        }

        // 5. Parse unlock condition and check target
        if (!def.unlockCondition.empty()) {
            auto trigger = AchievementTrigger::parse(def.unlockCondition);
            if (trigger.target == 0) {
                issues.push_back(AchievementIssue{
                    AchievementIssueSeverity::Warning,
                    AchievementIssueCategory::ZeroTarget,
                    def.id,
                    "Achievement unlock condition parses to target 0 (impossible to unlock)"
                });
            }
        }

        // 6. Empty icon ID
        if (def.iconId.empty()) {
            issues.push_back(AchievementIssue{
                AchievementIssueSeverity::Warning,
                AchievementIssueCategory::EmptyIconId,
                def.id,
                "Achievement has an empty iconId"
            });
        }
    }

    return issues;
}

} // namespace urpg::achievement
