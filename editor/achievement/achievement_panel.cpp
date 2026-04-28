#include "editor/achievement/achievement_panel.h"

namespace urpg::editor {

void AchievementPanel::bindRegistry(urpg::achievement::AchievementRegistry* registry) {
    m_registry = registry;
}

void AchievementPanel::bindPlatformProfile(const urpg::achievement::AchievementPlatformProfile* profile) {
    m_platformProfile = profile;
}

bool AchievementPanel::applyPlatformProfile() {
    if (!m_registry) {
        m_lastAction = {{"action", "apply_platform_profile"},
                        {"success", false},
                        {"message", "No AchievementRegistry is bound."}};
        return false;
    }
    if (!m_platformProfile) {
        m_lastAction = {{"action", "apply_platform_profile"},
                        {"success", false},
                        {"message", "No achievement platform profile is bound."}};
        return false;
    }

    const auto result = urpg::achievement::applyAchievementPlatformProfile(*m_registry, *m_platformProfile);
    m_lastAction = {{"action", "apply_platform_profile"},
                    {"success", result.applied},
                    {"backendCount", result.backendCount},
                    {"result", urpg::achievement::achievementPlatformProfileApplyResultToJson(result)}};
    if (!result.applied && !result.diagnostics.empty()) {
        m_lastAction["message"] = result.diagnostics.front().message;
    } else {
        m_lastAction["message"] = "Achievement platform profile applied.";
    }
    return result.applied;
}

std::vector<urpg::achievement::AchievementPlatformResult> AchievementPanel::syncUnlockedAchievements() {
    if (!m_registry) {
        m_lastAction = {{"action", "sync_unlocked_achievements"},
                        {"success", false},
                        {"message", "No AchievementRegistry is bound."}};
        return {};
    }
    auto results = m_registry->syncUnlockedAchievementsToBackends();
    nlohmann::json rows = nlohmann::json::array();
    bool success = true;
    for (const auto& result : results) {
        success = success && result.success;
        rows.push_back({
            {"success", result.success},
            {"platform", result.platform},
            {"achievementId", result.achievementId},
            {"message", result.message},
        });
    }
    m_lastAction = {{"action", "sync_unlocked_achievements"},
                    {"success", success},
                    {"resultCount", results.size()},
                    {"results", rows},
                    {"message", success ? "Unlocked achievements synchronized." : "Achievement platform sync failed."}};
    return results;
}

void AchievementPanel::render() {
    if (!m_registry) {
        m_snapshot = {
            {"panel", "achievement"},
            {"status", "disabled"},
            {"disabled_reason", "No AchievementRegistry is bound."},
            {"owner", "editor/achievement"},
            {"unlock_condition", "Bind AchievementRegistry before rendering the achievement panel."},
            {"platform_profile_bound", m_platformProfile != nullptr},
            {"last_action", m_lastAction},
            {"total_count", 0},
            {"unlocked_count", 0},
            {"achievements", nlohmann::json::array()},
        };
        return;
    }

    const auto achievements = m_registry->getAllAchievements();
    nlohmann::json rows = nlohmann::json::array();
    size_t unlockedCount = 0;

    for (const auto& def : achievements) {
        auto progressOpt = m_registry->getProgress(def.id);
        bool unlocked = false;
        if (progressOpt.has_value()) {
            unlocked = progressOpt->unlocked;
        }
        if (unlocked) {
            ++unlockedCount;
        }

        nlohmann::json row{
            {"title", def.title},
            {"unlocked", unlocked}
        };
        rows.push_back(row);
    }

    nlohmann::json platformProfileDiagnostics = nlohmann::json::array();
    if (m_platformProfile) {
        const urpg::achievement::AchievementPlatformProfileApplyResult validationResult{
            false, 0, urpg::achievement::validateAchievementPlatformProfile(*m_platformProfile)};
        platformProfileDiagnostics =
            urpg::achievement::achievementPlatformProfileApplyResultToJson(validationResult)["diagnostics"];
    }

    nlohmann::json actions = {
        {"applyPlatformProfile", m_platformProfile != nullptr},
        {"syncUnlockedAchievements", true},
    };

    m_snapshot = nlohmann::json{
        {"panel", "achievement"},
        {"status", "ready"},
        {"platform_profile_bound", m_platformProfile != nullptr},
        {"platform_profile", m_platformProfile ? m_platformProfile->toJson() : nlohmann::json(nullptr)},
        {"platform_profile_diagnostics", platformProfileDiagnostics},
        {"platform_backends", m_registry->platformBackendSnapshot()},
        {"last_action", m_lastAction},
        {"actions", actions},
        {"total_count", achievements.size()},
        {"unlocked_count", unlockedCount},
        {"trophy_export", m_registry->exportTrophyPayload()["summary"]},
        {"achievements", rows}};
}

nlohmann::json AchievementPanel::lastRenderSnapshot() const {
    return m_snapshot;
}

} // namespace urpg::editor
